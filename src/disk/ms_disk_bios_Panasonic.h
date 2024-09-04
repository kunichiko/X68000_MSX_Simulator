#ifndef MS_MEMMAP_DISKBIOS_PANASONIC_H
#define MS_MEMMAP_DISKBIOS_PANASONIC_H

#include "../memmap/ms_memmap.h"
#include "../memmap/ms_memmap_driver.h"
#include "ms_disk.h"
#include "ms_disk_controller_TC8566AF.h"
#include "ms_disk_container.h"

#define DISKBIOS_PANASONIC_SIZE (16*1024)

/**
 * @brief TC8566AFを使用する Panasonicの DISK BIOS ROMのドライバ
 * 
 * メモリマップドI/Oを持ち、そのI／Oを通じて TC8566AFコントローラーを操作します
 */
typedef struct ms_memmap_driver_DISKBIOS_PANASONIC {
	ms_memmap_driver_t base;
	// properties
	uint8_t* zero_buffer;
	// fdc
	ms_disk_controller_TC8566AF_t fdc;
} ms_memmap_driver_DISKBIOS_PANASONIC_t;

ms_memmap_driver_DISKBIOS_PANASONIC_t* ms_disk_bios_Panasonic_alloc();
void ms_disk_bios_Panasonic_init(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, ms_disk_container_t* container);

void ms_memmap_deinit_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);
void ms_memmap_did_attach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif