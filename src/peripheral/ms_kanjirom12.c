#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom12.h"

#define THIS ms_kanjirom12_t

static void write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_kanji_D8(ms_ioport_t* ioport, uint8_t port);
static void write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_kanji_D9(ms_ioport_t* ioport, uint8_t port);

THIS* ms_kanjirom12_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_kanjirom12_t*)new_malloc(sizeof(ms_kanjirom12_t))) == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}
	return instance;
}

void ms_kanjirom12_init(ms_kanjirom12_t* instance, ms_iomap_t* iomap, char* rom_path) {
	if (instance == NULL) {
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

	// I/O port アクセスを提供
	instance->io_port_D8.instance = instance;
	instance->io_port_D8.read = read_kanji_D8;
	instance->io_port_D8.write = write_kanji_D8;
	ms_iomap_attach_ioport(iomap, 0xd8, &instance->io_port_D8);

	instance->io_port_D9.instance = instance;
	instance->io_port_D9.read = read_kanji_D9;
	instance->io_port_D9.write = write_kanji_D9;
	ms_iomap_attach_ioport(iomap, 0xd9, &instance->io_port_D9);
}

void ms_kanjirom12_deinit(ms_kanjirom12_t* instance, ms_iomap_t* iomap) {
	if (instance->rom_data != NULL) {
		new_free(instance->rom_data);
	}
	ms_iomap_detach_ioport(iomap, 0xd8);
	ms_iomap_detach_ioport(iomap, 0xd9);
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
static void write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x1f800) | (((uint32_t)data & 0x3f) << 5);
}

static uint8_t read_kanji_D8(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static void write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_D9(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	if(instance->addr1 >= instance->rom_size) {
		return 0xff;
	}
	uint8_t ret = instance->rom_data[instance->addr1];
	// 上位下位アドレスはインクリメントせず、32バイト内でインクリメントする
	instance->addr1 = (instance->addr1 & 0x1ffe0) | ((instance->addr1 + 1) & 0x1f);
	return ret;
}


// 第二水準
static void write_kanji_DA(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr2 = (instance->addr2 & 0x3f800) | (((uint32_t)data & 0x3f) << 5);
}

static uint8_t read_kanji_DA(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static void write_kanji_DB(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr2 = (instance->addr2 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_DB(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	if(instance->addr2 >= instance->rom_size) {
		return 0xff;
	}
	uint8_t ret = instance->rom_data[instance->addr2];
	// 上位下位アドレスはインクリメントせず、32バイト内でインクリメントする
	instance->addr2 = (instance->addr2 & 0x3f800) | ((instance->addr2 + 1) & 0x1f);
	return ret;
}