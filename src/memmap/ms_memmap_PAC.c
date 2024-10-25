/*
	PAC (Pana Amusement Pack、Panasoft SW-M001) のドライバ

	PACの SRAMアクセス方法 (ソフト側から見た挙動)

	* 1: 5FFEh、5FFFhが FFh であることを確認する
	* 2: 5FFEhに 4Dh を書き込んで、読み出して、4Dh であることを確認する
		* → 異なる値が読めたら、1で読み込んだ 5FFEhの値を書き戻して終了
	* 3: 5FFFhに 69h を書き込んで、読み出して、69h であることを確認する
		* → 異なる値が読めたら、1で読み込んだ 5FFEhの値を書き戻して終了
	* 4: 4000h-5FFDh の範囲がSRAM領域として利用可能になる
	* 5: 5FFEh, 5FFFh に 00h を書き込んで終了 (4Dh, 69h 以外が書き込まれればOK)
	* 6: 終了すると、5FFEh, 5FFFh から FFh が読めるようになる

	● FM-PACのSRAMバックアップファイルの仕様
	FM-PACにはメンテナンスツールがROMに内蔵されており、SRAMデータをファイルにバックアップすることができる。
	* 拡張子は .PAC
	* ファイルサイズは 8206 バイト
		* 16バイトヘッダ: "PAC2 BACKUP DATA"
		* 1024バイトのSRAMデータ x 7
		* 1022バイトのSRAMデータ x 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include "ms_memmap.h"
#include "ms_memmap_PAC.h"

#define THIS ms_memmap_driver_PAC_t

static char* driver_name = "PAC";

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
THIS* ms_memmap_PAC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_PAC_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, int buffer_length) {
	if (instance == NULL) {
		return;
	}

	if (buffer_length != 8206) {
		printf("PACのファイルサイズは 8206バイトである必要があります\n");
		return;
	}
	// bufferの先頭16バイトが "PAC2 BACKUP DATA" であることを確認
	if ( memcmp(buffer, "PAC2 BACKUP DATA", 16) != 0 ) {
		printf("PACのファイルヘッダが不正です\n");
		return;
	}	

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MIRRORED_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_PAC_deinit; オーバーライド不要
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
