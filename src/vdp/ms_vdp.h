#ifndef MS_VDP_H
#define MS_VDP_H

#include <stdint.h>

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
#define CRTR_12		(*(volatile uint16_t *)0xe80018)	// CRTCレジスタ12
#define CRTR_13		(*(volatile uint16_t *)0xe8001a)	// CRTCレジスタ13
#define CRTR_14		(*(volatile uint16_t *)0xe8001c)	// CRTCレジスタ14
#define CRTR_15		(*(volatile uint16_t *)0xe8001e)	// CRTCレジスタ15

#define CRTR_20		(*(volatile uint16_t *)0xe80028)	// CRTCレジスタ20
#define CRTR_21		(*(volatile uint16_t *)0xe8002a)	// CRTCレジスタ21
#define CRTR_23		(*(volatile uint16_t *)0xe8002e)	// CRTCレジスタ23

#define VCRR_00		(*(volatile uint16_t *)0xe82400)	// ビデオコントロールレジスタ0
#define VCRR_01		(*(volatile uint16_t *)0xe82500)	// ビデオコントロールレジスタ1
#define VCRR_02		(*(volatile uint16_t *)0xe82600)	// ビデオコントロールレジスタ2


/*
Control_registers:
R_00:	.dc.b	0		* Mode register 0

VDP_R_01:
R_01:	.dc.b	0		* Mode register 1

R_02:	.dc.b	0
	.dc.b	0
	.dc.b	0
	.dc.b	0
R_06:	.dc.b	0		* Sprite
	.dc.b	0
R_08:	.dc.b	0		* Mode register 2
	.dc.b	0		* Mode register 3
R_10:	.dc.b	0		* >> R#3
R_11:	.dc.b	0		* >> R#5
R_12:	.dc.b	0		* TEXT color / Back color register (ブリンクで使用）
R_13:	.dc.b	0
R_14:	.dc.b	0		* >> vram_add
R_15:	.dc.b	0
R_16:	.dc.b	0		* Color Palette Address pointer
R_17:	.dc.b	0		* Control Register pointer
R_18:	.dc.b	0
R_19:	.dc.b	0
R_20:	.dc.b	0
R_21:	.dc.b	0
R_22:	.dc.b	0
R_23:	.dc.b	0
R_24:	.dc.b	0
R_25:	.dc.b	0
R_26:	.dc.b	0
R_27:	.dc.b	0
	.dc.b	0		* dummy
	.dc.b	0		* dummy
	.dc.b	0		* dummy
	.dc.b	0		* dummy
SX:	.dc.w	0		* 32,33 並びが逆になるので注意
SY:	.dc.w	0		* 34,35
DX:	.dc.w	0		* 36,37
DY:	.dc.w	0		* 38,39
NX:	.dc.w	0		* 40,41
NY:	.dc.w	0		* 42,43
CLR:	.dc.b	0
ARG:	.dc.b	0
R_46:	.dc.b	0

*	ステータスレジスタ
VDP_S_00:
S_0:	.dc.b	0
S_1:	.dc.b	%00000100		* V9958 のＩＤ
S_2:	.dc.b	%10001100		* TR は常に１
S_3:	.dc.b	0
S_4:	.dc.b	%11111110
S_5:	.dc.b	0
S_6:	.dc.b	%11111100
S_7:	.dc.b	0
S_8:	.dc.b	0
S_9:	.dc.b	%11111100

	.even

*	パレットレジスタ
*		%GGGG_GRRR_RRBB_B001
*P_0:	.dc.w	%0000_0000_0000_0000
*P_1:	.dc.w	%0000_0000_0000_0000
*P_2:	.dc.w	%0000_0000_0000_0000
*P_3:	.dc.w	%0000_0000_0000_0000
*P_4:	.dc.w	%0000_0000_0000_0000
*P_5:	.dc.w	%0000_0000_0000_0000
*P_6:	.dc.w	%0000_0000_0000_0000
*P_7:	.dc.w	%0000_0000_0000_0000
*P_8:	.dc.w	%0000_0000_0000_0000
*P_9:	.dc.w	%0000_0000_0000_0000
*P_10:	.dc.w	%0000_0000_0000_0000
*P_11:	.dc.w	%0000_0000_0000_0000
*P_12:	.dc.w	%0000_0000_0000_0000
*P_13:	.dc.w	%0000_0000_0000_0000
*P_14:	.dc.w	%0000_0000_0000_0000
*P_15:	.dc.w	%0000_0000_0000_0000

P_0:	.dc.w	%0000_0000_0000_0000
P_1:	.dc.w	%0000_0000_0000_0000
P_2:	.dc.w	%1101_1001_0000_1001
P_3:	.dc.w	%1111_1011_0101_1011
P_4:	.dc.w	%0010_0001_0011_1111
P_5:	.dc.w	%0110_1010_0111_1111
P_6:	.dc.w	%0010_0101_1000_1001
P_7:	.dc.w	%1101_1010_0111_1111
P_8:	.dc.w	%0010_0111_1100_1001
P_9:	.dc.w	%0110_1111_1101_1011
P_10:	.dc.w	%1101_1110_1100_1001
P_11:	.dc.w	%1101_1110_1110_0101
P_12:	.dc.w	%1001_0001_0000_1001
P_13:	.dc.w	%0100_1110_1110_1101
P_14:	.dc.w	%1011_0101_1010_1101
P_15:	.dc.w	%1111_1111_1111_1111

*	.dc.w	%00000			* 0
*	.dc.w	%00100
*	.dc.w	%01001
*	.dc.w	%01101			* 3
*	.dc.w	%10010			* 4
*	.dc.w	%10110
*	.dc.w	%11011
*	.dc.w	%11111			* 7

*	特殊コントロールレジスタ
* R_00:	.dc.b	0		* [ 0 ][ DG][IE2][IE1][ M5][ M4][ M3][ 0 ]
* R_01:	.dc.b	0		* [ 0 ][ BL][IE0][ M1][ M2][ 0 ][ SI][MAG]	

CRT_mode:			* R_0,R_1の[ M5]～[ M0]
	.dc.b	0		* M5～M0 がそれぞれビット５～０に入っている
_sprite_size:			* R_1の[ SI]
	.dc.b	0		* スプライトのサイズ ０・・８×８  １・・１６×１６
sprite_zoom:			* R_1の[MAG]
	.dc.b	0		* スプライトの拡大   ０・・なし    １・・あり

	.even

R_2:	.dc.l	0		* Pattern name table base address
R_3:	.dc.l	0		* Color table base address
R_4:	.dc.l	$0800		* Pattern generator table base address
R_5:	.dc.l	0		* Sprite attribute table bse address
R_6:	.dc.l	0		* Sprite pattern generator table base address
*R_7:				  Text color / Back drop color register
text_color:			* 文字の色
	.dc.w	0
back_color:			* 背景の色
	.dc.w	0
*/

typedef struct ms_vdp {
	uint8_t	r00;	// Mode register 0
	uint8_t	r01;	// Mode register 1
	uint8_t	_r02;	// R02の生の値。実際のアクセスでは pnametbl_baddr を使う
	uint8_t _r03;	// R03の生の値。実際のアクセスでは R10 と結合した colortbl_baddr を使う
	uint8_t _r04;	// R04の生の値。実際のアクセスでは pgentbl_baddr を使う
	uint8_t _r05;	// R05の生の値。実際のアクセスでは R11 と結合した sprattrtbl_baddr を使う
	uint8_t _r06;	// R06の生の値。実際のアクセスでは sprpgentbl_baddr を使う
	uint8_t _r07;	// R09の生の値。実際のアクセスでは text_color / back_color を使う
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

	// Special Control Registers
	// 	特殊コントロールレジスタ
	//  R_00:	.dc.b	0		* [ 0 ][ DG][IE2][IE1][ M5][ M4][ M3][ 0 ]
	//	R_01:	.dc.b	0		* [ 0 ][ BL][IE0][ M1][ M2][ 0 ][ SI][MAG]
	uint8_t crt_mode;		// R_0,R_1の[ M5]～[ M0]
	uint8_t sprite_size;	// R_1の[ SI]
	uint8_t sprite_zoom;	// R_1の[MAG]
	uint8_t dummy2;

	// base address registers
	uint32_t pnametbl_baddr;		// R02: Pattern name table base address
	uint32_t colortbl_baddr;		// R03: Color table base address
	uint32_t pgentbl_baddr;		// R04: Pattern generator table base address
	uint32_t sprattrtbl_baddr;		// R05: Sprite attribute table bse address
	uint32_t sprpgentbl_baddr;	// R06: Sprite pattern generator table base address
	uint16_t text_color;			// R07: Text color
	uint16_t back_color;			// R07: Back color

	//
	uint32_t vram_addr;
	uint32_t gram_addr;
	//
	uint16_t display_mode;
	uint16_t tx_active;
	uint16_t gr_active;

	//
	uint8_t* vram;	// X68000側のVRAMの先頭アドレス
} ms_vdp_t;


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
	void (*update_text_color)(ms_vdp_t* vdp);
	void (*update_back_color)(ms_vdp_t* vdp);
} ms_vdp_mode_t;

int ms_vdp_init( void *);
void ms_vdp_deinit(void);

void initSprite(ms_vdp_t* vdp);
void writeSpritePattern(ms_vdp_t* vdp, int offset, unsigned int pattern);
void writeSpriteAttribute(ms_vdp_t* vdp, int offset, unsigned int attribute);
void updateSpriteVisibility(ms_vdp_t* vdp);

#endif
