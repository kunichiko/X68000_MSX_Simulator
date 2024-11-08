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
#include "../ms.h"
#include "ms_memmap.h"
#include "ms_memmap_PAC.h"

#define THIS ms_memmap_driver_PAC_t

static char* driver_name = "PAC";

static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);
static void _did_pause(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

static void _fflush(ms_memmap_driver_t* driver);

/*
	確保ルーチン
 */
THIS* ms_memmap_PAC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_PAC_init(ms_memmap_driver_PAC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t buf_length, uint32_t file_length, uint8_t* file_path) {
	if (instance == NULL) {
		return;
	}

	if (file_length != 8206) {
		MS_LOG(MS_LOG_ERROR,"PACのファイルサイズは 8206バイトである必要があります\n");
		return;
	}
	// bufferの先頭16バイトが "PAC2 BACKUP DATA" であることを確認
	if ( memcmp(buffer, "PAC2 BACKUP DATA", 16UL) != 0 ) {
		MS_LOG(MS_LOG_ERROR,"PACのファイルヘッダが不正です\n");
		return;
	}

	// データ長は16バイトのヘッダ+8192-2バイト=8206バイトだが、メモリ確保は8K単位で行われているので
	// bufferは16バイト+8192バイトであるとして扱っても問題ないはず
	if (buf_length < 16+8192) {
		MS_LOG(MS_LOG_ERROR,"PACのバッファサイズが不正です\n");
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MIRRORED_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_PAC_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_pause = _did_pause;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	// プライベートプロパティ
	memcpy(instance->file_path, file_path, 256);
	instance->sram_enabled = 0;
	instance->_5ffe = 0xff;
	instance->_5fff = 0xff;
	int i;
	for(i=0;i<8192;i++) {
		instance->disabled_buffer[i] = 0xff;
	}

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->disabled_buffer;
	}
	
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

/**
 * @brief ドライバのデタッチ時に呼び出されるコールバックで、PACのSRAMをファイルに書き出します
 * 
 * @param driver 
 * @return int 
 */
static int _will_detach(ms_memmap_driver_t* driver) {
	MS_LOG(MS_LOG_INFO, "PACのSRAMをファイルに書き出します\n");
	_fflush(driver);
	return 0;
}

/**
 * @brief ポーズ時に呼び出されるコールバック
 * 
 * @param driver 
 */
static void _did_pause(ms_memmap_driver_t* driver) {
	volatile uint8_t* BITSNS_WORK = (uint8_t*)0x800;
	if (BITSNS_WORK[0xe] & 1) {
		// ポーズ時に SHIFTキーが押されていたら、SRAMをファイルに書き出す
		_fflush(driver);
		printf("PACのSRAMをファイルに書き出しました\n");
	}
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	if(local_addr == 0x5ffe) {
		return d->_5ffe;
	} else if(local_addr == 0x5fff) {
		return d->_5fff;
	}
	uint8_t ret = driver->page8k_pointers[page8k][local_addr];
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	if(addr == 0x5ffe) {
		d->_5ffe = data;
	} else if(addr == 0x5fff) {
		d->_5fff = data;
	} else if (page8k == 2 && d->sram_enabled) {
		// SRAMが有効な時は page8k == 2 には SRAMがマッピングされている
		driver->page8k_pointers[2][local_addr] = data;
	}
	if(d->_5ffe == 0x4d && d->_5fff == 0x69) {
		if (d->sram_enabled == 0) {
			MS_LOG(MS_LOG_INFO,"PACのSRAMが開かれました\n");
			d->sram_enabled = 1;
			driver->page8k_pointers[2] = driver->buffer+16;
			// 切り替えが起こったことを memmap に通知
			d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, 2);
		}
	} else {
		if (d->sram_enabled != 0) {
			MS_LOG(MS_LOG_INFO,"PACのSRAMが閉じられたので、ファイルに書き出します\n");
			d->sram_enabled = 0;
			driver->page8k_pointers[2] = d->disabled_buffer;
			_fflush(driver);
			// 切り替えが起こったことを memmap に通知
			d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, 2);
		}
	}
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

/**
 * @brief PACのSRAMをファイルに書き出します
 * 
 * @param instance 
 */
void _fflush(ms_memmap_driver_t* driver) {
	THIS* instance = (THIS*)driver;
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( instance->file_path, O_RDWR | O_BINARY);
	if (crt_fh == -1) {
		MS_LOG(MS_LOG_ERROR,"PACファイルが開けません. %s\n", instance->file_path);
		return;
	}

	write(crt_fh, instance->base.buffer, 16+8192-2);

	close(crt_fh);
}
