#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MAINRAM.h"

#define THIS ms_memmap_driver_MAINRAM_t

static char* driver_name = "MAINRAM";

static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

/*
	確保ルーチン
 */
THIS* ms_memmap_MAINRAM_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_MAINRAM_init(THIS* instance, ms_memmap_t* memmap) {
	if (instance == NULL) {
		return;
	}
	uint8_t* buffer = (uint8_t*)new_malloc( MAINRAM_SIZE );
	if(buffer == NULL) {
		printf("メモリが確保できません。\n");
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MAPPER_RAM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MAINRAM_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->num_segments = MAINRAM_SIZE / (16*1024);
	instance->selected_segment[0] = 3;
	instance->selected_segment[1] = 2;
	instance->selected_segment[2] = 1;
	instance->selected_segment[3] = 0;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k * 0x2000);
	}

	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

static int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
	THIS* d = (THIS*)driver;
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

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MAINRAM: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);
	driver->buffer[long_addr] = data;
	//printf("MAINRAM: write %04x[%06x] <- %02x\n", addr, long_addr, data);
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	uint16_t ret = driver->buffer[long_addr] | (driver->buffer[long_addr + 1] << 8);
	return ret;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	int slot = (addr >> 14) & 0x03;
	int long_addr = (addr & 0x3fff) + (0x4000 * d->selected_segment[slot]);

	// addr はページ境界をまたがないようになっているので気にせずOK
	driver->buffer[long_addr] = data & 0xff;
	driver->buffer[long_addr + 1] = (data >> 8) & 0xff;
}
