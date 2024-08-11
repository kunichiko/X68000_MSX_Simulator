#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

extern ms_vdp_t ms_vdp;

ms_vdp_mode_t *ms_vdp_current_mode;

extern ms_vdp_mode_t ms_vdp_DEFAULT;
extern ms_vdp_mode_t ms_vdp_TEXT1;
extern ms_vdp_mode_t ms_vdp_TEXT2;
extern ms_vdp_mode_t ms_vdp_MULTICOLOR;
extern ms_vdp_mode_t ms_vdp_GRAPHIC1;
extern ms_vdp_mode_t ms_vdp_GRAPHIC2;
extern ms_vdp_mode_t ms_vdp_GRAPHIC3;
extern ms_vdp_mode_t ms_vdp_GRAPHIC4;
extern ms_vdp_mode_t ms_vdp_GRAPHIC5;
extern ms_vdp_mode_t ms_vdp_GRAPHIC6;
extern ms_vdp_mode_t ms_vdp_GRAPHIC7;
extern ms_vdp_mode_t ms_vdp_SCREEN10;
extern ms_vdp_mode_t ms_vdp_SCREEN12;

uint16_t * const X68_GR_PAL = (uint16_t *)0xE82000;
uint16_t * const X68_TX_PAL = (uint16_t *)0xE82200;
uint16_t * const X68_SP_PAL_B0 = (uint16_t *)0xE82200;	// ブロック0はテキストと共用
uint16_t * const X68_SP_PAL_B1 = (uint16_t *)0xE82220;	// ブロック1はスプライトパレットに使用
uint16_t * const X68_SP_PAL_B2 = (uint16_t *)0xE82240;	// ブロック2以降は使用していない

/* 
	画面モード一覧
set_CRT_jpt:
	.dc.l	set_GRAPHIC1	* 0x00
	.dc.l	set_TEXT1	* 0x01
	.dc.l	set_MULTICOLOR	* 0x02
	.dc.l	no_mode
	.dc.l	set_GRAPHIC2	* 0x04
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC3	* 0x08
	.dc.l	set_TEXT2	* 0x09
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC4	* 0x0c
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC5	* 0x10
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC6	* 0x14
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC7	* 0x1c
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
*/
ms_vdp_mode_t *ms_vdp_mode_table[32] = {
	&ms_vdp_GRAPHIC1,	// 0x00
	&ms_vdp_TEXT1,
	&ms_vdp_MULTICOLOR,
	NULL,
	&ms_vdp_GRAPHIC2,	// 0x04
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC3,	// 0x08
	&ms_vdp_TEXT2,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC4,	// 0x0c
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC5,	// 0x10
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC6,	// 0x14
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC7,	// 0x1c
	NULL,
	NULL,
	NULL
};

int ms_vdp_init_mac( void *);
void ms_vdp_deinit_mac(void);

int ms_vdp_init(void *vram_ptr) {
	// 画面モードの初期化
//	ms_vdp

	return ms_vdp_init_mac(vram_ptr);
}

void ms_vdp_deinit(void) {
	ms_vdp_deinit_mac();
}

/*
	VDPの画面モードをセットする
 */
void ms_vdp_set_mode(int mode) {
	ms_vdp_current_mode = ms_vdp_mode_table[mode];
	if (ms_vdp_current_mode == NULL) {
		ms_vdp_current_mode = &ms_vdp_DEFAULT;
	}
	ms_vdp_current_mode->init(&ms_vdp);
}