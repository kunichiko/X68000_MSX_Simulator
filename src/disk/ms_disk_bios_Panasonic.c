#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "../memmap/ms_memmap.h"
#include "ms_disk_bios_Panasonic.h"

char* driver_name_DISKBIOS_PANASONIC = "DISKBIOS_PANASONIC";

/*
	確保ルーチン
 */
ms_memmap_driver_DISKBIOS_PANASONIC_t* ms_disk_bios_Panasonic_alloc() {
	return (ms_memmap_driver_DISKBIOS_PANASONIC_t*)new_malloc(sizeof(ms_memmap_driver_DISKBIOS_PANASONIC_t));
}

/*
	初期化ルーチン
 */
void ms_disk_bios_Panasonic_init(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	instance->base.type = ROM_TYPE_DOS_ROM;
	instance->base.name = driver_name_DISKBIOS_PANASONIC;
	//instance->base.deinit = ms_memmap_DISKBIOS_PANASONIC_deinit; オーバーライド不要
	instance->base.did_attach = ms_memmap_did_attach_DISKBIOS_PANASONIC;
	instance->base.will_detach = ms_memmap_will_detach_DISKBIOS_PANASONIC;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC;
	instance->base.read8 = ms_memmap_read8_DISKBIOS_PANASONIC;
	instance->base.read16 = ms_memmap_read16_DISKBIOS_PANASONIC;
	instance->base.write8 = ms_memmap_write8_DISKBIOS_PANASONIC;
	instance->base.write16 = ms_memmap_write16_DISKBIOS_PANASONIC;

	instance->base.buffer = buffer;
	instance->length = length;

	int page8k = 0;
	for(; page8k < 2; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}
	for(; page8k < 4; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + ((page8k-2) * 0x2000);
	}
	for(; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	// FDCの初期化
	ms_disk_controller_TC8566A_init(&instance->fdc, container);

	return;
}

void ms_memmap_did_attach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
}


/**
 * @brief 
 * 	Address	R/W	Feature
 *	0x3FF8	W	レジスタ2 を更新 (write only)
 *	0x3FF9	W	レジスタ3 を更新 (write only)
 *	0x3FFA	R/W	レジスタ4 を参照・更新
 *	0x3FFB	R/W	レジスタ5 を参照・更新
 */
uint8_t ms_memmap_read8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	addr &= 0x3fff;
	if ((addr & 0x3ff0) == 0x3ff0) {
		// Memory mapped DISK I/O
		switch(addr) {
		case 0x3ff8:
			// レジスタ2は write only
			return 0xff;
		case 0x3ff9:
			// レジスタ3は write only
			return 0xff;
		case 0x3ffa:
			// レジスタ4 を参照
			return _TC8556AF_reg4_read(d);
		case 0x3ffb:
			// レジスタ5 を参照
			return _TC8556AF_reg5_read(d);
		default:
			return 0xff;
		}
	} else {
		uint8_t ret = driver->buffer[addr];
		return ret;
	}
}

/**
 * @brief 
 * 	Address	R/W	Feature
 *	0x3FF8	W	レジスタ2 を更新 (write only)
 *	0x3FF9	W	レジスタ3 を更新 (write only)
 *	0x3FFA	R/W	レジスタ4 を参照・更新
 *	0x3FFB	R/W	レジスタ5 を参照・更新
 */
void ms_memmap_write8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	int local_addr = addr & 0x3fff;
	if ( local_addr >= 0x3ff0) {
		// Memory mapped DISK I/O
		switch(local_addr) {
		case 0x3ff8:
			// レジスタ2 を更新
			_TC8556AF_reg2_write(d, data);
			break;
		case 0x3ff9:
			// レジスタ3 を更新
			_TC8556AF_reg3_write(d, data);
			break;
		case 0x3ffa:
			// レジスタ4 を更新
			_TC8556AF_reg4_write(d, data);
			break;
		case 0x3ffb:
			// レジスタ5 を更新
			_TC8556AF_reg5_write(d, data);
			break;
		}
	}
}

uint16_t ms_memmap_read16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr) {
	// addr はページ境界をまたがないようになっているので気にせずOK
	return ms_memmap_read8_DISKBIOS_PANASONIC(driver, addr) | (ms_memmap_read8_DISKBIOS_PANASONIC(driver, addr + 1) << 8);
}

void ms_memmap_write16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_write8_DISKBIOS_PANASONIC(driver, addr, data & 0xff);
	ms_memmap_write8_DISKBIOS_PANASONIC(driver, addr + 1, data >> 8);
}
