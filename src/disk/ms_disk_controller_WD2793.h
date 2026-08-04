#ifndef MS_DISK_CONTROLLER_WD2793_H
#define MS_DISK_CONTROLLER_WD2793_H

#include <stdint.h>

#include "ms_disk.h"
#include "ms_disk_container.h"
#include "ms_disk_drive_floppy.h"

/*
 * WD2793 (Western Digital) フロッピーディスクコントローラーのエミュレーション。
 *
 * このチップは SONY / Philips (その他) の MSX で使われています。TC8566AF
 * (Panasonic系の MSX で使用) と異なり、WD2793 は直接アクセスできる4つのレジスタ
 * (コマンド/ステータス, トラック, セクタ, データ) と、コマンドバイトの上位ニブルで
 * デコードされる小さなコマンドセットを持っています。
 *
 * DISK BIOS のメモリマップドライバ (ms_disk_bios_Sony) が、メモリマップドされた
 * レジスタアクセスをここで宣言するメソッドへ転送します。実際のメディアアクセスは
 * TC8566AF コントローラーと同様に ms_disk_drive_floppy_t インターフェースを
 * 再利用します。
 *
 * 実装は TC8566AF コントローラーと同じく、サイクル精度を持たない簡易モデルです。
 * 回転/シークのタイミングは再現しません。データはデータレジスタを通して 512 バイト
 * セクタ単位で転送され、BIOS がステータスレジスタでポーリングする DRQ/BUSY の
 * ハンドシェイクで駆動されます。
 */

// --- WD2793 ステータスレジスタのビット ---------------------------------------
// Type I コマンドと Type II/III コマンドでビットの意味が異なります。
#define WD2793_ST_NOT_READY 0x80        // (全て)   ドライブがレディでない
#define WD2793_ST_WRITE_PROTECTED 0x40  // (全て)   ライトプロテクト
#define WD2793_ST_HEAD_LOADED 0x20      // (TypeI)  ヘッドがロードされている
#define WD2793_ST_RECORD_TYPE 0x20      // (TypeII) 削除データマーク
#define WD2793_ST_SEEK_ERROR 0x10       // (TypeI)  シークエラー
#define WD2793_ST_RECORD_NOT_FOUND 0x10 // (TypeII) レコードが見つからない
#define WD2793_ST_CRC_ERROR 0x08        // (全て)   CRC エラー
#define WD2793_ST_TRACK00 0x04          // (TypeI)  ヘッドがトラック0上にある
#define WD2793_ST_LOST_DATA 0x04        // (TypeII) データロスト
#define WD2793_ST_INDEX 0x02            // (TypeI)  インデックスパルス
#define WD2793_ST_DRQ 0x02              // (TypeII) データ要求
#define WD2793_ST_BUSY 0x01             // (全て)   コマンド実行中

// --- コマンドフラグ ----------------------------------------------------------
#define WD2793_FLAG_T 0x10  // TypeI:  ステップ時にトラックレジスタを更新
#define WD2793_FLAG_M 0x10  // TypeII: マルチセクタ
#define WD2793_FLAG_V 0x04  // TypeI:  ベリファイ

// データ転送フェーズ (コマンドの実行フェーズ)
#define WD2793_XFER_NONE 0
#define WD2793_XFER_READ_SECTOR 1
#define WD2793_XFER_WRITE_SECTOR 2
#define WD2793_XFER_READ_ADDRESS 3
#define WD2793_XFER_WRITE_TRACK 4

typedef struct ms_disk_controller_WD2793 {
    // 参照
    ms_disk_drive_floppy_t drive[4];

    // レジスタ
    uint8_t command_reg;
    uint8_t status_reg;
    uint8_t track_reg;
    uint8_t sector_reg;
    uint8_t data_reg;

    // インターフェースの外部制御レジスタ (0x7FFC / 0x7FFD)
    uint8_t current_drive;  // 選択中のドライブ (0-3)
    uint8_t side;           // 選択中のサイド (0/1)
    uint8_t motor;          // モーターの on/off

    // ステータスライン
    uint8_t intrq;  // コマンド完了割り込み要求
    uint8_t drq;    // データ要求

    // 現在の物理ヘッド位置と直前のステップ方向 (Type I 用)
    uint8_t present_track;
    int8_t step_direction;

    // フリーランのインデックスパルス位相 (実際のタイミングは持たず、インデックスの
    // エッジ待ちループが進むようにトグルするだけ)
    uint8_t index_phase;

    // データ転送の状態 (実行フェーズ)
    uint8_t xfer_mode;
    uint8_t multi;                    // マルチセクタ動作
    int transfer_rest;                // 転送残りバイト数
    ms_disk_sector_t sector_buffer;   // 読み書き中のセクタ
    int sector_buffer_byte_offset;    // sector_buffer 内の読み書きオフセット
    uint8_t addr_buffer[6];           // READ ADDRESS の結果 (C,H,R,N,CRC1,CRC2)
    int addr_buffer_offset;
} ms_disk_controller_WD2793_t;

ms_disk_controller_WD2793_t* ms_disk_controller_WD2793_alloc();
void ms_disk_controller_WD2793_init(ms_disk_controller_WD2793_t* instance, ms_disk_container_t* container);
void ms_disk_controller_WD2793_deinit(ms_disk_controller_WD2793_t* instance);

// レジスタアクセス。DISK BIOS のメモリマップドライバから呼び出されます
void ms_disk_controller_WD2793_set_command_reg(ms_disk_controller_WD2793_t* d, uint8_t value);
uint8_t ms_disk_controller_WD2793_get_status_reg(ms_disk_controller_WD2793_t* d);
void ms_disk_controller_WD2793_set_track_reg(ms_disk_controller_WD2793_t* d, uint8_t value);
uint8_t ms_disk_controller_WD2793_get_track_reg(ms_disk_controller_WD2793_t* d);
void ms_disk_controller_WD2793_set_sector_reg(ms_disk_controller_WD2793_t* d, uint8_t value);
uint8_t ms_disk_controller_WD2793_get_sector_reg(ms_disk_controller_WD2793_t* d);
void ms_disk_controller_WD2793_set_data_reg(ms_disk_controller_WD2793_t* d, uint8_t value);
uint8_t ms_disk_controller_WD2793_get_data_reg(ms_disk_controller_WD2793_t* d);

// 外部制御ライン
void ms_disk_controller_WD2793_set_side(ms_disk_controller_WD2793_t* d, uint8_t side);
void ms_disk_controller_WD2793_set_drive(ms_disk_controller_WD2793_t* d, uint8_t drive_no);
void ms_disk_controller_WD2793_set_motor(ms_disk_controller_WD2793_t* d, uint8_t motoron);
uint8_t ms_disk_controller_WD2793_get_irq(ms_disk_controller_WD2793_t* d);
uint8_t ms_disk_controller_WD2793_get_drq(ms_disk_controller_WD2793_t* d);
uint8_t ms_disk_controller_WD2793_is_disk_changed(ms_disk_controller_WD2793_t* d);

#endif
