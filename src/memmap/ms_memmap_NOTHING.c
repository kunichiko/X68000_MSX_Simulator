#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_NOTHING.h"

#define THIS ms_memmap_driver_NOTHING_t

char* driver_name_NOTHING = "NOTHING";

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
THIS* ms_memmap_NOTHING_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_NOTHING_init(THIS* instance, ms_memmap_t* memmap) {
	if (instance == NULL) {
		return;
	}
	uint8_t* buffer = (uint8_t*)new_malloc( 8*1024 );
	if(buffer == NULL) {
		printf("メモリが確保できません。\n");
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	instance->base.type = ROM_TYPE_NOTHING;
	instance->base.name = driver_name_NOTHING;
	//instance->base.deinit = ms_memmap_NOTHING_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	int i;
	for(i = 0; i<8*1024; i++) {
		instance->base.buffer[i] = 0xff;
	}

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer; // 重なっていて良い
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
	return 0xff;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	return 0xffff;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
}
