#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"

#define THIS ms_memmap_driver_MEGAROM_KONAMI_SCC_t

static char* driver_name = "MEGAROM_KONAMI_SCC";

static void _select_bank(THIS* d, int rom_page, int bank);
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
THIS* ms_memmap_MEGAROM_KONAMI_SCC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_MEGAROM_KONAMI_SCC_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MEGAROM_KONAMI_SCC;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MEGAROM_KONAMI_SCC_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k = 2; page8k < 6; page8k++) {
		_select_bank(instance, page8k, page8k-2);	// KONAMI SCCメガロムの場合、初期値は0,1,2,3
	}

	return;
}

void _did_attach(ms_memmap_driver_t* driver) {
}

int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static void _select_bank(THIS* d, int page8k, int segment) {
	if ( segment >= d->num_segments) {
		printf("MEGAROM_KONAMI_SCC: segment out of range: %d\n", segment);
		return;
	}
	d->base.page8k_pointers[page8k] = d->base.buffer + (segment * 0x2000);
	d->selected_segment[page8k] = segment;

	// 切り替えが起こったことを memmap に通知
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page8k);
	return;
}


static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	if( page8k < 2 || page8k > 5) {
		printf("MEGAROM_KONAMI_SCC: read out of range: %04x\n", addr);
		return 0xff;
	}
	int segment = d->selected_segment[page8k];
	if (segment >= d->num_segments) {
		printf("MEGAROM_KONAMI_SCC: segment out of range: %d\n", segment);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * segment);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MEGAROM_KONAMI_SCC: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

/*
	SCC無しのKONAMI_SCCメガロムの切り替え処理
	https://www.msx.org/wiki/MegaROM_Mappers#Konami_MegaROMs_with_SCC

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* 切り替えアドレス:	5000h (mirrors: 5001h~57FFh)
		* 初期セグメント	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* 切り替えアドレス	7000h (mirrors: 7001h~77FFh)
		* 初期セグメント	1
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* 切り替えアドレス	9000h (mirrors: 9001h~97FFh)
		* 初期セグメント	Random
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* 切り替えアドレス	b000h (mirrors: B001h~B7FFh)
		* 初期セグメント	Random
 */
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	// バンク切り替え処理
	int page8k = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x5*2:
			page8k = 2;
			break;
		case 0x7*2:
			page8k = 3;
			break;
		case 0x9*2:
			page8k = 4;
			break;
		case 0xb*2:
			page8k = 5;
			break;
	}
	if (page8k != -1) {
		_select_bank(d, page8k, data);
	}
	return;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}
