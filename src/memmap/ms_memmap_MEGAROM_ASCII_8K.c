#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MEGAROM_ASCII_8K.h"
#include "../ms.h"

#define THIS ms_memmap_driver_MEGAROM_ASCII_8K_t

static char* driver_name = "MEGAROM_ASCII_8K";

static void _select_bank(THIS* d, int region, int bank);
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
THIS* ms_memmap_MEGAROM_ASCII_8K_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_MEGAROM_ASCII_8K_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MEGAROM_ASCII_8K;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MEGAROM_ASCII_8K_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->base.buffer_length = length;
	instance->num_segments = length / 0x2000;

	int page_index;
	for(page_index = 0; page_index < 4; page_index++) {
		_select_bank(instance, page_index, 0);	// ASCII 8Kメガロムの場合、初期値は0
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

/**
 * @brief region (0-3)に見せるバンクを選択します
 * 
 * @param d 
 * @param page_index 
 * @param segment 
 */
static void _select_bank(THIS* d, int region, int segment) {
	uint8_t* buf;
	if ( segment >= d->num_segments) {
		printf("MEGAROM_ASCII_8K: segment out of range: %d\n", segment);
		buf = NULL;
	} else {
		buf = d->base.buffer + (segment * 0x2000);
	}

	d->selected_segment[region] = segment;
	d->base.page8k_pointers[(region+2)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+2)&0x7);	// 切り替えが起こったことを memmap に通知
	d->base.page8k_pointers[(region+6)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+6)&0x7);	// 切り替えが起こったことを memmap に通知

	return;
}


static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"MEGAROM_ASCII_8K: read out of range: %04x\n", addr);
		return 0xff;
	}

	uint8_t ret = p8k[local_addr];
	return ret;
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

/*
	ASCII 8Kメガロムの切り替え処理
	https://www.msx.org/wiki/MegaROM_Mappers#ASCII_8K

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* 切り替えアドレス:	6000h (mirrors: 6001h~67FFh)
		* 初期セグメント	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* 切り替えアドレス	6800h (mirrors: 6801h~6FFFh)
		* 初期セグメント	0
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* 切り替えアドレス	7000h (mirrors: 7001h~77FFh)
		* 初期セグメント	0
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* 切り替えアドレス	7800h (mirrors: 7801h~7FFFh)
		* 初期セグメント	0
 */
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	// バンク切り替え処理
	int region = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x6*2+0:
			region = 0;
			break;
		case 0x6*2+1:
			region = 1;
			break;
		case 0x7*2+0:
			region = 2;
			break;
		case 0x7*2+1:
			region = 3;
			break;
	}
	if (region != -1) {
		_select_bank(d, region, data);
	}
	return;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

