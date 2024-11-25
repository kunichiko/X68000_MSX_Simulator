#ifndef MS_VDP_H
#define MS_VDP_H

#include <stdint.h>
#include "../ms.h"

// extern uint16_t * const X68_GR_PAL;
// extern uint16_t * const X68_TX_PAL;
// extern uint16_t * const X68_SP_PAL_B0;
// extern uint16_t * const X68_SP_PAL_B1;
// extern uint16_t * const X68_SP_PAL_B2;

// uint16_t * const X68_GR_PAL = (uint16_t *)0xE82000;
// uint16_t * const X68_TX_PAL = (uint16_t *)0xE82200;
// uint16_t * const X68_SP_PAL_B0 = (uint16_t *)0xE82200;	// ブロック0はテキストと共用
// uint16_t * const X68_SP_PAL_B1 = (uint16_t *)0xE82220;	// ブロック1はスプライトパレットに使用
// uint16_t * const X68_SP_PAL_B2 = (uint16_t *)0xE82240;	// ブロック2以降は使用していない

#define X68_GRAM	((uint16_t *)0xc00000)
#define X68_GRAM_LEN	0x100000	// ワード数
#define X68_GR_PAL	((uint16_t *)0xE82000)
#define X68_TX_PAL	((uint16_t *)0xE82200)
#define X68_SP_PAL_B0	((uint16_t *)0xE82200)	// ブロック0はテキストと共用
#define X68_SP_PAL_B1	((uint16_t *)0xE82220)	// ブロック1はスプライトパレットに使用
#define X68_SP_PAL_B2	((uint16_t *)0xE82240)	// ブロック2以降は使用していない

#define CRTR		((volatile uint16_t *)0xe80000)		// CRTCレジスタ
#define CRTR_00		(*(volatile uint16_t *)0xe80000)	// CRTCレジスタ0
#define CRTR_01		(*(volatile uint16_t *)0xe80002)	// CRTCレジスタ1
#define CRTR_02		(*(volatile uint16_t *)0xe80004)	// CRTCレジスタ2
#define CRTR_03		(*(volatile uint16_t *)0xe80006)	// CRTCレジスタ3
#define CRTR_04		(*(volatile uint16_t *)0xe80008)	// CRTCレジスタ4
#define CRTR_05		(*(volatile uint16_t *)0xe8000a)	// CRTCレジスタ5
#define CRTR_06		(*(volatile uint16_t *)0xe8000c)	// CRTCレジスタ6
#define CRTR_07		(*(volatile uint16_t *)0xe8000e)	// CRTCレジスタ7
#define CRTR_08		(*(volatile uint16_t *)0xe80010)	// CRTCレジスタ8
#define CRTR_09		(*(volatile uint16_t *)0xe80012)	// CRTCレジスタ9
#define CRTR_10		(*(volatile uint16_t *)0xe80014)	// CRTCレジスタ10 (テキストスクロールX)
#define CRTR_11		(*(volatile uint16_t *)0xe80016)	// CRTCレジスタ11 (テキストスクロールY)
#define CRTR_SCR_p	((volatile uint16_t *)0xe80018)		// CRTCスクロールレジスタへのポインタ
#define CRTR_12		(*(volatile uint16_t *)0xe80018)	// CRTCレジスタ12 (GR0のスクロールX）
#define CRTR_13		(*(volatile uint16_t *)0xe8001a)	// CRTCレジスタ13 (GR0のスクロールY)
#define CRTR_14		(*(volatile uint16_t *)0xe8001c)	// CRTCレジスタ14 (GR1のスクロールX）
#define CRTR_15		(*(volatile uint16_t *)0xe8001e)	// CRTCレジスタ15 (GR1のスクロールY)

#define CRTR_20		(*(volatile uint16_t *)0xe80028)	// CRTCレジスタ20
#define CRTR_21		(*(volatile uint16_t *)0xe8002a)	// CRTCレジスタ21
#define CRTR_23		(*(volatile uint16_t *)0xe8002e)	// CRTCレジスタ23

#define VCRR_00		(*(volatile uint16_t *)0xe82400)	// ビデオコントロールレジスタ0
#define VCRR_01		(*(volatile uint16_t *)0xe82500)	// ビデオコントロールレジスタ1
#define VCRR_02		(*(volatile uint16_t *)0xe82600)	// ビデオコントロールレジスタ2

#define SPCON_BGCON	(*(volatile uint16_t *)0xeb0808)	// BGコントロールレジスタ
#define SPCON_HTOTAL (*(volatile uint16_t *)0xeb080a)	// 水平トータルレジスタ
#define SPCON_HDISP	(*(volatile uint16_t *)0xeb080c)	// 水平解像度設定レジスタ
#define SPCON_VSISP	(*(volatile uint16_t *)0xeb080e)	// 垂直解像度設定レジスタ
#define SPCON_RES	(*(volatile uint16_t *)0xeb0810)	// スプライト解像度設定レジスタ

#define X68_SSR		((volatile uint16_t *)0xeb0000)	// スプライトスクロールレジスタ
#define X68_PCG		((volatile uint32_t *)0xeb8000)	// スプライトパターン

#define CRT_MODE_GRAPHIC1	0x00
#define CRT_MODE_TEXT1		0x01
#define CRT_MODE_MULTICOLOR	0x02
#define CRT_MODE_GRAPHIC2	0x04
#define CRT_MODE_GRAPHIC3	0x08
#define CRT_MODE_TEXT2		0x09
#define CRT_MODE_GRAPHIC4	0x0c
#define CRT_MODE_GRAPHIC5	0x10
#define CRT_MODE_GRAPHIC6	0x14
#define CRT_MODE_GRAPHIC7	0x1c

extern uint32_t* ms_vdp_rewrite_flag_tbl;

typedef struct ms_vdp_mode ms_vdp_mode_t;

/**
 * @brief VDPクラスの定義
 * 
 */
typedef struct ms_vdp {
	uint8_t	r00;	// Mode register 0
	uint8_t	r01;	// Mode register 1
	uint8_t	_r02;	// R02の生の値。実際のアクセスでは pnametbl_baddr を使う
	uint8_t _r03;	// R03の生の値。実際のアクセスでは R10 と結合した colortbl_baddr を使う
	uint8_t _r04;	// R04の生の値。実際のアクセスでは pgentbl_baddr を使う
	uint8_t _r05;	// R05の生の値。実際のアクセスでは R11 と結合した sprattrtbl_baddr を使う
	uint8_t _r06;	// R06の生の値。実際のアクセスでは sprpgentbl_baddr を使う
	uint8_t _r07;	// R07の生の値。実際のアクセスでは text_color / back_color を使う
	uint8_t r08;	// Mode register 2
	uint8_t r09;	// Mode register 3
	uint8_t _r10;	// R10の生の値。実際のアクセスでは R03 と結合した colortbl_baddr を使う
	uint8_t _r11;	// R11の生の値。実際のアクセスでは R05 と結合した sprattrtbl_baddr を使う
	uint8_t r12;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t r16;
	uint8_t r17;
	uint8_t r18;
	uint8_t r19;
	uint8_t r20;
	uint8_t r21;
	uint8_t r22;
	uint8_t r23;
	uint8_t r24;
	uint8_t r25;
	uint8_t r26;
	uint8_t r27;
	uint8_t r28;
	uint8_t r29;
	uint8_t r30;
	uint8_t r31;
	uint16_t sx;	// 32,33 (並びが逆になるので注意)
	uint16_t sy;	// 34,35
	uint16_t dx;	// 36,37
	uint16_t dy;	// 38,39
	uint16_t nx;	// 40,41
	uint16_t ny;	// 42,43
	uint8_t clr;	// 44
	uint8_t arg;	// 45
	uint8_t r46;

	uint8_t dummy1;	// 47

	// Status Registers
	uint8_t s00;	// offset = +48
	uint8_t s01;
	uint8_t s02;
	uint8_t s03;
	uint8_t s04;
	uint8_t s05;
	uint8_t s06;
	uint8_t s07;
	uint8_t s08;
	uint8_t s09;	// offset = +57

	// Palette Registers
	uint16_t palette[16];	// offset = +58
	uint8_t alt_color_zero; // カラーコード0の色に最も近い色を持つパレット番号。スプライトがカラーコード0で描画できる問題への対処
	uint8_t dummy2;

	// Special Control Registers
	// 	特殊コントロールレジスタ
	//  R_00:	.dc.b	0		* [ 0 ][ DG][IE2][IE1][ M5][ M4][ M3][ 0 ]
	//	R_01:	.dc.b	0		* [ 0 ][ BL][IE0][ M1][ M2][ 0 ][ SI][MAG]
	uint8_t crt_mode;		// R_0,R_1の[ M5]～[ M0]
	uint8_t sprite_size;	// R_1の[ SI]
	uint8_t sprite_zoom;	// R_1の[MAG]
	uint8_t dummy3;

	// base address registers
	uint32_t pnametbl_baddr;		// R02: Pattern name table base address
	uint32_t colortbl_baddr;		// R03: Color table base address
	uint32_t pgentbl_baddr;		// R04: Pattern generator table base address
	uint32_t sprattrtbl_baddr;		// R05: Sprite attribute table bse address
	uint32_t sprcolrtbl_baddr;		// Sprite attibute table base address - 512
	uint32_t sprpgentbl_baddr;	// R06: Sprite pattern generator table base address
	uint16_t text_color;			// R07: Text color
	uint16_t back_color;			// R07: Back color

	//
	uint32_t vram_addr;
	uint32_t gram_addr;

	//
	uint8_t disablehsyncint;
	uint8_t dummy4;

	//
	ms_vdp_mode_t *ms_vdp_current_mode;

	//
	uint16_t tx_active;
	uint16_t gr_active;
	uint16_t gr_active_interlace;

	// X68000側に確保したVRAMの先頭アドレス
	uint8_t* vram;

	// X68000のPCGに転送するためのバッファ領域
	uint32_t* x68_pcg_buffer;
	int last_visible_sprite_planes;
	int last_visible_sprite_size;

	// スプライトリフレッシュ要求フラグ
	// どのビットが1になっているかで、再描画する範囲を決定します。
	// ビット番号が下のものほど軽い処理で、上になる程思い処理としています。
	// 現状で定義しているのは以下の通りです。
	// bit 0: スプライトの座標や、表示非表示のみの変更
	// bit 1: スプライトアトリビュートテーブル/カラーテーブルの変更 (パターン番号や色の変更のみで、パターン自体に変更はない)
	// bit 2: CCビットビットマップフラグの変更
	// bit 3: パターンジェネレータテーブルの変更
	//
	// bit0をセットすると、スプライト非表示ビットの変更や、スプライトのY=208による非表示、R#23の縦スクロールによる
	// 位置変化などを更新します。
	// 個別スプライトの X座標、Y座標の変更は即座に反映させているので、これをセットしなくても、
	// スプライトの座標は変化します。
	// （もしかしたら、将来、XY座標も遅延リフレッシュさせるようにするかも？）
	//
	// bit1をセットすると、全スプライトプレーンに対応するPCGを更新します。
	// 実PCGは事前に計算したPCGバッファを使っているため、VRAM上のパターンが変更されている場合は、
	// bit3の方もセットしないと追従しません。
	// bit1はたとえば、VDPで直接スプライトアトリビュートテーブルを書き潰された場合や、スプライトアトリビュート
	// テーブルベースアドレスレジスタに変更が入った場合に使用します。このビットが立っていた場合は、
	// キャッシュされているPCGバッファを元に X68000側のスプライトパターンを全て再描画しますが、
	// MSX側で色が変更されただけでも、X68000側はカラーコードを書き直さないといけないため、
	// この処理が必要です
	//
	// bit2をセットすると、スプライトモード2のCCビットを全検査して、ビットマップフラグ再作成します。
	// スプライトカラーテーブルベースアドレスが変更された時にセットする必要があります。
	//
	// bit3は、VDPで直接スプライトパターンジェネレータテーブルを書き潰された場合や、スプライトパターン
	// ジェネレータテーブルベースアドレスレジスタに変更が入った場合に使用します。
	// このビットが立っていた場合は、MSXのパターンジェネレータテーブルの値を元にPCGバッファを再構築した上で、
	// X68000側のスプライトパターンを全て再描画しなおします。
	uint16_t sprite_refresh_flag;

	// VDPコマンドのワークエリア
	// これ以降は ms_vdp_mac.has に定義されているオフセットに影響しないので増減してもOK
	uint8_t cmd_current;
	uint8_t cmd_logiop;
	uint8_t cmd_arg;
	uint32_t cmd_vram_addr;
	uint16_t cmd_vram_addr_mod;		// 論理転送時、vram_addrの1バイト内の位置 GRAPHIC4,6の場合は 0,1、GRAPHIC5の場合は 0,1,2,3
	uint16_t cmd_nx_count;
	uint16_t cmd_ny_count;
	uint16_t cmd_ny_sprite_start;	// VDPでスプライト領域を書き換えられたかの検出用
	uint16_t cmd_ny_sprite_end;		// VDPでスプライト領域を書き換えられたかの検出用


} ms_vdp_t;

#define SPRITE_REFRESH_FLAG_COORD	0x01
#define SPRITE_REFRESH_FLAG_ATTR	0x02
#define SPRITE_REFRESH_FLAG_CC		0x04
#define SPRITE_REFRESH_FLAG_PGEN	0x08

/*
 MSXの画面モードごとに切り替える処理群
 
 * init
 	* X68000側のCRTCパラメータ設定などを行う
 * VDPレジスタの書き換えに対する処理
 * VRAM読み込み
 * VRAM書き込み
	* VDPレジスタの値とアドレスを比較し、以下を分岐
		* 画面書き換え系
		* スプライトの書き換え
 * VDPコマンドの実行

 */
typedef struct ms_vdp_mode {
	int (*init)(ms_vdp_t* vdp);
	uint8_t (*read_vram)(ms_vdp_t* vdp);
	void (*write_vram)(ms_vdp_t* vdp, uint8_t data);
	void (*update_palette)(ms_vdp_t* vdp);
	void (*update_pnametbl_baddr)(ms_vdp_t* vdp);
	void (*update_colortbl_baddr)(ms_vdp_t* vdp);
	void (*update_pgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprattrtbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprpgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_r7_color)(ms_vdp_t* vdp, uint8_t data);
	char* (*get_mode_name)(ms_vdp_t* vdp);
	void (*vdp_command_exec)(ms_vdp_t* vdp, uint8_t cmd);
	uint8_t (*vdp_command_read)(ms_vdp_t* vdp);
	void (*vdp_command_write)(ms_vdp_t* vdp, uint8_t data);
	void (*update_resolution)(ms_vdp_t* vdp);
	void (*vsync_draw)(ms_vdp_t* vdp);
	int sprite_mode; // 0x00: 未使用, 0x01: MODE1, 0x02: MODE2, bit7: 0=256ドット, 1=512ドット
	int crt_width; // MSXの画面幅
	int dots_per_byte; // 1バイトあたりのドット数 (VDPコマンド用)
	int bits_per_dot; // 1ドットあたりのビット数 (VDPコマンド用)
} ms_vdp_mode_t;

// singleton instance
ms_vdp_t* ms_vdp_shared_instance();
void ms_vdp_shared_deinit();

void ms_vdp_set_mode(ms_vdp_t* vdp, int mode);
void ms_vdp_vsync_draw(ms_vdp_t* vdp, int hostdebugmode);

void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern, int32_t old_pattern);
void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color, int32_t old_color);

void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_MODE2(ms_vdp_t* vdp);

void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_MODE2(ms_vdp_t* vdp);

void vsync_draw_NONE(ms_vdp_t* vdp);

void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd);
void vdp_command_exec(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_NONE(ms_vdp_t* vdp);
uint8_t vdp_command_read(ms_vdp_t* vdp);
void vdp_command_write_NONE(ms_vdp_t* vdp, uint8_t data);
void vdp_command_write(ms_vdp_t* vdp, uint8_t data);

/**
 * 
 * @param vdp 
 * @param res 0=256ドット, 1=512ドット
 * @param color 0=16色, 1=256色, 3=65536色
 * @param bg 0=非表示, 1=表示
 */
void ms_vdp_update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg);
void ms_vdp_update_visibility(ms_vdp_t* vdp);
void ms_vdp_update_sprite_area(ms_vdp_t* vdp);
void ms_vdp_sprite_vsync_draw(ms_vdp_t* vdp, int hostdebugmode);

#endif
