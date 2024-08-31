#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_NORMALROM.h"

char* driver_name_NORMALROM = "NORMALROM";

/*
	確保ルーチン
 */
ms_memmap_driver_NORMALROM_t* ms_memmap_NORMALROM_alloc() {
	return (ms_memmap_driver_NORMALROM_t*)new_malloc(sizeof(ms_memmap_driver_NORMALROM_t));
}

/*
	初期化ルーチン
 */
void ms_memmap_NORMALROM_init(ms_memmap_driver_NORMALROM_t* instance, ms_memmap_t* memmap, uint8_t* buffer, int page) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_NORMAL_ROM;
	instance->base.name = driver_name_NORMALROM;
	//instance->base.deinit = ms_memmap_NORMALROM_deinit; オーバーライド不要
	instance->base.did_attach = ms_memmap_did_attach_NORMALROM;
	instance->base.will_detach = ms_memmap_will_detach_NORMALROM;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_NORMALROM;
	instance->base.read8 = ms_memmap_read8_NORMALROM;
	instance->base.read16 = ms_memmap_read16_NORMALROM;
	instance->base.write8 = ms_memmap_write8_NORMALROM;
	instance->base.write16 = ms_memmap_write16_NORMALROM;


	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		if ( page8k/2 == page ) {
			instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k%2)*0x2000;
		} else {
			instance->base.page8k_pointers[page8k] = NULL;
		}
	}
	
	return;
}

void ms_memmap_did_attach_NORMALROM(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_NORMALROM(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_NORMALROM(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

uint8_t ms_memmap_read8_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_NORMALROM_t* d = (ms_memmap_driver_NORMALROM_t*)driver;
	int local_addr = addr & 0x3fff;
	uint8_t ret = driver->buffer[local_addr];
	return ret;
}

void ms_memmap_write8_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	// read only
}

uint16_t ms_memmap_read16_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_NORMALROM_t* d = (ms_memmap_driver_NORMALROM_t*)driver;
	int local_addr = addr & 0x3fff;

	// addr はページ境界をまたがないようになっているので気にせずOK
	return driver->buffer[local_addr] | (driver->buffer[local_addr + 1] << 8);
}

void ms_memmap_write16_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	// read only
}
