#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_NOTHING.h"

char* driver_name_NOTHING = "NOTHING";

/*
	確保 & 初期化ルーチン
 */
ms_memmap_driver_t* ms_memmap_NOTHING_init(ms_memmap_t* memmap) {
	ms_memmap_driver_NOTHING_t* instance;
	instance = (ms_memmap_driver_NOTHING_t*)new_malloc(sizeof(ms_memmap_driver_NOTHING_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_NOTHING;
	instance->base.name = driver_name_NOTHING;
	instance->base.deinit = ms_memmap_deinit_NOTHING;
	instance->base.did_attach = ms_memmap_did_attach_NOTHING;
	instance->base.will_detach = ms_memmap_will_detach_NOTHING;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_NOTHING;
	instance->base.read8 = ms_memmap_read8_NOTHING;
	instance->base.read16 = ms_memmap_read16_NOTHING;
	instance->base.write8 = ms_memmap_write8_NOTHING;
	instance->base.write16 = ms_memmap_write16_NOTHING;

	instance->base.buffer = (uint8_t*)new_malloc( 8*1024 );
	if(instance->base.buffer == NULL) {
		printf("メモリが確保できません。\n");
		ms_memmap_deinit_NOTHING((ms_memmap_driver_t*)instance);
		return NULL;
	}
	int i;
	for(i = 0; i<8*1024; i++) {
		instance->base.buffer[i] = 0xff;
	}

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer; // 重なっていて良い
	}

	return (ms_memmap_driver_t*)instance;
}

void ms_memmap_deinit_NOTHING(ms_memmap_driver_t* driver) {
	ms_memmap_driver_NOTHING_t* d = (ms_memmap_driver_NOTHING_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_NOTHING(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_NOTHING(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_NOTHING(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

uint8_t ms_memmap_read8_NOTHING(ms_memmap_driver_t* driver, uint16_t addr) {
	return 0xff;
}

void ms_memmap_write8_NOTHING(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
}

uint16_t ms_memmap_read16_NOTHING(ms_memmap_driver_t* driver, uint16_t addr) {
	return 0xffff;
}

void ms_memmap_write16_NOTHING(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
}
