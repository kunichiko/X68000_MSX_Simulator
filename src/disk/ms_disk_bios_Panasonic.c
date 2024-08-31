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
void ms_disk_bios_Panasonic_init(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}
	ms_memmap_driver_init(instance->base, memmap);

	instance->base.type = ROM_TYPE_MAPPER_RAM;
	instance->base.name = driver_name_DISKBIOS_PANASONIC;
	instance->base.deinit = ms_memmap_deinit_DISKBIOS_PANASONIC;
	instance->base.did_attach = ms_memmap_did_attach_DISKBIOS_PANASONIC;
	instance->base.will_detach = ms_memmap_will_detach_DISKBIOS_PANASONIC;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC;
	instance->base.read8 = ms_memmap_read8_DISKBIOS_PANASONIC;
	instance->base.read16 = ms_memmap_read16_DISKBIOS_PANASONIC;
	instance->base.write8 = ms_memmap_write8_DISKBIOS_PANASONIC;
	instance->base.write16 = ms_memmap_write16_DISKBIOS_PANASONIC;

	instance->base.buffer = buffer;
	instance->length = length;

	int page8k;
	for(page8k = 0; page8k < 4; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k * 0x2000);
	}
	for(; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	return;
}

void ms_memmap_deinit_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
}



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
	}
	uint8_t ret = driver->buffer[addr];
	return ret;
}

void ms_memmap_write8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
}

uint16_t ms_memmap_read16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	uint16_t ret = driver->buffer[long_addr] | (driver->buffer[long_addr + 1] << 8);
	return ret;
}

void ms_memmap_write16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	driver->buffer[long_addr] = data & 0xff;
	driver->buffer[long_addr + 1] = (data >> 8) & 0xff;
}
