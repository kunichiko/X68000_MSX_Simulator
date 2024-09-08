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
void init_palette(ms_vdp_t* vdp);

// Singleton instance
static ms_vdp_t* _shared = NULL;

ms_vdp_t* ms_vdp_shared_instance() {
	if( _shared != NULL) {
		return _shared;
	}
	if ( (_shared = (ms_vdp_t*)new_malloc(sizeof(ms_vdp_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"メモリが確保できません\n");
		return NULL;
	}
	if ( (_shared->vram = (uint8_t*)new_malloc(0x20000)) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"メモリが確保できません\n");
		ms_vdp_shared_deinit();
		return NULL;
	}
	// X68000は 1スプライト(16x16)パターンあたり128バイト(uint32_tが32ワード)が必要
	// MSXは 256個定義できるが、X68000は128個しか定義できないため、メモリ上に定義領域を作っておき
	// 表示時に転送するようにしている
	// PCGバッファの必要最大量は、
	//  * MSX 8x8ドットのスプライト256定義
	//  * X68000の512ドットモードで拡大スプライトを使うと1ドットが4ドットになり、16x16ドットのスプライトを4つ並べて表示する
	//  * => 256 * 4 * 32 * 4バイト = 128KB
	if ( (_shared->x68_pcg_buffer = (uint32_t*)new_malloc( 256 * 4 * 32 * sizeof(uint32_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"メモリが確保できません\n");
		ms_vdp_shared_deinit();
		return NULL;
	}

	// b7: F,  b6: 5S, b5: Collision, b4-b0: 衝突番号
	_shared->s00 = 0b00011111;
	// b7: FL, b6: LPS, b5-1: V9958のID, b0: FH
	_shared->s01 = 0b00000100;
	// b7: TR, b6: VR, b5: HR, b4: BD, b3: 1, b2: 1, b1: EO, b0: CE
	_shared->s02 = 0b10001100; // TRは常に1
	_shared->s04 = 0b11111110; // 上位ビットは1固定
	_shared->s06 = 0b11111100; // 上位ビットは1固定
	_shared->s09 = 0b11111100; // 上位ビットは1固定

	// 初期画面モードを 512x512にする
	// 実際には、MSXの画面モードに応じてこの後色々変化する
	_iocs_crtmod(4);	// 512x512, 31kHz, 16色 4枚
	_iocs_g_clr_on();	// グラフィックス画面初期化
	_iocs_sp_init();

	ms_vdp_init_mac(_shared);
	// 初期状態はTEXT1
	ms_vdp_set_mode(_shared, 0);

	init_sprite(_shared);

	ms_vdp_update_resolution_COMMON(_shared, 1, 0, 0); // 512, 16色, BG不使用

	// GRAMクリア
	int i;
	for(i=0;i<X68_GRAM_LEN;i++) {
		X68_GRAM[i] = 0;
	}
	// VRAMクリア
	for(i=0;i<0x20000;i++) {
		_shared->vram[i] = 0;
	}

	// パレット初期化
	init_palette(_shared);

	return _shared;
}

void ms_vdp_shared_deinit() {
	if (_shared == NULL) {
		return;
	}
	ms_vdp_deinit_mac(_shared);
	new_free(_shared->x68_pcg_buffer);
	new_free(_shared->vram);
	// シングルトンの場合は deinitで freeする
	new_free(_shared);
	_shared = NULL;
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

void init_palette(ms_vdp_t* vdp) {
	// X68000のパレットフォーマット GGGGGRRR_RRBBBBB1に合わせる
	int i;
	for (i = 0; i < 16; i++)
	{
		uint16_t color = 1;
		color |= ((default_palette[i][0] >> 3) & 0x1f) << 6;  // R
		color |= ((default_palette[i][1] >> 3) & 0x1f) << 11; // G
		color |= ((default_palette[i][2] >> 3) & 0x1f) << 1;  // B
		vdp->palette[i] = color;
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
		MS_LOG(MS_LOG_INFO,"Unknown VDP mode: %d\n", mode);
		vdp->ms_vdp_current_mode = &ms_vdp_DEFAULT;
	}
	vdp->ms_vdp_current_mode->update_resolution(vdp);
	// GRAMクリア
	uint32_t words = 0;
	switch(vdp->ms_vdp_current_mode->bits_per_dot) {
	case 0:	// TEST, GRAPHIC1など
	case 2:
	case 4:
		// 4色, 16色
		words = X68_GRAM_LEN;
		break;
	case 8:
		// 256色
		words = X68_GRAM_LEN / 2;
		break;
	case 16:
		// 65536色
		words = X68_GRAM_LEN / 4;
		break;
	}
	int i;
	for(i=0;i<words;i++) {
		X68_GRAM[i] = 0;
	}
	vdp->ms_vdp_current_mode->init(vdp);
	MS_LOG(MS_LOG_INFO,"VDP Mode: %s\n", vdp->ms_vdp_current_mode->get_mode_name(vdp));

	// スプライトの初期化処理
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
}


uint16_t crtc_values[4][13] = {
	// 256x192, 60Hz
	{	45, 4,  6, 38, 524, 5, 40, 424, 25, // CRTCレジスタ0-8
		0xff,   6+4,           40, // スプライトコントローラ画面モードレジスタ
		8*(32+8)	// テキスト画面のオフセット値
	},
	// 256x212, 60Hz
	{	45, 4,  6, 38, 524, 5, 52, 476, 27, // CRTCレジスタ0-8
		0xff,   6+4,           52, // スプライトコントローラ画面モードレジスタ
		8*(32+5)+4	// テキスト画面のオフセット値
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
void ms_vdp_update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg) {
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
	CRTR_20 = ((color&0x3) << 8) | 0x10 | ((res&0x1) << 2) | (res&0x1);
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

	// 画面モードが変わったら、表示状態も更新
	ms_vdp_update_visibility(vdp);
}

/**
 * @brief 画面の表示/非表示、ページ切り替え、スクロール量、インターレースなどの表示に関わる設定を更新します
 * 
 * 画面モードが変わった時に、X68000の画面解像度を変更する処理は、ms_vdp_update_resolution_COMMON() で
 * 行っているので、ここでは、画面の表示/非表示、ページ切り替え、スクロール量、インターレースなどの表示に
 * 関わる設定を更新します。
 * 
 * 具体的には、MSXの以下のパラメータがわかった時にこの処理を呼び出してください。
 * 
 * * 画面の表示/非表示
 * 		* VDP Mode register 1 (R#1) の bit6 (BL) が 0 の時は画面を表示しない
 * * ページ切り替え
 * 		* GRAPHIC4-7の pattern name table base address によるページ切り替え
 * * スクロール量
 * 		* VDP R#23の値によるスクロール量の変更
 * * インターレース
 * 		* VDP R#9の bit2 (交互表示) が1 かつ、bit3 (インターレース)が1
 * 		* さらに現在のページが奇数ページの時、インターレースモードになる
 * 		* 実機では偶数ページと奇数ページを交互に切り替えるが、MS.Xは 512x424 の画面として表示します
 * 
 * それぞれの設定が相互に影響し合うため、この関数で一括して設定しています。
 * 
 * @param vdp 
 */
void ms_vdp_update_visibility(ms_vdp_t* vdp) {
	int is_blank = (vdp->r01 & 0x40) ? 0 : 1;
	int is_interlace = (vdp->r09 & 0x0c) == 0x0c ? 1 : 0; 

	// ビデオコントロールレジスタ(VCR R#02)の設定
	// 	b7	常に0
	// 	b6	スプライト+BG画面の表示
	// 	b5	テキスト画面の表示
	// 	b4	1024x1024時のグラフィック画面の表示(512x512なので使わない)
	// 	b3	512x512時のグラフィック画面 GR3 の表示
	// 	b2	512x512時のグラフィック画面 GR2 の表示
	// 	b1	512x512時のグラフィック画面 GR1 の表示
	// 	b0	512x512時のグラフィック画面 GR0 の表示
	uint16_t r02 = VCRR_02 & 0b1111111100100000;
	r02 |= 0b01000000;		// スプライト+BG面は常に表示(スプライトの表示/非表示は個別のスプライトのON/OFFで行っている)
	if( !is_blank) {
		r02 |= is_interlace ? vdp->gr_active_interlace : vdp->gr_active;
	}
	r02 |= vdp->tx_active << 5;
	VCRR_02 = r02;

	// スクロール量の設定
	uint16_t r23 = vdp->r23;
	if(vdp->ms_vdp_current_mode->crt_width == 256) {
		// 256ドットモードの時
		uint16_t scrY = r23;
		CRTR_SCR_p[0*2+1] = scrY;		// GR0のYスクロール
		CRTR_SCR_p[1*2+1] = scrY;		// GR1のYスクロール
		CRTR_SCR_p[2*2+1] = scrY;		// GR2のYスクロール
		CRTR_SCR_p[3*2+1] = scrY;		// GR3のYスクロール
	} else {
		// 512ドットモードの時
		uint16_t scrYe = (r23 * 2 - 0) & 0x1ff;
		uint16_t scrYo = is_interlace ? //
			(r23 * 2 - 1) & 0x1ff : // インターレース時は奇数ページの時は1ドット下に表示
			(r23 * 2 - 0) & 0x1ff;
		CRTR_SCR_p[0*2+1] = scrYe;		// GR0のYスクロール
		CRTR_SCR_p[1*2+1] = scrYo;		// GR1のYスクロール
		//CRTR_SCR_p[2*2+1] = scrYe;		// GR2のYスクロール (3ページ目は存在しないが)
		//CRTR_SCR_p[3*2+1] = scrYo;		// GR3のYスクロール (4ページ目は存在しないが)
	}
}

uint8_t last_vdp_R1 = 0;
uint8_t last_vdp_R8 = 0;
uint8_t last_vdp_R23 = 0;

/*
*/
void ms_vdp_vsync_draw(ms_vdp_t* vdp) {
	// 画面モードごとの再描画処理を呼び出す
	vdp->ms_vdp_current_mode->vsync_draw(vdp);

	// スプライトの再描画処理

	if ( (vdp->r01 & 0x01) != (last_vdp_R1 & 0x01) ) {
		// スプライトの拡大/標準サイズが変化
		// TODO: まだスプライトの拡大は対応していない
	}
	if ( (vdp->r01 & 0x02) != (last_vdp_R1 & 0x02) ) {
		// スプライトサイズ(8x8 or 16x16)が変化
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
	}
	if ( (vdp->r08 & 0x02) != (last_vdp_R8 & 0x02) ) {
		// スプライト表示 ON/OFFフラグが変化
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
	}
	if ( vdp->r23 != last_vdp_R23 ) {
		// スクロール量が変化
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
	}
	ms_vdp_sprite_vsync_draw(vdp);
	last_vdp_R1 = vdp->r01;
	last_vdp_R8 = vdp->r08;
	last_vdp_R23 = vdp->r23;
}