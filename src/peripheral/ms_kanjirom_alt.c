/*
	X68000のCGROMの漢字を使用してMSXの漢字ROMをエミュレートするものです
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom_alt.h"

#define THIS ms_kanjirom_alt_t

#define CGROM_KANJI_KIGO	((uint16_t*)0xf00000)
#define CGROM_KANJI_LEVEL1	((uint16_t*)0xf05e00)
#define CGROM_KANJI_LEVEL2	((uint16_t*)0xf1d600)

static void _write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_kanji_D8(ms_ioport_t* ioport, uint8_t port);
static void _write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_kanji_D9(ms_ioport_t* ioport, uint8_t port);

THIS* ms_kanjirom_alt_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_kanjirom_alt_t*)new_malloc(sizeof(ms_kanjirom_alt_t))) == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}
	return instance;
}

void ms_kanjirom_alt_init(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}

	// 第一水準アドレス
	instance->k1_num = 0;
	instance->k1_line = 0;
	// 第二水準アドレス
	instance->k2_num = 0;
	instance->k2_line = 0;

	// I/O port アクセスを提供
	instance->io_port_D8.instance = instance;
	instance->io_port_D8.read = _read_kanji_D8;
	instance->io_port_D8.write = _write_kanji_D8;
	ms_iomap_attach_ioport(iomap, 0xd8, &instance->io_port_D8);

	instance->io_port_D9.instance = instance;
	instance->io_port_D9.read = _read_kanji_D9;
	instance->io_port_D9.write = _write_kanji_D9;
	ms_iomap_attach_ioport(iomap, 0xd9, &instance->io_port_D9);
}

void ms_kanjirom_alt_deinit(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap) {
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
 * 
 * # X68000のCGROMからの変換
 * X68000のCGROMには、以下のように3つの領域に分かれてフォントデータが格納されています。
 * Inside X68000 P.219の表を参考にします。
 * 
 * * 0xf00000-0xf05dff: 第一水準-非漢字領域	(1区-8区)
 * 		* JISコード上位: $21-$28 (1区-8区)
 * 		* JISコード下位: $21-$7e (1点-94点)
 * * 0xf05e00-0xf1d5ff: 第一水準-漢字領域 	(16区-47区)
 * 		* JISコード上位: $30-$4f (16区-47区)
 * 		* JISコード下位: $21-$7e
 * * 0xf1d600-0xf388bf: 第二水準-漢字領域	(48区-84区)
 * 		* JISコード上位: $50-$74 (48区-84区)
 * 		* JISコード下位: $21-$7e
 * 
 * # MSXの漢字ROMのチェックサム
 * MSXの漢字ROMは、正しい漢字ROMかどうかをチェックするために、以下のチェックサムを使います。
 * * 第一水準
 *  第一水準のROMは JISコード 2140H (1区32点)の左上の8x8領域が、以下のパターンになっているかをチェックします。
 * 
 *	00000000
 *	01000000
 *	00100000
 *	00010000
 *	00001000
 *	00000100
 *	00000010
 *	00000001
 *
 * * 第二水準
 * 第二水準のROMは JISコード 737EH (83区94点)の左上の8x8領域のチェクサムが 95H になっているかをチェックします。
 * 例:
 *  01 02 0c 37 c0 3b 2a 2a
 * 
 */

// 第一水準

/**
 * @brief 第一水準ROMの漢字番号(前述)の下位アドレス6bit (addr1のbit10-bit5) をセットします。
 * 漢字ロムは1文字が16x16ドットで、1文字あたり32バイトです。
 * そのため、addr1の下位6bitは0x20ごとに切り替わります。
 * 
 * @param data 
 */
static void _write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k1_num = (instance->k1_num & 0xffc0) | (data & 0x3f);	// 下位6bitを更新
	instance->k1_line = 0;
}

/**
 * @brief 第一水準ROMの漢字番号(前述)の上位アドレス6bit (addr1のbit16-bit11) をセットします。
 * 
 * @param data 
 */
static void _write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k1_num =  (((uint16_t)data & 0x3f) << 6) | (instance->k1_num & 0x003f);	// 上位6bitを更新
	instance->k1_line = 0;
}

static uint8_t _read_kanji_D8(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static uint8_t _read_kanji_D9(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	int knum = instance->k1_num;
	int line = instance->k1_line;
	int ku;
	int ten;
	uint16_t pattern;
	if( knum < 16*96-512 ) {
		// 非漢字領域
		ku = knum / 96 - 1;
		ten = knum % 96 - 1;
		pattern = CGROM_KANJI_KIGO[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
		if ( ku = 0 && ten == 31 && (line < 8)) {
			// 1区32点の左上の8x8領域
			uint8_t check[8] = {
				0b00000000,
				0b01000000,
				0b00100000,
				0b00010000,
				0b00001000,
				0b00000100,
				0b00000010,
				0b00000001
			};
			pattern = (check[line] << 8) | (pattern & 0xff);
		}
	} else {
		// 漢字領域
		knum += 512;
		ku = knum / 96 - 16;
		ten = knum % 96 - 1;
		pattern = CGROM_KANJI_LEVEL1[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
	}
	uint8_t ret = ((line & 8) == 0) ? (pattern >> 8) & 0xff : (pattern >> 0) & 0xff;
	// ライン番号をインクリメント
	instance->k1_line = (line + 1) % 32;
	return ret;
}


// 第二水準
static void _write_kanji_DA(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k2_num = (instance->k2_num & 0xffc0) | (data & 0x3f);	// 下位6bitを更新
	instance->k2_line = 0;
}

static void _write_kanji_DB(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k2_num =  ((uint16_t)data & 0x3f) << 6 | (instance->k2_num & 0x003f);	// 上位6bitを更新
	instance->k2_line = 0;
}

static uint8_t _read_kanji_DA(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}


static uint8_t _read_kanji_DB(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	int knum = instance->k2_num;
	int line = instance->k2_line;
	int ku;
	int ten;
	uint16_t pattern;

	ku = knum / 96;
	ten = knum % 96 - 1;
	pattern = CGROM_KANJI_LEVEL2[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
	if ( (ku == 35) && (ten == 93) && (line < 8)) {
		// 83区94点の左上の8x8領域
		// 区 = 83-48 = 35
		// 点 = 0x7e - 0x20 = 0x5e = 94
		// 漢字番号 = 35 * 96 + 94 = 3454 = 0xd7e
		//   上位6bit = 0x35
		//   下位6bit = 0x3e
		//
		// ku  = 3454 / 96 - 48 = 35
		// ten = 3454 % 96 -  1 = 93
		uint8_t check[8] = {
			0x01, 0x02, 0x0c, 0x37, 0xc0, 0x3b, 0x2a, 0x2a
		};
		pattern = (check[line] << 8) | (pattern & 0xff);
	}
	uint8_t ret = ((line & 8) == 0) ? (pattern >> 8) & 0xff : (pattern >> 0) & 0xff;
	// ライン番号をインクリメント
	instance->k2_line = (line + 1) % 32;
	return ret;
}


