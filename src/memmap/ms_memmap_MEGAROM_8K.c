#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"

int init_MEGAROM_8K(ms_memmap_driver_t* memmap, char* filename, int argc, char* argv[]);
void deinint_MEGAROM_8K(ms_memmap_driver_t* memmap);
uint8_t read8_MEGAROM_8K(ms_memmap_driver_t* memmap, uint16_t addr);
uint16_t read16_MEGAROM_8K(ms_memmap_driver_t* memmap, uint16_t addr);
void write8_MEGAROM_8K(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
void write16_MEGAROM_8K(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);

// 構造体を拡張し、プロパティを追加する
typedef struct ms_memmap_driver_MEGAROM_8K {
	ms_memmap_driver_t base;
	// extended properties
	int bank_size;
	int selected_bank[4];	// SLOT1前半、SLOT1後半、SLOT2前半、SLOT2後半の4つのバンクの選択状態
} ms_memmap_driver_MEGAROM_8K_t;

/*
	確保 & 初期化ルーチン
 */
ms_memmap_driver_t* ms_memmap_MEGAROM_8K_init(ms_memmap_t* memmap, char* filename, int argc, char* argv[]) {
	ms_memmap_driver_MEGAROM_8K_t* instance;
	instance = (ms_memmap_driver_MEGAROM_8K_t*)new_malloc(sizeof(ms_memmap_driver_MEGAROM_8K_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.read8 = read8_MEGAROM_8K;
	instance->base.read16 = read16_MEGAROM_8K;
	instance->base.write8 = write8_MEGAROM_8K;
	instance->base.write16 = write16_MEGAROM_8K;

	instance->base.mem_slot1 = (uint8_t*)new_malloc( 16*1024 + MS_MEMMAP_HEADER_LENGTH );
	if(instance->base.mem_slot1 == NULL) {
		printf("メモリが確保できません。\n");
		deinint_MEGAROM_8K((ms_memmap_driver_t*)instance);
		return NULL;
	}
	instance->base.mem_slot2 = (uint8_t*)new_malloc( 16*1024 + MS_MEMMAP_HEADER_LENGTH );
	if(instance->base.mem_slot2 == NULL) {
		printf("メモリが確保できません。\n");
		return NULL;
	}

	//
	int fh;
	int length;
	fh = open( filename, O_RDONLY | O_BINARY);
	if( fh == -1) {
		printf("ファイルが見つかりません。%s\n", filename);
		return NULL;
	}

	length = filelength(fh);
	if(length == -1) {
		printf("ファイルの長さが取得できません%s\n", filename);
		return NULL;
	}
	instance->base.buffer = (uint8_t*)new_malloc( length + MS_MEMMAP_HEADER_LENGTH );
	if(instance->base.buffer == NULL) {
		printf("メモリが確保できません。\n");
		return NULL;
	}
	read( fh, instance->base.buffer + MS_MEMMAP_HEADER_LENGTH, length);
	instance->bank_size = length / 0x2000;
	return (ms_memmap_driver_t*)instance;
}

void deinint_MEGAROM_8K(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	new_free(d->base.buffer);
	new_free(d->base.mem_slot2);
	new_free(d->base.mem_slot1);
	new_free(d);
}

uint8_t read8_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	int slot_page = addr >> 13;
	if( addr < 2 || addr > 5) {
		printf("MEGAROM_8K: read out of range: %04x\n", addr);
		return 0xff;
	}
	slot_page -= 2;
	int bank = d->selected_bank[slot_page];
	if (bank > d->bank_size) {
		printf("MEGAROM_8K: bank out of range: %d\n", bank);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * bank);
	return driver->buffer[long_addr];
}

uint16_t read16_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	return read8_MEGAROM_8K(driver, addr) | (read8_MEGAROM_8K(driver, addr + 1) << 8);
}

void write8_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	// バンク切り替え処理
	int page = -1;
	int slot;
	if(addr == 0x6800) {
		page = 0;
		slot = 1;
	}
	if(addr == 0x7000) {
		page = 1;
		slot = 1;
	}
	if(addr == 0x7000) {
		page = 2;
		slot = 2;
	}
	if(addr == 0x7800) {
		page = 3;
		slot = 2;
	}
	if (page != -1) {
		d->selected_bank[page] = data % d->bank_size;
		// バンク切り替え処理(メモリコピー)
		int i;
		for(i=0;i<0x2000;i++) {
			if(slot == 1) {
				driver->mem_slot1[(page%2)*0x2000 + i] = driver->buffer[data*0x2000 + i];
			} else {
				driver->mem_slot2[(page%2)*0x2000 + i] = driver->buffer[data*0x2000 + i];
			}
		}
		return;
	}
	return;
}

void write16_MEGAROM_8K(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MEGAROM_8K_t* d = (ms_memmap_driver_MEGAROM_8K_t*)driver;
	write8_MEGAROM_8K(driver, addr + 0, data & 0xff);
	write8_MEGAROM_8K(driver, addr + 1, data >> 8);
	return;
}
