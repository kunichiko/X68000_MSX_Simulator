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
void init_sprite(ms_vdp_t* vdp);
void init_palette();

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

	// b7: FL, b6: LPS, b5-1: V9958のID, b0: FH
	ms_vdp_shared->s01 = 0b00000100;
	// b7: TR, b6: VR, b5: HR, b4: BD, b3: 1, b2: 1, b1: EO, b0: CE
	ms_vdp_shared->s02 = 0b10001100; // TRは常に1
	ms_vdp_shared->s04 = 0b11111110; // 上位ビットは1固定
	ms_vdp_shared->s06 = 0b11111100; // 上位ビットは1固定
	ms_vdp_shared->s09 = 0b11111100; // 上位ビットは1固定

	// 初期画面モードを 512x512にする
	// 実際には、MSXの画面モードに応じてこの後色々変化する
	_iocs_crtmod(4);	// 512x512, 31kHz, 16色 4枚
	_iocs_g_clr_on();	// グラフィックス画面初期化
	_iocs_sp_init();

	ms_vdp_init_mac(ms_vdp_shared);
	// 初期状態はTEXT1
	ms_vdp_set_mode(ms_vdp_shared, 0);

	init_sprite(ms_vdp_shared);

	update_resolution_COMMON(ms_vdp_shared, 1, 0, 0); // 512, 16色, BG不使用

	// GRAMクリア
	int i;
	for(i=0;i<X68_GRAM_LEN;i++) {
		X68_GRAM[i] = 0;
	}
	// VRAMクリア
	for(i=0;i<0x20000;i++) {
		ms_vdp_shared->vram[i] = 0;
	}

	// パレット初期化
	init_palette();

	return ms_vdp_shared;
}

void ms_vdp_deinit(ms_vdp_t* vdp) {
	ms_vdp_deinit_mac(ms_vdp_shared);
	new_free(vdp->x68_pcg_buffer);
	new_free(vdp->vram);
}

/*
	TMS9918はパレットがないので、MSX1のROMを使うとパレットが初期化されない
	そのため、ここで初期化する
	TMS9918の色味とV9938のパレットは厳密には違うが、ここではTMS9918の色味に合わせてX68000のパレットを初期化する
	MSX2以降のROMを使うと上書きされてしまうのでOKとする
	カラーコードはこの辺りを参考にした
	https://forums.atariage.com/topic/262599-palette-fixes-for-colem-adamem-and-classic99/
*/
uint16_t default_palette[16][3] = {
	{0,0,0},		// 0 TRANSPARENT
	{0,0,0},		// 1 BLACK
	{79,176,69},	// 2 MEDIUM GREEN
	{129,202,119},	// 3 LIGHT GREEN
	{95,81,237},	// 4 DARK BLUE
	{129,116,255},	// 5 LIGHT BLUE
	{173,101,77},	// 6 DARK RED
	{103,195,228},	// 7 CYAN
	{204,110,80},	// 8 MEDIUM RED
	{240,146,116},	// 9 LIGHT RED
	{193,202,81},	// 10 DARK YELLOW
	{209,215,129},	// 11 LIDHT YELLOW
	{72,156,59},	// 12 DARK GREEN
	{176,104,190},	// 13 MAGENTA
	{204,204,204},	// 14 GRAY
	{255,255,255}	// 15 WHITE
};

void init_palette() {
	// X68000のパレットフォーマット GGGGGRRR_RRBBBBB1に合わせる
	int i;
	for (i = 0; i < 16; i++)
	{
		uint16_t color = 1;
		color |= ((default_palette[i][0] >> 3) & 0x1f) << 6;  // R
		color |= ((default_palette[i][1] >> 3) & 0x1f) << 11; // G
		color |= ((default_palette[i][2] >> 3) & 0x1f) << 1;  // B
		X68_GR_PAL[i] = color;
		X68_SP_PAL_B1[i] = color;
	}
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
 * @param bg 0=非表示, 1=表示
 */
void update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg) {
	// lines 0=192ライン, 1=212ライン (MSX換算)
	int lines = (vdp->r09 & 0x80) >> 7;
	// sprite 0=非表示, 1=表示
 	int sprite = (vdp->ms_vdp_current_mode->sprite_mode > 0) ? 1 : 0;
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

	// スプライトとBGの設定
	SPCON_BGCON =	(((sprite | bg) & 0x1) << 9) | // SP/BG = ON
					(0x0 << 4 ) | // BG1 TXSEL
					(0x0 << 3 ) | // BG1 ON
					(0x1 << 2 ) | // BG0 TXSEL
					((bg & 0x1) << 0 );  // BG0 ON
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
 スプライトの初期化
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCGバッファの初期化
	for ( i = 0; i < 256 * 32; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern);

/*
 スプライトパターンジェネレータテーブルへの書き込み
     offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込むパターン(下位8bitのみ使用)
*/
void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_pattern_512(vdp, offset, pattern);
	} else {
		write_sprite_pattern_256(vdp, offset, pattern);
	}
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine; // MSXの1ラインはX68000でも1ライン
	uint32_t pcg_pattern=0; // x68000の16x16のパターンに変換したもの

    // 右端のドットから処理
	for(i =0; i < 8; i++) {
		pcg_pattern >>= 4;
		if(pattern & 1) {
			pcg_pattern |= (0xf0000000);
		}
		pattern >>= 1;
	}
	// パターンジェネレータテーブルのバッファに書き込む
	vdp->x68_pcg_buffer[ptNum * 8 + pcgLine] = pcg_pattern;
}

void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	unsigned int pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドットを X68000の8x8のパターン2つに変換

    // 右端のドットから処理
	for(i =0; i < 4; i++) {
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
	// パターンジェネレータテーブルのバッファに書き込み
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute);

void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_attribute_512(vdp, offset, attribute);
	} else {
		write_sprite_attribute_256(vdp, offset, attribute);
	}
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y座標
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
			}
			X68_SSR[plNum*4+1] = ((attribute + 1 ) & 0xff) + 16; // MSXのY座標の1倍, MSXは1ライン下に表示されるので+1, X68000のスプライトの原点は(16,16)なのでずらす
			break;
		case 1: // X座標
			X68_SSR[plNum*4+0] = (attribute & 0xff) + 16; // MSXのX座標の1倍, X68000のスプライトの原点は(16,16)なのでずらす
			// TODO ECビットによる位置補正処理
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
			if (vdp->sprite_size == 0) {
				// 8x8
				for( i = 0; i < 8; i++) { 
					X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*8+i] & colorex;
				}
			} else {
				// 16x16
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*8+i] & colorex;
				}
			}
			break;
		default:
			break;
	}

	// パターン指定 & パレット指定
	X68_SSR[plNum*4+2] = 0x100 + plNum; // パレット0x10-0x1fを使用するので 0x100を足す
}

void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y座標
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
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
void update_sprite_visibility(ms_vdp_t* vdp) {
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
