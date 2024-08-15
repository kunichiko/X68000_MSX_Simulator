#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

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

int ms_vdp_init_mac(ms_vdp_t* vdp);
void ms_vdp_deinit_mac(ms_vdp_t* vdp);
void ms_vdp_init_sprite(ms_vdp_t* vdp);

// Singleton instance
ms_vdp_t* ms_vdp_shared = NULL;

ms_vdp_t* ms_vdp_init() {
	if( ms_vdp_shared != NULL ) {
		return ms_vdp_shared;
	}

	if ( (ms_vdp_shared = (ms_vdp_t*)new_malloc(sizeof(ms_vdp_t))) >= (ms_vdp_t *)0x81000000)
	{
		printf("メモリが確保できません\n");
		return NULL;
	}
	if ( (ms_vdp_shared->vram = (uint8_t*)new_malloc(0x20000)) >= (uint8_t *)0x81000000)
	{
		printf("メモリが確保できません\n");
		return NULL;
	}
	// X68000は 1スプライト(16x16)パターンあたり128バイトが必要
	// MSXは 256個定義できるが、X68000は128個しか定義できないため、メモリ上に定義領域を作っておき
	// 表示時に転送するようにしている
	if ( (ms_vdp_shared->x68_pcg_buffer = (unsigned int*)new_malloc( 256 * 32 * sizeof(unsigned int))) >= (unsigned int *)0x81000000)
	{
		printf("メモリが確保できません\n");
		return NULL;
	}

	// 初期画面モードを 512x512にする
	// 実際には、MSXの画面モードに応じてこの後色々変化する
	_iocs_crtmod(4);	// 512x512, 31kHz, 16色 4枚
	_iocs_g_clr_on();	// グラフィックス画面初期化

	ms_vdp_init_mac(ms_vdp_shared);

	ms_vdp_init_sprite(ms_vdp_shared);

	update_resolution_COMMON(ms_vdp_shared, 1, 0);

	// GRAMクリア
	int i;
	for(i=0;i<X68_GRAM_LEN;i++) {
		X68_GRAM[i] = 0;
	}
	// VRAMクリア
	for(i=0;i<0x20000;i++) {
		ms_vdp_shared->vram[i] = 0;
	}

	return ms_vdp_shared;
}

void ms_vdp_deinit(ms_vdp_t* vdp) {
	ms_vdp_deinit_mac(ms_vdp_shared);
	new_free(vdp->x68_pcg_buffer);
	new_free(vdp->vram);
}

/*
	VDPの画面モードをセットする
 */
void ms_vdp_set_mode(ms_vdp_t* vdp, int mode) {
	vdp->ms_vdp_current_mode = ms_vdp_mode_table[mode];
	if (vdp->ms_vdp_current_mode == NULL) {
		vdp->ms_vdp_current_mode = &ms_vdp_DEFAULT;
	}
	vdp->ms_vdp_current_mode->update_resolution(vdp);
	vdp->ms_vdp_current_mode->init(vdp);
	printf("VDP Mode: %s\n", vdp->ms_vdp_current_mode->get_mode_name(vdp));
}


uint16_t crtc_values[4][13] = {
	// 256x192, 60Hz
	{	45, 4,  6, 38, 524, 5, 40, 424, 25, // CRTCレジスタ0-8
		0xff,   6+4,           40, // スプライトコントローラ画面モードレジスタ
		8*8	// テキスト画面のオフセット値
	},
	// 256x212, 60Hz
	{	45, 4,  6, 38, 524, 5, 52, 476, 27, // CRTCレジスタ0-8
		0xff,   6+4,           52, // スプライトコントローラ画面モードレジスタ
		8*5+4	// テキスト画面のオフセット値
	},
	// 512x384, 60Hz
	{	91, 9, 17, 81, 524, 5, 40, 424, 25, // CRTCレジスタ0-8
		0xff,  17+4,           40, // スプライトコントローラ画面モードレジスタ
		16*8	// テキスト画面のオフセット値
	},
	// 512x424, 60Hz
	{	91, 9, 17, 81, 524, 5, 52, 476, 27, // CRTCレジスタ0-8
		0xff,  17+4,           52, // スプライトコントローラ画面モードレジスタ
		16*5+8	// テキスト画面のオフセット値
	}
};

/**
 * @brief Set the display resolution
 * 
 * @param vdp 
 * @param res 0=256ドット, 1=512ドット
 * @param color 0=16色, 1=256色, 3=65536色
 */
void update_resolution_COMMON(ms_vdp_t* vdp, int res, int color) {
	// lines 0=192ライン, 1=212ライン (MSX換算)
	int lines = (vdp->r09 & 0x80) >> 7;
	int m = res * 2 + lines;

	CRTR_00	= crtc_values[m][0];
	CRTR_01	= crtc_values[m][1];
	CRTR_02	= crtc_values[m][2];
	CRTR_03	= crtc_values[m][3];
	CRTR_04	= crtc_values[m][4];
	CRTR_05	= crtc_values[m][5];
	CRTR_06	= crtc_values[m][6];
	CRTR_07	= crtc_values[m][7];
	CRTR_08	= crtc_values[m][8];
	CRTR_20 = ((color&0x3) << 10) | 0x10 | ((res&0x1) << 2) | (res&0x1);
	SPCON_HTOTAL = crtc_values[m][9];
	SPCON_HDISP = crtc_values[m][10];
	SPCON_VSISP = crtc_values[m][11];
	SPCON_RES = 0x10 | ((res&0x1) << 2) | (res&0x1);

	// ビデオコントロールレジスタの色設定
	VCRR_00 = (color&0x3);

	// テキスト画面のスクロール位置補正
	CRTR_11 = crtc_values[m][12];
}

/*
 	スプライトの処理

	MS.Xは、MSXの256ドットをX68000の512ドットに拡大している
	そのため、MSXの8x8ドットのスプライトは、X68000上は16x16ドットになる
	MSXのスプライトパターンは最大256個定義することができるが、
	X68000は16x16ドットのスプライトパターンを最大128個しか定義できず、数が足りない。
	そこで、MSXのスプライトは最大32個しか表示できないことを利用し、
	X68000のスプライトパターンは、現在表示中のスプライトのみを定義することにする。
	 
 */

unsigned short* X68_SP_PALETTE = (unsigned short*)0x00e82200; // スプライト/テキストパレット
unsigned short* X68_SSR = (unsigned short*)0x00eb0000; // スプライトスクロールレジスタ
unsigned int* X68_PCG = (unsigned int*)0x00eb8000;

/*
 プライトの初期化
 */
void ms_vdp_init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCGバッファの初期化
	for ( i = 0; i < 256 * 32; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;
}

/*
 スプライトパターンジェネレータテーブルへの書き込み
     offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込むパターン(下位8bitのみ使用)
*/
void writeSpritePattern(ms_vdp_t* vdp, int offset, unsigned int pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	unsigned int pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドットを X68000の8x8のパターン2つに変換

    // 右端のドットから処理
	for(i =0; i < 4;i++) {
		pRight >>= 8;
		if(pattern & 1) {
			pRight |= (0xff000000);
		}
		pattern >>= 1;
	}
    // 残りの左4ドットの処理
	for(i =0; i < 4;i++) {
		pLeft >>= 8;
		if(pattern & 1) {
			pLeft |= (0xff000000);
		}
		pattern >>= 1;
	}
	// パターンジェネレータテーブルへの書き込み
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void writeSpriteAttribute(ms_vdp_t* vdp, int offset, unsigned int attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y座標
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				updateSpriteVisibility(vdp);
			}
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = (((attribute + 1 ) & 0xff) * 2) + (i%2)*16 + 16; // MSXのY座標の2倍, MSXは1ライン下に表示されるので+1, X68000のスプライトの原点は(16,16)なのでずらす
			}
			break;
		case 1: // X座標
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+0] = ((attribute & 0xff) * 2) + (i/2)*16 + 16; // MSXのX座標の2倍
			}
			// TODO ECビットによる位置補正処理
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color; // MSXの4ドット分(X68000だと2倍の8ドットに拡大)
			if (vdp->sprite_size == 0) { // 8x8
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for( i = 0; i < 32*4; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*32+i] & colorex;
				}
			}
			break;
		default:
			break;
	}
	if (vdp->sprite_size == 0) {
		// 8x8モード
		X68_SSR[plNum*4+2] = 0x100 + plNum; // パレット0x10-0x1fを使用するので 0x100を足す
	} else {
		// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
		for( i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // パレット0x10-0x1fを使用するので 0x100を足す
		}
	}

}

/*
 スプライトの表示/非表示を更新する
 関連する仕様は以下の2つ
  * R#8のbit1が1のときはスプライトを非表示にする
  * あるスプライトプレーンのY座標を208にすると、そのスプライトプレーン以降は全て非表示になる
 */
void updateSpriteVisibility(ms_vdp_t* vdp) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;

	int	visible_sprite_planes; // 画面に表示するスプライトプレーンの数。0にすると非表示になる
	if (vdp->r08 & 0x01) {
		visible_sprite_planes = 0;
	} else {
		visible_sprite_planes = 32;
		for (i = 0; i < 32; i++) {
			if (p[i*4+0] == 208) {
				visible_sprite_planes = i;
				break;
			}
		}
	}
	if ((vdp->last_visible_sprite_planes == visible_sprite_planes) && //
		(vdp->last_visible_sprite_size == vdp->sprite_size)) {
		return;
	}

	vdp->last_visible_sprite_planes = visible_sprite_planes;
	vdp->last_visible_sprite_size = vdp->sprite_size;
	for (i = 0; i < visible_sprite_planes; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8モード
			X68_SSR[i*4+3] = 3; // スプライト表示
		} else {
			// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 3; // スプライト表示
			}
		}
	}
	for (; i < 32; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8モード
			X68_SSR[i*4+3] = 0; // スプライト非表示
		} else {
			// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 0; // スプライト非表示
			}
		}
	}
}
