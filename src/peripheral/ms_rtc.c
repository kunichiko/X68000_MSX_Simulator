#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_rtc.h"

#define THIS ms_rtc_t

// I/O port
static void write_rtc_B4(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_rtc_B4(ms_ioport_t* ioport, uint8_t port);
static void write_rtc_B5(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_rtc_B5(ms_ioport_t* ioport, uint8_t port);

#define RTC_X68K ((volatile uint16_t *)0xe8a000) // RTCレジスタ

uint8_t read_rtc_datetime(uint8_t regnum);
void write_rtc_datetime(uint8_t regnum, uint8_t data);

THIS* ms_rtc_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_rtc_t*)new_malloc(sizeof(ms_rtc_t))) == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}
	return instance;
}

void ms_rtc_init(ms_rtc_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}

	instance->regnum = 0;
	instance->r13 = 0;
	instance->r14 = 0;
	instance->r15 = 0;

	int i;
	for (i = 0; i < 13; i++) {
		instance->block2[i] = 0;
		instance->block3[i] = 0;
	}

	// MSXでバッテリバックアップされている設定値をデフォルト値にしておく
	// TODO: ファイルに永続化できるようにする
	instance->block2[0] = 10;		// 保存値が有効な場合は10になる
	instance->block2[1] = 0x00;	// X-Adjust
	instance->block2[2] = 0x00;	// Y-Adjust
	instance->block2[3] = 0x01;	// SCREEN MODE
	instance->block2[4] = 0x00;	// SCREEN WIDTH (下位4bit)
	instance->block2[5] = 0x02;	// SCREEN WIDTH (上位3bit)
	instance->block2[6] = 0x0f;	// TEXT COLOR
	instance->block2[7] = 0x04;	// BACKGROUND COLOR
	instance->block2[8] = 0x07;	// BORDER COLOR
	instance->block2[9] = 0x01;	// b3:ボーレート, b2:プリンタ種別, b1:キークリック, b0:キーON/OFF
	instance->block2[10] = 0x00;	// b3-2:BEEP timbre, b1-0:BEEP volume
	instance->block2[11] = 0x00;	// b1-0:起動ロゴの色
	instance->block2[12] = 0x00;	// エリアコード(0:日本, 1:米国, etc)

	// I/O port アクセスを提供
	instance->io_port_B4.instance = instance;
	instance->io_port_B4.read = read_rtc_B4;
	instance->io_port_B4.write = write_rtc_B4;
	ms_iomap_attach_ioport(iomap, 0xb4, &instance->io_port_B4);

	instance->io_port_B5.instance = instance;
	instance->io_port_B5.read = read_rtc_B5;
	instance->io_port_B5.write = write_rtc_B5;
	ms_iomap_attach_ioport(iomap, 0xb5, &instance->io_port_B5);
}


void ms_rtc_deinit(ms_rtc_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}
	ms_iomap_detach_ioport(iomap, 0xb4);
	ms_iomap_detach_ioport(iomap, 0xb5);
}


// I/O port

// 
// Ｒｅａｌ Ｔｉｍｅ Ｃｌｏｃｋ
// 
//  Port B4: レジスタ番号指定
//  Port B5: レジスタの読み書き
// 
// [MSXのRTCレジスタは RICOH RP-5C01](https://www.msx.org/wiki/Ricoh_RP-5C01)で、
// 4つのブロックに15個ずつのレジスタがある(各レジスタは4bit幅)。
// ただし、レジスタ13-15はどのブロックでも同じものが見えているので、総レジスタ数は 3+12*4=51。
// 
// * ブロック共通: 13-15
//    * レジスタ13: モードレジスタ
// * ブロック0: 0-12
//    * 時計の年月日時分秒が格納されている
// * ブロック1: 0-12
//    * アラーム時刻が格納されているが、MSXでは使用していない
// * ブロック2: 0-12
//    * MSXの設定値(画面の色やSET ADJUSTの値など)が格納されている

static uint8_t read_rtc_B4(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	// レジスタ番号を返す
	return instance->regnum;
}

static void write_rtc_B4(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	// レジスタ番号をセット
	instance->regnum = data;
}

static uint8_t read_rtc_B5(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	// レジスタの読み出し
	switch(instance->regnum) {
	case 13:
		// Mode register
		// b3: Timer Enable
		// b2: Alarm Enable
		// b1-2: Block number
		return instance->r13;
	case 14:
		// Test register
		return instance->r14;
	case 15:
		// Reset controller
		return instance->r15;
	default:
		// レジスタ0-12はブロックによって異なる
		switch(instance->r13 & 0x03) {
		case 0:
			return read_rtc_datetime(instance->regnum);
		case 1:
			return 0xff;
		case 2:
			return instance->block2[instance->regnum];
		case 3:
			return instance->block3[instance->regnum];
		default:
			break;
		}
	}
	return 0xff;
}

static void write_rtc_B5(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	// レジスタの書き込み
	switch(instance->regnum) {
	case 13:
		instance->r13 = data;
		break;
	case 14:
		instance->r14 = data;
		break;
	case 15:
		instance->r15 = data;
		break;
	default:
		// レジスタ0-12はブロックによって異なる
		switch(instance->r13 & 0x03) {
		case 0:
			write_rtc_datetime(instance->regnum, data);
			break;
		case 1:
			break;
		case 2:
			instance->block2[instance->regnum] = data;
			break;
		case 3:
			instance->block3[instance->regnum] = data;
			break;
		default:
			break;
		}
	}
}

/**
 * @brief RTCの年月日時分秒を読み出します
 * 
 * 指定されたレジスタの値に相当する値をX68000のRTCから年月日時分秒を読み出し、
 * その値を返します。
 * 
 * @param regnum MSXのRTCのレジスタ番号(0-12) (ブロック0前提)
 * @return uint8_t 読み出した値
 */
uint8_t read_rtc_datetime(uint8_t regnum) {
	// X68000のRTCとレジスタの並びは同じだが、X68000側はワードアクセスが必要
	// ただ、Cのポインタを uint16_t 型にしてあるので、そのまま番号を渡せば読み出せる
	return RTC_X68K[regnum] & 0x0f;	 // 下位4bitだけ有効
}

void write_rtc_datetime(uint8_t regnum, uint8_t data) {
	// 日付の書き込みは危ないので実装しない
}
