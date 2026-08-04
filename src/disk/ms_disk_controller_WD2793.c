/*
 * WD2793 フロッピーディスクコントローラーのエミュレーション
 *
 * 概要は ms_disk_controller_WD2793.h を参照してください。レジスタの I/O は
 * DISK BIOS のメモリマップドライバ (ms_disk_bios_Sony) から駆動され、メディア
 * アクセスは TC8566AF コントローラーと同じく ms_disk_drive_floppy_t
 * インターフェースを再利用します。
 *
 * レジスタ配置 (SONY/Philips インターフェースがページ1に見せているもの):
 *   0x7FF8  R:ステータス  W:コマンド
 *   0x7FF9  R/W トラックレジスタ
 *   0x7FFA  R/W セクタレジスタ
 *   0x7FFB  R/W データレジスタ
 *
 * コマンドはコマンドバイトの上位ニブルでデコードします:
 *   0x0X Restore, 0x1X Seek, 0x2/3X Step, 0x4/5X Step-in, 0x6/7X Step-out (Type I)
 *   0x8X Read Sector, 0x9X Read Sector (マルチ)   (Type II)
 *   0xAX Write Sector, 0xBX Write Sector (マルチ) (Type II)
 *   0xCX Read Address, 0xEX Read Track, 0xFX Write Track (Type III)
 *   0xDX Force Interrupt (Type IV)
 */

#include "ms_disk_controller_WD2793.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../ms.h"

#define THIS ms_disk_controller_WD2793_t

// 2DD の 1トラック分のバイト数 (ms_disk_raw_track_t と同じ)。インデックスパルスを
// エミュレーションしていないため、Write Track (フォーマット) の終了判定に使います。
#define WD2793_TRACK_LENGTH 6250

static void wd2793_start_type1(THIS* d);
static void wd2793_start_read_sector(THIS* d);
static void wd2793_start_write_sector(THIS* d);
static void wd2793_start_read_address(THIS* d);
static void wd2793_start_write_track(THIS* d);
static void wd2793_end_command(THIS* d);
static uint8_t wd2793_find_sector(THIS* d);

THIS* ms_disk_controller_WD2793_alloc() {
    return (THIS*)new_malloc(sizeof(THIS));
}

void ms_disk_controller_WD2793_init(THIS* instance, ms_disk_container_t* container) {
    if (instance == NULL) {
        return;
    }
    // ドライブ0 のみ実コンテナを持ち、1-3 はダミードライブとして初期化します
    int i = 0;
    ms_disk_drive_floppy_init(&instance->drive[i], container);
    for (i = 1; i < 4; i++) {
        ms_disk_drive_floppy_init(&instance->drive[i], NULL);
    }

    instance->command_reg = 0;
    instance->status_reg = 0;
    instance->track_reg = 0;
    instance->sector_reg = 1;
    instance->data_reg = 0;

    instance->current_drive = 0;
    instance->side = 0;
    instance->motor = 0;

    instance->intrq = 0;
    instance->drq = 0;

    instance->present_track = 0;
    instance->step_direction = 1;
    instance->index_phase = 0;

    instance->xfer_mode = WD2793_XFER_NONE;
    instance->multi = 0;
    instance->transfer_rest = 0;
    instance->sector_buffer_byte_offset = 0;
    instance->addr_buffer_offset = 0;
}

void ms_disk_controller_WD2793_deinit(THIS* instance) {
}

// -----------------------------------------------------------------------------
//  コマンドレジスタ (0x7FF8 書き込み)
// -----------------------------------------------------------------------------
void ms_disk_controller_WD2793_set_command_reg(THIS* d, uint8_t value) {
    d->command_reg = value;
    d->intrq = 0;  // 新しいコマンドの発行で割り込み要求はクリアされる

    uint8_t type = value & 0xF0;
    MS_LOG(MS_LOG_FINE, "WD2793 CMD: %02x\n", value);

    if (type == 0xD0) {
        // Type IV: Force Interrupt - 実行中のコマンドを中断する
        d->status_reg &= ~WD2793_ST_BUSY;
        d->drq = 0;
        d->xfer_mode = WD2793_XFER_NONE;
        if (value & 0x08) {
            d->intrq = 1;  // 即時割り込みが要求された場合
        }
        return;
    }

    if ((value & 0x80) == 0) {
        // Type I: Restore / Seek / Step
        wd2793_start_type1(d);
    } else {
        switch (type) {
        case 0x80:  // Read Sector
        case 0x90:  // Read Sector (マルチ)
            wd2793_start_read_sector(d);
            break;
        case 0xA0:  // Write Sector
        case 0xB0:  // Write Sector (マルチ)
            wd2793_start_write_sector(d);
            break;
        case 0xC0:  // Read Address
            wd2793_start_read_address(d);
            break;
        case 0xE0:  // Read Track (未実装。ベストエフォート)
            MS_LOG(MS_LOG_INFO, "WD2793: READ TRACK not implemented\n");
            d->status_reg = 0;
            d->intrq = 1;
            break;
        case 0xF0:  // Write Track (フォーマット)
            wd2793_start_write_track(d);
            break;
        default:
            MS_LOG(MS_LOG_ERROR, "WD2793: unknown command %02x\n", value);
            d->status_reg = 0;
            d->intrq = 1;
            break;
        }
    }
}

// -----------------------------------------------------------------------------
//  ステータスレジスタ (0x7FF8 読み込み)
// -----------------------------------------------------------------------------
uint8_t ms_disk_controller_WD2793_get_status_reg(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    uint8_t st = d->status_reg;
    uint8_t type = d->command_reg & 0xF0;
    int is_type1 = ((d->command_reg & 0x80) == 0) || (type == 0xD0);

    if (is_type1) {
        // Type I / Type IV: bit1-5 はヘッド/トラック/インデックスの状態を随時反映する
        st &= ~(WD2793_ST_INDEX | WD2793_ST_TRACK00 | WD2793_ST_HEAD_LOADED | WD2793_ST_WRITE_PROTECTED);
        // インデックス待ちのループが進むように、疑似インデックスパルスをトグルする
        d->index_phase ^= 1;
        if (d->index_phase) {
            st |= WD2793_ST_INDEX;
        }
        if (d->present_track == 0) {
            st |= WD2793_ST_TRACK00;
        }
        if (d->motor) {
            st |= WD2793_ST_HEAD_LOADED;
        }
        if (drive->is_write_protected) {
            st |= WD2793_ST_WRITE_PROTECTED;
        }
    } else {
        // Type II / III: bit1 は DRQ
        if (d->drq) {
            st |= WD2793_ST_DRQ;
        } else {
            st &= ~WD2793_ST_DRQ;
        }
    }

    // NOT READY (全コマンド共通)
    if ((drive->container != NULL) && drive->is_disk_inserted(drive)) {
        st &= ~WD2793_ST_NOT_READY;
    } else {
        st |= WD2793_ST_NOT_READY;
    }

    d->intrq = 0;  // ステータスレジスタの読み込みで割り込み要求はクリアされる
    MS_LOG(MS_LOG_TRACE, "WD2793 ST: %02x\n", st);
    return st;
}

// -----------------------------------------------------------------------------
//  トラック / セクタレジスタ
// -----------------------------------------------------------------------------
void ms_disk_controller_WD2793_set_track_reg(THIS* d, uint8_t value) {
    d->track_reg = value;
}
uint8_t ms_disk_controller_WD2793_get_track_reg(THIS* d) {
    return d->track_reg;
}
void ms_disk_controller_WD2793_set_sector_reg(THIS* d, uint8_t value) {
    d->sector_reg = value;
}
uint8_t ms_disk_controller_WD2793_get_sector_reg(THIS* d) {
    return d->sector_reg;
}

// -----------------------------------------------------------------------------
//  データレジスタ (0x7FFB)
// -----------------------------------------------------------------------------
uint8_t ms_disk_controller_WD2793_get_data_reg(THIS* d) {
    if (d->xfer_mode == WD2793_XFER_READ_SECTOR && d->drq) {
        d->data_reg = d->sector_buffer.data[d->sector_buffer_byte_offset++];
        d->transfer_rest--;
        if (d->sector_buffer_byte_offset >= 512) {
            // 1セクタ分の読み込みが完了
            if (d->multi) {
                d->sector_reg++;
                if (wd2793_find_sector(d)) {
                    d->sector_buffer_byte_offset = 0;
                    d->transfer_rest = 512;
                } else {
                    d->status_reg |= WD2793_ST_RECORD_NOT_FOUND;
                    wd2793_end_command(d);
                }
            } else {
                wd2793_end_command(d);
            }
        }
        return d->data_reg;
    }

    if (d->xfer_mode == WD2793_XFER_READ_ADDRESS && d->drq) {
        d->data_reg = d->addr_buffer[d->addr_buffer_offset++];
        d->transfer_rest--;
        if (d->addr_buffer_offset >= 6) {
            wd2793_end_command(d);
        }
        return d->data_reg;
    }

    return d->data_reg;
}

void ms_disk_controller_WD2793_set_data_reg(THIS* d, uint8_t value) {
    d->data_reg = value;

    if (d->xfer_mode == WD2793_XFER_WRITE_SECTOR && d->drq) {
        d->sector_buffer.data[d->sector_buffer_byte_offset++] = value;
        d->transfer_rest--;
        if (d->sector_buffer_byte_offset >= 512) {
            ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
            d->sector_buffer.track = d->track_reg;
            d->sector_buffer.head = d->side;
            d->sector_buffer.sector = d->sector_reg;
            MS_LOG(MS_LOG_DEBUG, "WD2793 WRITE: C:%d H:%d R:%d\n", d->sector_buffer.track, d->sector_buffer.head,
                   d->sector_buffer.sector);
            drive->write_sector(drive, &d->sector_buffer);
            if (d->multi) {
                d->sector_reg++;
                d->sector_buffer_byte_offset = 0;
                d->transfer_rest = 512;
            } else {
                wd2793_end_command(d);
            }
        }
        return;
    }

    if (d->xfer_mode == WD2793_XFER_WRITE_TRACK && d->drq) {
        // ベストエフォートのフォーマット処理。書き込みバイト列から ID マーク(0xFE)に
        // 続く C,H,R,N と、データマーク(0xFB/0xF8)に続くデータフィールドを取り出します。
        // .DSK 形式はギャップ情報を保持しないため、ギャップ/CRC バイトは無視します。
        //
        // パーサの状態はインスタンス側に保持します (addr_buffer / オフセットを流用):
        //   addr_buffer[0..3]        = 構築中セクタの CHRN
        //   addr_buffer_offset phase = 0            : アイドル (マーク待ち)
        //                              1..4          : CHRN の n-1 バイト目を収集中
        //                              100..(100+L)  : データフィールドを収集中
        d->transfer_rest--;
        int phase = d->addr_buffer_offset;
        if (phase == 0) {
            if (value == 0xFE) {
                d->addr_buffer_offset = 1;  // ID アドレスマーク -> CHRN の収集を開始
            } else if (value == 0xFB || value == 0xF8) {
                d->addr_buffer_offset = 100;  // データ (または削除データ) マーク
            }
        } else if (phase >= 1 && phase <= 4) {
            d->addr_buffer[phase - 1] = value;
            if (phase == 4) {
                d->addr_buffer_offset = 0;  // CHRN を取得。データマーク待ちへ
            } else {
                d->addr_buffer_offset = phase + 1;
            }
        } else if (phase >= 100) {
            int len = 128 << (d->addr_buffer[3] & 0x03);
            if (len > 512) {
                len = 512;
            }
            int idx = phase - 100;
            if (idx < 512) {
                d->sector_buffer.data[idx] = value;
            }
            if (idx + 1 >= len) {
                // データフィールドを全て収集 -> セクタを書き出す
                ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
                d->sector_buffer.track = d->addr_buffer[0];
                d->sector_buffer.head = d->addr_buffer[1];
                d->sector_buffer.sector = d->addr_buffer[2];
                MS_LOG(MS_LOG_INFO, "WD2793 FORMAT: C:%d H:%d R:%d N:%d\n", d->addr_buffer[0], d->addr_buffer[1],
                       d->addr_buffer[2], d->addr_buffer[3]);
                drive->write_sector(drive, &d->sector_buffer);
                d->addr_buffer_offset = 0;
            } else {
                d->addr_buffer_offset = phase + 1;
            }
        }

        if (d->transfer_rest <= 0) {
            wd2793_end_command(d);
        }
        return;
    }
}

// -----------------------------------------------------------------------------
//  外部制御ライン (0x7FFC サイド, 0x7FFD ドライブ/モーター, 0x7FFF irq/drq)
// -----------------------------------------------------------------------------
void ms_disk_controller_WD2793_set_side(THIS* d, uint8_t side) {
    d->side = side & 1;
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    drive->set_side(drive, d->side);
}

void ms_disk_controller_WD2793_set_drive(THIS* d, uint8_t drive_no) {
    if (drive_no > 3) {
        return;
    }
    d->current_drive = drive_no;
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    drive->set_side(drive, d->side);
}

void ms_disk_controller_WD2793_set_motor(THIS* d, uint8_t motoron) {
    d->motor = motoron ? 1 : 0;
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    drive->set_motor(drive, d->motor);
}

uint8_t ms_disk_controller_WD2793_get_irq(THIS* d) {
    return d->intrq;
}
uint8_t ms_disk_controller_WD2793_get_drq(THIS* d) {
    return d->drq;
}
uint8_t ms_disk_controller_WD2793_is_disk_changed(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    if (drive->container == NULL) {
        return 0;
    }
    return drive->base.is_disk_changed(&drive->base);
}

// -----------------------------------------------------------------------------
//  コマンド実行ヘルパー
// -----------------------------------------------------------------------------
static void wd2793_end_command(THIS* d) {
    d->status_reg &= ~WD2793_ST_BUSY;
    d->drq = 0;
    d->xfer_mode = WD2793_XFER_NONE;
    d->intrq = 1;
}

static void wd2793_start_type1(THIS* d) {
    uint8_t type = d->command_reg & 0xF0;
    d->status_reg &= ~(WD2793_ST_SEEK_ERROR | WD2793_ST_CRC_ERROR);

    switch (type) {
    case 0x00:  // Restore
        d->present_track = 0;
        d->track_reg = 0;
        break;
    case 0x10:  // Seek (シーク先トラックはデータレジスタに入っている)
        d->present_track = d->data_reg;
        d->track_reg = d->data_reg;
        break;
    case 0x20:  // Step (直前の方向を繰り返す)
    case 0x30:
        d->present_track += d->step_direction;
        if (d->command_reg & WD2793_FLAG_T) {
            d->track_reg += d->step_direction;
        }
        break;
    case 0x40:  // Step-in (トラック番号が増える方向へ)
    case 0x50:
        d->step_direction = 1;
        d->present_track++;
        if (d->command_reg & WD2793_FLAG_T) {
            d->track_reg++;
        }
        break;
    case 0x60:  // Step-out (トラック0へ向かう方向へ)
    case 0x70:
        d->step_direction = -1;
        if (d->present_track > 0) {
            d->present_track--;
        }
        if (d->command_reg & WD2793_FLAG_T) {
            d->track_reg--;
        }
        break;
    default:
        break;
    }

    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    if (drive->container != NULL) {
        drive->seek(drive, d->present_track);
    }

    // この簡易モデルでは Type I コマンドは即座に完了する
    d->status_reg &= ~WD2793_ST_BUSY;
    d->drq = 0;
    d->xfer_mode = WD2793_XFER_NONE;
    d->intrq = 1;
    MS_LOG(MS_LOG_FINE, "WD2793 TYPE1 done: track=%d\n", d->present_track);
}

// 現在のトラックから track_reg / side / sector_reg に一致するセクタを探します。
// 見つかれば d->sector_buffer を埋めて 1 を、見つからなければ 0 を返します。
static uint8_t wd2793_find_sector(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    int i;
    for (i = 0; i < 9 * 2; i++) {
        if (!drive->get_next_sector(drive, &d->sector_buffer)) {
            return 0;
        }
        if (d->sector_buffer.track == d->track_reg &&    //
            d->sector_buffer.head == d->side &&          //
            d->sector_buffer.sector == d->sector_reg) {  //
            return 1;
        }
        MS_LOG(MS_LOG_FINE, "WD2793 skip C:%d H:%d R:%d (want %d %d %d)\n", d->sector_buffer.track, d->sector_buffer.head,
               d->sector_buffer.sector, d->track_reg, d->side, d->sector_reg);
    }
    return 0;
}

static void wd2793_start_read_sector(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    d->status_reg = WD2793_ST_BUSY;
    d->status_reg &= ~(WD2793_ST_RECORD_NOT_FOUND | WD2793_ST_CRC_ERROR | WD2793_ST_LOST_DATA | WD2793_ST_RECORD_TYPE);

    if (drive->container == NULL || !drive->is_disk_inserted(drive)) {
        d->status_reg = WD2793_ST_NOT_READY;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        return;
    }

    d->multi = (d->command_reg & WD2793_FLAG_M) ? 1 : 0;
    if (wd2793_find_sector(d)) {
        d->xfer_mode = WD2793_XFER_READ_SECTOR;
        d->sector_buffer_byte_offset = 0;
        d->transfer_rest = 512;
        d->drq = 1;
        MS_LOG(MS_LOG_DEBUG, "WD2793 READ SECTOR: C:%d H:%d R:%d\n", d->track_reg, d->side, d->sector_reg);
    } else {
        d->status_reg &= ~WD2793_ST_BUSY;
        d->status_reg |= WD2793_ST_RECORD_NOT_FOUND;
        d->drq = 0;
        d->xfer_mode = WD2793_XFER_NONE;
        d->intrq = 1;
        MS_LOG(MS_LOG_INFO, "WD2793 READ SECTOR not found: C:%d H:%d R:%d\n", d->track_reg, d->side, d->sector_reg);
    }
}

static void wd2793_start_write_sector(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    d->status_reg = WD2793_ST_BUSY;
    d->status_reg &= ~(WD2793_ST_RECORD_NOT_FOUND | WD2793_ST_CRC_ERROR | WD2793_ST_LOST_DATA | WD2793_ST_WRITE_PROTECTED);

    if (drive->container == NULL || !drive->is_disk_inserted(drive)) {
        d->status_reg = WD2793_ST_NOT_READY;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        return;
    }
    if (drive->is_write_protected) {
        d->status_reg &= ~WD2793_ST_BUSY;
        d->status_reg |= WD2793_ST_WRITE_PROTECTED;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        MS_LOG(MS_LOG_INFO, "WD2793 WRITE SECTOR: write protected\n");
        return;
    }

    d->multi = (d->command_reg & WD2793_FLAG_M) ? 1 : 0;
    d->xfer_mode = WD2793_XFER_WRITE_SECTOR;
    d->sector_buffer_byte_offset = 0;
    d->transfer_rest = 512;
    memset(d->sector_buffer.data, 0, sizeof(d->sector_buffer.data));
    d->drq = 1;  // ホストに最初のデータバイトを要求する
    MS_LOG(MS_LOG_DEBUG, "WD2793 WRITE SECTOR: C:%d H:%d R:%d\n", d->track_reg, d->side, d->sector_reg);
}

static void wd2793_start_read_address(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    d->status_reg = WD2793_ST_BUSY;
    d->status_reg &= ~(WD2793_ST_RECORD_NOT_FOUND | WD2793_ST_CRC_ERROR | WD2793_ST_LOST_DATA);

    if (drive->container == NULL || !drive->is_disk_inserted(drive) || !drive->get_next_sector(drive, &d->sector_buffer)) {
        d->status_reg &= ~WD2793_ST_BUSY;
        d->status_reg |= WD2793_ST_RECORD_NOT_FOUND;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        return;
    }

    // ID フィールド: C, H, R, N, CRC 上位, CRC 下位。512 バイトセクタなので N=2。
    d->addr_buffer[0] = d->sector_buffer.track;
    d->addr_buffer[1] = d->sector_buffer.head;
    d->addr_buffer[2] = d->sector_buffer.sector;
    d->addr_buffer[3] = 2;
    d->addr_buffer[4] = 0;  // CRC (モデル化していない)
    d->addr_buffer[5] = 0;
    // ID のトラックアドレスはセクタレジスタにコピーされる
    d->sector_reg = d->sector_buffer.track;

    d->addr_buffer_offset = 0;
    d->transfer_rest = 6;
    d->xfer_mode = WD2793_XFER_READ_ADDRESS;
    d->drq = 1;
    MS_LOG(MS_LOG_DEBUG, "WD2793 READ ADDRESS: C:%d H:%d R:%d\n", d->addr_buffer[0], d->addr_buffer[1], d->addr_buffer[2]);
}

static void wd2793_start_write_track(THIS* d) {
    ms_disk_drive_floppy_t* drive = &d->drive[d->current_drive];
    d->status_reg = WD2793_ST_BUSY;
    d->status_reg &= ~(WD2793_ST_WRITE_PROTECTED | WD2793_ST_LOST_DATA);

    if (drive->container == NULL || !drive->is_disk_inserted(drive)) {
        d->status_reg = WD2793_ST_NOT_READY;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        return;
    }
    if (drive->is_write_protected) {
        d->status_reg &= ~WD2793_ST_BUSY;
        d->status_reg |= WD2793_ST_WRITE_PROTECTED;
        d->intrq = 1;
        d->xfer_mode = WD2793_XFER_NONE;
        return;
    }

    // ベストエフォートのフォーマット。インデックスパルスをエミュレーションしていないため、
    // 1トラック分のバイトを受け取った時点で終了します。addr_buffer_offset はパーサの状態です。
    d->xfer_mode = WD2793_XFER_WRITE_TRACK;
    d->addr_buffer_offset = 0;
    d->transfer_rest = WD2793_TRACK_LENGTH;
    memset(d->sector_buffer.data, 0, sizeof(d->sector_buffer.data));
    d->drq = 1;
    MS_LOG(MS_LOG_DEBUG, "WD2793 WRITE TRACK (format) track=%d side=%d\n", d->present_track, d->side);
}
