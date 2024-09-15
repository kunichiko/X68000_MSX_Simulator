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


/*
 * MSXの漢字ROMは大きく、第一水準と第二水準に分かれています。
 * 第一水準はポート0xD8,D9でアクセスし、第二水準はポート0xDA,DBでアクセスします。
 * この時に使用するアドレスは、「漢字番号」と呼ばれるもので、以下の計算式で求められます。
 * 
 * # 漢字番号
 * 漢字ROMの漢字番号は、JISの句点コードそのものではないため、変換が必要です。
 * MSX Datapack Volume 1 P.306の変換式を引用します。
 * * 第一水準: 1区-15区まで (非漢字領域。実際は10区以降は第一水準の漢字番号と被るので、8区までしか収録されていないと思われる)
 * 		* 区番号 * 96 + 点番号
 * * 第一水準: 16区-47区まで (漢字領域)
 * 		* 区番号 * 96 + 点番号 - 512
 * * 第二水準: 48区-84区まで
 * 		* (区番号 - 48) * 96 + 点番号
 * これによって出てきた漢字番号を「上位6bit」「下位6bit」に分けて、ポートに書き込むことで
 * 漢字ROMから漢字を読み出すことができます。
 * 
 * # ROMサイズ
 * 第一水準のROMは、47区94点が最後です。この漢字の漢字番号は、47*96 + 94 - 512 = 4094です。
 * 漢字1文字に32バイトが必要なので、4094 * 32 + 32 = 131040バイト = 0x1ffe0バイトです。
 * 
 * 第二水準は 0x20000バイトから始まります。84区94点が最後で、漢字番号は、(84-48) * 96 + 94 = 3550です。
 * 漢字1文字に32バイトが必要なので、3550 * 32 + 32 = 113632バイト = 0x1bfe0バイトです。
 * 
 * このように、それぞれ128KB、合計で256KBのROMが必要です。
 * 
 * # パターンの格納順
 * MSXの漢字ROMは、1文字が16x16ドットで、1文字あたり32バイトです。
 * 1文字は、左8bitと右8bitに分かれて格納されていて、並び順は、
 * * 左上
 * * 右上
 * * 左下
 * * 右下
 * の順に格納されています。
 */


// 第一水準

/**
 * @brief 第一水準ROMの漢字番号(前述)の下位アドレス6bit (addr1のbit10-bit5) をセットします。
 * 漢字ロムは1文字が16x16ドットで、1文字あたり32バイトです。
 * そのため、addr1の下位6bitは0x20ごとに切り替わります。
 * 
 * @param data 
 */
static void write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x1f800) | (((uint32_t)data & 0x3f) << 5);
}

/**
 * @brief 第一水準ROMの漢字番号(前述)の上位アドレス6bit (addr1のbit16-bit11) をセットします。
 * 
 * @param data 
 */
static void write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_D8(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
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

static void write_kanji_DB(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr2 = (instance->addr2 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_DA(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static uint8_t read_kanji_DB(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	if(instance->addr2 >= instance->rom_size) {
		return 0xff;
	}
	uint8_t ret = instance->rom_data[instance->addr2];
	// 上位下位アドレスはインクリメントせず、32バイト内でインクリメントする
	instance->addr2 = (instance->addr2 & 0x3ffe0) | ((instance->addr2 + 1) & 0x1f);
	return ret;
}