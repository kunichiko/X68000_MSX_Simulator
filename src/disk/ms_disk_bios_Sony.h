#ifndef MS_MEMMAP_DISKBIOS_SONY_H
#define MS_MEMMAP_DISKBIOS_SONY_H

#include "../memmap/ms_memmap.h"
#include "../memmap/ms_memmap_driver.h"
#include "ms_disk.h"
#include "ms_disk_container.h"
#include "ms_disk_controller_WD2793.h"

#define DISKBIOS_SONY_SIZE (16 * 1024)

/**
 * @brief SONY / Philips タイプのインターフェース (WD2793) 用の DISK BIOS ROM ドライバ
 *
 * ページ1 (0x4000-0x7FFF) にメモリマップド I/O されます。WD2793 のレジスタと
 * 外部の制御/ステータスレジスタは 0x?FF8-0x?FFF にミラーされています:
 *
 *   0x7FF8  R:ステータス     W:コマンド
 *   0x7FF9  R/W トラックレジスタ
 *   0x7FFA  R/W セクタレジスタ
 *   0x7FFB  R/W データレジスタ
 *   0x7FFC  W:サイド選択 (bit0)
 *   0x7FFD  R/W ドライブ選択 (bit1-0) + モーター (bit7)。読み出しの bit2 = 0 でディスク交換あり
 *   0x7FFF  R:  bit6 = !INTRQ, bit7 = !DRQ  (負論理。未接続ビットは 1)
 *
 * openMSX では SONY と Philips のマシンは同じ PhilipsFDC のマッピングを使うため、
 * このドライバ1つで両方に対応できます。
 */
typedef struct ms_memmap_driver_DISKBIOS_SONY {
    ms_memmap_driver_t base;
    // プロパティ
    uint8_t* zero_buffer;
    // インターフェースの外部制御レジスタ
    uint8_t side_reg;
    uint8_t drive_reg;
    // fdc
    ms_disk_controller_WD2793_t fdc;
} ms_memmap_driver_DISKBIOS_SONY_t;

ms_memmap_driver_DISKBIOS_SONY_t* ms_disk_bios_Sony_alloc();
void ms_disk_bios_Sony_init(ms_memmap_driver_DISKBIOS_SONY_t* instance, ms_memmap_t* memmap, uint8_t* buffer,
                            ms_disk_container_t* container);

void ms_memmap_did_attach_DISKBIOS_SONY(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_DISKBIOS_SONY(ms_memmap_driver_t* driver);
void ms_memmap_did_update_memory_mapper_DISKBIOS_SONY(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_DISKBIOS_SONY(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif
