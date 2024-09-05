#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom12.h"

// Singleton instance
ms_kanjirom12_t* ms_kanjirom12_shared = NULL;
uint8_t ms_kanjirom12shared_initialized = 0;

ms_kanjirom12_t* ms_kanjirom12_alloc() {
	if (ms_kanjirom12_shared != NULL) {
		return ms_kanjirom12_shared;
	}
	if( (ms_kanjirom12_shared = (ms_kanjirom12_t*)new_malloc(sizeof(ms_kanjirom12_t))) == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}
	return ms_kanjirom12_shared;
}

void ms_kanjirom12_init(ms_kanjirom12_t* instance, char* rom_path) {
	if (instance == NULL || ms_kanjirom12shared_initialized) {
		return;
	}
	instance->rom_path = rom_path;
	instance->rom_data = NULL;
	// rom_path のファイルを読み込む
	int fh = open(rom_path, O_RDONLY | O_BINARY);
	if (fh == -1) {
		printf("ファイルが開けません. %s\n", rom_path);
		return;
	}
	int length = filelength(fh);
	if (length == -1) {
		printf("ファイルの長さが取得できません。\n");
		close(fh);
		return;
	}
	instance->rom_data = (uint8_t*)new_malloc(length);
	if (instance->rom_data == NULL) {
		printf("メモリが確保できません。\n");
		close(fh);
		return;
	}
	read(fh, instance->rom_data, length);
	close(fh);
	instance->rom_size = length;

	// 第一水準アドレス
	instance->addr1 = 0x00000;
	// 第二水準アドレス
	instance->addr2 = 0x20000;

	ms_kanjirom12shared_initialized = 1;
}

void ms_kanjirom12_deinit(ms_kanjirom12_t* instance) {
	if (instance->rom_data != NULL) {
		new_free(instance->rom_data);
	}
}


// I/O port


// 第一水準

/**
 * @brief 第一水準ROMの下位アドレス6bit (addr1のbit10-bit5) をセットします
 * 漢字ロムは1文字が16x16ドットで、1文字あたり32バイトです。
 * そのため、addr1の下位6bitは0x20ごとに切り替わります。
 * 上位アドレスと下位アドレスはそれぞれJIS漢字コードのコード表の縦横(区と番号)に対応していて、
 * 6bitずつ割り当てられています。
 * @param data 
 */
void write_kanji_D8(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x1f800) | (((uint32_t)data & 0x3f) << 5);
}

void write_kanji_D9(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

uint8_t read_kanji_D9(uint8_t port) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	if(ms_kanjirom12_shared->addr1 >= ms_kanjirom12_shared->rom_size) {
		return 0xff;
	}
	uint8_t ret = ms_kanjirom12_shared->rom_data[ms_kanjirom12_shared->addr1];
	// 上位下位アドレスはインクリメントせず、32バイト内でインクリメントする
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x1ffe0) | ((ms_kanjirom12_shared->addr1 + 1) & 0x1f);
	return ret;
}


// 第二水準
void write_kanji_DA(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x3f800) | (((uint32_t)data & 0x3f) << 5);
}

void write_kanji_DB(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

uint8_t read_kanji_DB(uint8_t port) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	if(ms_kanjirom12_shared->addr2 >= ms_kanjirom12_shared->rom_size) {
		return 0xff;
	}
	uint8_t ret = ms_kanjirom12_shared->rom_data[ms_kanjirom12_shared->addr2];
	// 上位下位アドレスはインクリメントせず、32バイト内でインクリメントする
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x3f800) | ((ms_kanjirom12_shared->addr2 + 1) & 0x1f);
	return ret;
}