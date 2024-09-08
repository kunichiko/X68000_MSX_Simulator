#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_rtc.h"

#define THIS ms_rtc_t

#define RTC_X68K ((volatile uint16_t *)0xe8a000) // RTCレジスタ

uint8_t read_rtc_datetime(uint8_t regnum);
void write_rtc_datetime(uint8_t regnum, uint8_t data);

// Singleton instance
THIS* ms_rtc_shared = NULL;
uint8_t ms_rtcshared_initialized = 0;

THIS* ms_rtc_alloc() {
	if (ms_rtc_shared != NULL) {
		return ms_rtc_shared;
	}
	if( (ms_rtc_shared = (THIS*)new_malloc(sizeof(THIS))) == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}

	ms_rtc_shared->regnum = 0;
	ms_rtc_shared->r13 = 0;
	ms_rtc_shared->r14 = 0;
	ms_rtc_shared->r15 = 0;

	// MSXでバッテリバックアップされている設定値をデフォルト値にしておく
	// TODO: ファイルに永続化できるようにする
	ms_rtc_shared->block2[0] = 10;		// 保存値が有効な場合は10になる
	ms_rtc_shared->block2[1] = 0x00;	// X-Adjust
	ms_rtc_shared->block2[2] = 0x00;	// Y-Adjust
	ms_rtc_shared->block2[3] = 0x01;	// SCREEN MODE
	ms_rtc_shared->block2[4] = 0x00;	// SCREEN WIDTH (下位4bit)
	ms_rtc_shared->block2[5] = 0x02;	// SCREEN WIDTH (上位3bit)
	ms_rtc_shared->block2[6] = 0x0f;	// TEXT COLOR
	ms_rtc_shared->block2[7] = 0x04;	// BACKGROUND COLOR
	ms_rtc_shared->block2[8] = 0x07;	// BORDER COLOR
	ms_rtc_shared->block2[9] = 0x01;	// b3:ボーレート, b2:プリンタ種別, b1:キークリック, b0:キーON/OFF
	ms_rtc_shared->block2[10] = 0x00;	// b3-2:BEEP timbre, b1-0:BEEP volume
	ms_rtc_shared->block2[11] = 0x00;	// b1-0:起動ロゴの色
	ms_rtc_shared->block2[12] = 0x00;	// エリアコード(0:日本, 1:米国, etc)

	return ms_rtc_shared;
}

void ms_rtc_init(THIS* instance) {
	if (instance == NULL || ms_rtcshared_initialized) {
		return;
	}
	int i;
	for (i = 0; i < 13; i++) {
		instance->block2[i] = 0;
		instance->block3[i] = 0;
	}

	ms_rtcshared_initialized = 1;
}

void ms_rtc_deinit(THIS* instance) {
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

uint8_t read_rtc_B4(uint8_t port) {
	if(ms_rtc_shared == NULL || !ms_rtcshared_initialized) {
		return;
	}
	// レジスタ番号を返す
	return ms_rtc_shared->regnum;
}

void write_rtc_B4(uint8_t port, uint8_t data) {
	if(ms_rtc_shared == NULL || !ms_rtcshared_initialized) {
		return;
	}
	// レジスタ番号をセット
	ms_rtc_shared->regnum = data;
}

uint8_t read_rtc_B5(uint8_t port) {
	if(ms_rtc_shared == NULL || !ms_rtcshared_initialized) {
		return;
	}
	// レジスタの読み出し
	switch(ms_rtc_shared->regnum) {
	case 13:
		// Mode register
		// b3: Timer Enable
		// b2: Alarm Enable
		// b1-2: Block number
		return ms_rtc_shared->r13;
	case 14:
		// Test register
		return ms_rtc_shared->r14;
	case 15:
		// Reset controller
		return ms_rtc_shared->r15;
	default:
		// レジスタ0-12はブロックによって異なる
		switch(ms_rtc_shared->r13 & 0x03) {
		case 0:
			return read_rtc_datetime(ms_rtc_shared->regnum);
		case 1:
			return 0xff;
		case 2:
			return ms_rtc_shared->block2[ms_rtc_shared->regnum];
		case 3:
			return ms_rtc_shared->block3[ms_rtc_shared->regnum];
		default:
			break;
		}
	}
	return 0xff;
}

void write_rtc_B5(uint8_t port, uint8_t data) {
	if(ms_rtc_shared == NULL || !ms_rtcshared_initialized) {
		return;
	}
	// レジスタの書き込み
	switch(ms_rtc_shared->regnum) {
	case 13:
		ms_rtc_shared->r13 = data;
		break;
	case 14:
		ms_rtc_shared->r14 = data;
		break;
	case 15:
		ms_rtc_shared->r15 = data;
		break;
	default:
		// レジスタ0-12はブロックによって異なる
		switch(ms_rtc_shared->r13 & 0x03) {
		case 0:
			write_rtc_datetime(ms_rtc_shared->regnum, data);
			break;
		case 1:
			break;
		case 2:
			ms_rtc_shared->block2[ms_rtc_shared->regnum] = data;
			break;
		case 3:
			ms_rtc_shared->block3[ms_rtc_shared->regnum] = data;
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
