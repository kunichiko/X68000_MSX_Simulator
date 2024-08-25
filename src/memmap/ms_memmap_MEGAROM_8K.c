#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"

char* driver_name_MEGAROM_8K = "MEGAROM_8K";

void _select_bank_8K(ms_memmap_driver_MEGAROM_8K_t* d, int page, int bank);

/*
	確保 & 初期化ルーチン
 */
ms_memmap_driver_t* ms_memmap_MEGAROM_8K_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length) {
	ms_memmap_driver_MEGAROM_8K_t* instance;
	instance = (ms_memmap_driver_MEGAROM_8K_t*)new_malloc(sizeof(ms_memmap_driver_MEGAROM_8K_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_MEGAROM_8;
	instance->base.name = driver_name_MEGAROM_8K;
	instance->base.deinit = ms_memmap_deinit_MEGAROM_8K;
	instance->base.did_attach = ms_memmap_did_attach_MEGAROM_8K;
	instance->base.will_detach = ms_memmap_will_detach_MEGAROM_8K;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_MEGAROM_8K;
	instance->base.read8 = ms_memmap_read8_MEGAROM_8K;
	instance->base.read16 = ms_memmap_read16_MEGAROM_8K;
	instance->base.write8 = ms_memmap_write8_MEGAROM_8K;
	instance->base.write16 = ms_memmap_write16_MEGAROM_8K;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k=0;page8k<4;page8k++) {
		_select_bank_8K(instance, page8k, 0);	// ASCII 8Kメガロムの場合、初期値は0
	}
	return (ms_memmap_driver_t*)instance;
}

void ms_memmap_deinit_MEGAROM_8K(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_MEGAROM_8K(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_MEGAROM_8K(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_MEGAROM_8K(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

void _select_bank_8K(ms_memmap_driver_MEGAROM_8K_t* d, int page8k, int segment) {
	if ( segment >= d->num_segments) {
		printf("MEGAROM_8K: segment out of range: %d\n", segment);
		return;
	}
	d->base.page8k_pointers[page8k] = d->base.buffer + (segment * 0x2000);
	d->selected_segment[page8k] = segment;

	// 切り替えが起こったことを memmap に通知
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page8k);
	return;
}


uint8_t ms_memmap_read8_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	int slot_page = addr >> 13;
	if( slot_page < 2 || slot_page > 5) {
		printf("MEGAROM_8K: read out of range: %04x\n", addr);
		return 0xff;
	}
	slot_page -= 2;
	int bank = d->selected_segment[slot_page];
	if (bank > d->num_segments) {
		printf("MEGAROM_8K: bank out of range: %d\n", bank);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * bank);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MEGAROM_8K: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

uint16_t ms_memmap_read16_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	return ms_memmap_read8_MEGAROM_8K(driver, addr) | (ms_memmap_read8_MEGAROM_8K(driver, addr + 1) << 8);
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
void ms_memmap_write8_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	// バンク切り替え処理
	int rom_page = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x6*2+0:
			rom_page = 0;
			break;
		case 0x6*2+1:
			rom_page = 1;
			break;
		case 0x7*2+0:
			rom_page = 2;
			break;
		case 0x7*2+1:
			rom_page = 3;
			break;
	}
	if (rom_page != -1) {
		_select_bank_8K(d, rom_page, data);
	}
	return;
}

void ms_memmap_write16_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	ms_memmap_write8_MEGAROM_8K(driver, addr + 0, data & 0xff);
	ms_memmap_write8_MEGAROM_8K(driver, addr + 1, data >> 8);
	return;
}

