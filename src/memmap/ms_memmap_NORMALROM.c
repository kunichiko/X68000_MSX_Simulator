#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_NORMALROM.h"

#define THIS ms_memmap_driver_NORMALROM_t

char* driver_name_NORMALROM = "NORMALROM";

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
THIS* ms_memmap_NORMALROM_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_NORMALROM_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, int page) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_NORMAL_ROM;
	instance->base.name = driver_name_NORMALROM;
	//instance->base.deinit = ms_memmap_NORMALROM_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;


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

static void _did_attach(ms_memmap_driver_t* driver) {
}

static int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int local_addr = addr & 0x3fff;
	uint8_t ret = driver->buffer[local_addr];
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	// read only
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int local_addr = addr & 0x3fff;

	// addr はページ境界をまたがないようになっているので気にせずOK
	return driver->buffer[local_addr] | (driver->buffer[local_addr + 1] << 8);
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	// read only
}
