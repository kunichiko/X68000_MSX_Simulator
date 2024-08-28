#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"

char* driver_name_MEGAROM_KONAMI_SCC = "MEGAROM_KONAMI_SCC";

void _select_bank_KONAMI_SCC(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d, int rom_page, int bank);

/*
	確保 & 初期化ルーチン
 */
ms_memmap_driver_t* ms_memmap_MEGAROM_KONAMI_SCC_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* instance;
	instance = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)new_malloc(sizeof(ms_memmap_driver_MEGAROM_KONAMI_SCC_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_MEGAROM_KONAMI_SCC;
	instance->base.name = driver_name_MEGAROM_KONAMI_SCC;
	instance->base.deinit = ms_memmap_deinit_MEGAROM_KONAMI_SCC;
	instance->base.did_attach = ms_memmap_did_attach_MEGAROM_KONAMI_SCC;
	instance->base.will_detach = ms_memmap_will_detach_MEGAROM_KONAMI_SCC;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_MEGAROM_KONAMI_SCC;
	instance->base.read8 = ms_memmap_read8_MEGAROM_KONAMI_SCC;
	instance->base.read16 = ms_memmap_read16_MEGAROM_KONAMI_SCC;
	instance->base.write8 = ms_memmap_write8_MEGAROM_KONAMI_SCC;
	instance->base.write16 = ms_memmap_write16_MEGAROM_KONAMI_SCC;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k = 2; page8k < 6; page8k++) {
		_select_bank_KONAMI_SCC(instance, page8k, page8k-2);	// KONAMI SCCメガロムの場合、初期値は0,1,2,3
	}

	return (ms_memmap_driver_t*)instance;
}

void ms_memmap_deinit_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

void _select_bank_KONAMI_SCC(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d, int page8k, int segment) {
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


uint8_t ms_memmap_read8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
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

uint16_t ms_memmap_read16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	return ms_memmap_read8_MEGAROM_KONAMI_SCC(driver, addr) | (ms_memmap_read8_MEGAROM_KONAMI_SCC(driver, addr + 1) << 8);
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
void ms_memmap_write8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	// バンク切り替え処理
	int page8k = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x5*2:
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
		_select_bank_KONAMI_SCC(d, page8k, data);
	}
	return;
}

void ms_memmap_write16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	ms_memmap_write8_MEGAROM_KONAMI_SCC(driver, addr + 0, data & 0xff);
	ms_memmap_write8_MEGAROM_KONAMI_SCC(driver, addr + 1, data >> 8);
	return;
}
