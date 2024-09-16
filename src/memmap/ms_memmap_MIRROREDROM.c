/*
	16Kバイト/32KバイトのROMをミラーリングして配置するドライバ

	通常、16KバイトのROMはページ1に配置され、32KバイトのROMはページ1とページ2に配置されます。
	ただ、多くのROMカートリッジは、それぞれの内容が他のページにもミラーされて見えるようになっています。
	* 16KバイトROMの場合
		* 全てのページに同じ内容が見える
	* 32KバイトROMの場合
		* ページ0とページ2が同じ内容
		* ページ1とページ3が同じ内容

	本ドライバは、指定されたバッファサイズに合わせて、これらのミラーリングを行います。
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MIRROREDROM.h"

#define THIS ms_memmap_driver_MIRROREDROM_t

static char* driver_name = "MIRROREDROM";

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
THIS* ms_memmap_MIRROREDROM_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_MIRROREDROM_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, int buffer_length) {
	if (instance == NULL) {
		return;
	}

	if (buffer_length != 0x4000 && buffer_length != 0x8000) {
		printf("MIRROREDROMのバッファサイズは16Kバイトまたは32Kバイトである必要があります\n");
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MIRRORED_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MIRROREDROM_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	// プライベートプロパティ
	instance->buffer_length = buffer_length;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		if ( instance->buffer_length == 0x4000 ) {
			// 16KバイトROM
			instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k%2)*0x2000;
		} else {
			// 32KバイトROM
			int page = page8k / 2;
			if ( page % 2 == 0 ) {
				// 偶数ページ(0,2)
				instance->base.page8k_pointers[page8k] = instance->base.buffer + 0x4000 + (page8k%2)*0x2000;
			} else {
				// 奇数ページ(1,3)
				instance->base.page8k_pointers[page8k] = instance->base.buffer + 0x0000 + (page8k%2)*0x2000;
			}
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
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	uint8_t ret = driver->page8k_pointers[page8k][local_addr];
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	// read only
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	// addr はページ境界をまたがないようになっているので気にせずOK
	uint16_t ret = (driver->page8k_pointers[page8k][local_addr]) | (((uint16_t)driver->page8k_pointers[page8k][local_addr+1]) << 8);
	return ret;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	// read only
}
