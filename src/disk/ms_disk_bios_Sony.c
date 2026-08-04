/*
 * SONY / Philips タイプの WD2793 インターフェース用 DISK BIOS ROM ドライバ。
 *
 * レジスタマップは ms_disk_bios_Sony.h を参照してください。Panasonic (TC8566AF)
 * 用ドライバを範として作成しています。16KB の ROM はページ1 (0x4000-0x7FFF) に
 * 配置され、FDC のレジスタはその 16KB ウィンドウの末尾 (オフセット 0x3FF8 以降)
 * でデコードされます。
 */

#include "ms_disk_bios_Sony.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../memmap/ms_memmap.h"
#include "../ms.h"

#define THIS ms_memmap_driver_DISKBIOS_SONY_t

static char* driver_name = "DISKBIOS_SONY";

THIS* ms_disk_bios_Sony_alloc() {
    return (THIS*)new_malloc(sizeof(THIS));
}

void ms_disk_bios_Sony_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, ms_disk_container_t* container) {
    if (instance == NULL) {
        return;
    }
    ms_memmap_driver_init(&instance->base, memmap, buffer);

    instance->base.type = ROM_TYPE_DOS_ROM;
    instance->base.name = driver_name;
    instance->base.did_attach = ms_memmap_did_attach_DISKBIOS_SONY;
    instance->base.will_detach = ms_memmap_will_detach_DISKBIOS_SONY;
    instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_DISKBIOS_SONY;
    instance->base.read8 = ms_memmap_read8_DISKBIOS_SONY;
    instance->base.read16 = ms_memmap_read16_DISKBIOS_SONY;
    instance->base.write8 = ms_memmap_write8_DISKBIOS_SONY;
    instance->base.write16 = ms_memmap_write16_DISKBIOS_SONY;

    // ROM を持たないページは 0x00 を読み返します (レジスタページは read8/write8 で
    // 読み書きできるようにするので、ここでは 0 埋めバッファを割り当てるだけです)。
    uint8_t* zero_buffer = (uint8_t*)new_malloc(8 * 1024);
    if (zero_buffer == NULL) {
        printf("メモリが確保できません。\n");
        return;
    }
    int i;
    for (i = 0; i < 8 * 1024; i++) {
        zero_buffer[i] = 0x00;
    }
    instance->zero_buffer = zero_buffer;

    // ROM はページ1 (0x4000-0x7FFF) = page8k[2],[3] に配置する
    int page8k = 0;
    for (; page8k < 2; page8k++) {
        instance->base.page8k_pointers[page8k] = zero_buffer;
    }
    for (; page8k < 4; page8k++) {
        instance->base.page8k_pointers[page8k] = instance->base.buffer + ((page8k - 2) * 0x2000);
    }
    for (; page8k < 8; page8k++) {
        instance->base.page8k_pointers[page8k] = zero_buffer;
    }

    instance->side_reg = 0;
    instance->drive_reg = 0;

    ms_disk_controller_WD2793_init(&instance->fdc, container);
}

void ms_memmap_did_attach_DISKBIOS_SONY(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_DISKBIOS_SONY(ms_memmap_driver_t* driver) {
    return 0;
}

void ms_memmap_did_update_memory_mapper_DISKBIOS_SONY(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
}

uint8_t ms_memmap_read8_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr) {
    THIS* d = (THIS*)driver;
    int addr_16k = addr & 0x3fff;
    if ((addr_16k & 0x3ff0) == 0x3ff0) {
        switch (addr_16k) {
        case 0x3ff8:
            return ms_disk_controller_WD2793_get_status_reg(&d->fdc);
        case 0x3ff9:
            return ms_disk_controller_WD2793_get_track_reg(&d->fdc);
        case 0x3ffa:
            return ms_disk_controller_WD2793_get_sector_reg(&d->fdc);
        case 0x3ffb:
            return ms_disk_controller_WD2793_get_data_reg(&d->fdc);
        case 0x3ffc:
            // サイドレジスタ (bit0 = サイド選択)
            return d->side_reg;
        case 0x3ffd: {
            // bit1-0 ドライブ番号, bit2 = 0 でディスク交換あり, bit7 モーター
            uint8_t res = d->drive_reg & ~0x04;
            if (!ms_disk_controller_WD2793_is_disk_changed(&d->fdc)) {
                res |= 0x04;
            }
            return res;
        }
        case 0x3fff: {
            // IRQ/DRQ ラインは負論理。未接続ビットは 1 にプルアップされている
            uint8_t value = 0xff;
            if (ms_disk_controller_WD2793_get_irq(&d->fdc)) {
                value &= ~0x40;
            }
            if (ms_disk_controller_WD2793_get_drq(&d->fdc)) {
                value &= ~0x80;
            }
            return value;
        }
        default:
            return 0xff;
        }
    } else {
        int page8k = addr >> 13;
        int addr_8k = addr_16k & 0x1fff;
        return driver->page8k_pointers[page8k][addr_8k];
    }
}

void ms_memmap_write8_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
    THIS* d = (THIS*)driver;
    int addr_16k = addr & 0x3fff;
    if ((addr_16k & 0x3ff0) == 0x3ff0) {
        switch (addr_16k) {
        case 0x3ff8:
            ms_disk_controller_WD2793_set_command_reg(&d->fdc, data);
            break;
        case 0x3ff9:
            ms_disk_controller_WD2793_set_track_reg(&d->fdc, data);
            break;
        case 0x3ffa:
            ms_disk_controller_WD2793_set_sector_reg(&d->fdc, data);
            break;
        case 0x3ffb:
            ms_disk_controller_WD2793_set_data_reg(&d->fdc, data);
            break;
        case 0x3ffc:
            // サイド選択 (bit0)
            d->side_reg = data;
            ms_disk_controller_WD2793_set_side(&d->fdc, data & 1);
            break;
        case 0x3ffd: {
            // bit1-0 ドライブ選択 (00/10 -> A, 01 -> B, 11 -> なし), bit7 モーター
            d->drive_reg = data;
            uint8_t drive_no;
            switch (data & 0x03) {
            case 0:
            case 2:
                drive_no = 0;  // ドライブ A
                break;
            case 1:
                drive_no = 1;  // ドライブ B
                break;
            case 3:
            default:
                drive_no = 3;  // なし (ダミードライブに割り当てる)
                break;
            }
            ms_disk_controller_WD2793_set_drive(&d->fdc, drive_no);
            ms_disk_controller_WD2793_set_motor(&d->fdc, (data & 0x80) ? 1 : 0);

            ms_fdd_led_1 = (data & 0x80) ? 1 : 0;
            break;
        }
        default:
            MS_LOG(MS_LOG_INFO, "DISKBIOS_SONY: write8: unknown addr: %04x\n", addr_16k);
            break;
        }
    }
}

uint16_t ms_memmap_read16_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr) {
    return ms_memmap_read8_DISKBIOS_SONY(driver, addr) | (ms_memmap_read8_DISKBIOS_SONY(driver, addr + 1) << 8);
}

void ms_memmap_write16_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
    ms_memmap_write8_DISKBIOS_SONY(driver, addr, data & 0xff);
    ms_memmap_write8_DISKBIOS_SONY(driver, addr + 1, data >> 8);
}
