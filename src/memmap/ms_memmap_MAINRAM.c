#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MAINRAM.h"

char* driver_name_MAINRAM = "MAINRAM";

/*
	確保 & 初期化ルーチン
 */
ms_memmap_driver_t* ms_memmap_MAINRAM_init(ms_memmap_t* memmap) {
	ms_memmap_driver_MAINRAM_t* instance;
	instance = (ms_memmap_driver_MAINRAM_t*)new_malloc(sizeof(ms_memmap_driver_MAINRAM_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_MAPPER_RAM;
	instance->base.name = driver_name_MAINRAM;
	instance->base.deinit = ms_memmap_deinit_MAINRAM;
	instance->base.did_attach = ms_memmap_did_attach_MAINRAM;
	instance->base.will_detach = ms_memmap_will_detach_MAINRAM;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_MAINRAM;
	instance->base.read8 = ms_memmap_read8_MAINRAM;
	instance->base.read16 = ms_memmap_read16_MAINRAM;
	instance->base.write8 = ms_memmap_write8_MAINRAM;
	instance->base.write16 = ms_memmap_write16_MAINRAM;

	instance->base.buffer = (uint8_t*)new_malloc( MAINRAM_SIZE );
	if(instance->base.buffer == NULL) {
		printf("メモリが確保できません。\n");
		ms_memmap_deinit_MAINRAM((ms_memmap_driver_t*)instance);
		return NULL;
	}
	//
	instance->num_segments = MAINRAM_SIZE / (16*1024);
	instance->selected_segment[0] = 0;
	instance->selected_segment[1] = 1;
	instance->selected_segment[2] = 2;
	instance->selected_segment[3] = 3;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k * 0x2000);
	}

	return (ms_memmap_driver_t*)instance;
}

void ms_memmap_deinit_MAINRAM(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_MAINRAM(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_MAINRAM(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_MAINRAM(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	if (page < 0 || page > 3) {
		printf("MAINRAM: page out of range: %d\n", page);
		return;
	}
	if (segment_num >= d->num_segments) {
		printf("MAINRAM: segment out of range: %d\n", segment_num);
		return;
	}
	d->base.page8k_pointers[page*2+0] = d->base.buffer + (segment_num * 0x4000);
	d->base.page8k_pointers[page*2+1] = d->base.buffer + (segment_num * 0x4000) + 0x2000;
	d->selected_segment[page] = segment_num;

	// 切り替えが起こったことを memmap に通知
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page*2+0);
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page*2+1);
}

uint8_t ms_memmap_read8_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MAINRAM: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

void ms_memmap_write8_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);
	driver->buffer[long_addr] = data;
	//printf("MAINRAM: write %04x[%06x] <- %02x\n", addr, long_addr, data);
}

uint16_t ms_memmap_read16_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	return driver->buffer[long_addr] | (driver->buffer[long_addr + 1] << 8);
}

void ms_memmap_write16_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MAINRAM_t* d = (ms_memmap_driver_MAINRAM_t*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	driver->buffer[long_addr] = data & 0xff;
	driver->buffer[long_addr + 1] = (data >> 8) & 0xff;
}
