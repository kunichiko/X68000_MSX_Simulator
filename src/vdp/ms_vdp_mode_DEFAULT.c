#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_DEFAULT(ms_vdp_t* vdp);
uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
void update_palette_DEFAULT(ms_vdp_t* vdp);
void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_DEFAULT(ms_vdp_t* vdp);
void update_resolution_DEFAULT(ms_vdp_t* vdp);
void vsync_draw_DEFAULT(ms_vdp_t* vdp);


ms_vdp_mode_t ms_vdp_DEFAULT = {
	// int init_DEFAULT(ms_vdp_t* vdp);
	init_DEFAULT,
	// uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
	read_vram_DEFAULT,
	// void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
	write_vram_DEFAULT,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_DEFAULT,
	// void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_pnametbl_baddr_DEFAULT,
	// void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_colortbl_baddr_DEFAULT,
	// void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_pgentbl_baddr_DEFAULT,
	// void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_MODE1,
	// void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_MODE1,
	// void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_DEFAULT,
	// char* get_mode_name_DEFAULT(ms_vdp_t* vdp);
	get_mode_name_DEFAULT,
	// void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_NONE,
	// uint8_t vdp_command_read_DEFAULT(ms_vdp_t* vdp);
	vdp_command_read_NONE,
	// void vdp_command_write_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_DEFAULT,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	0
};


int init_DEFAULT(ms_vdp_t* vdp) {
}

uint8_t read_vram_DEFAULT(ms_vdp_t* vdp) {
	uint8_t ret = vdp->vram[vdp->vram_addr];
	vdp->vram_addr = (vdp->vram_addr + 1) & 0x1ffff;
	return ret;
}

void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data) {
	vdp->vram[vdp->vram_addr] = data;
	vdp->vram_addr = (vdp->vram_addr + 1) & 0x1ffff;
}

/*
*
*	パレット０はバックドロップに使用された時のみ有効。－＞ＢＧのパレット１にセットする。
*	→TODO バックドロップにBGを使うのをやめたので考え直す
*

	lea.l	P_0,a0
	lea.l	$E82220+2,a1			* ＢＧ用のパレット

	move.w	back_color,d0
	add.w	d0,d0
	move.w	0(a0,d0.w),(a1)			* a0 は P_0 のアドレスが入っている

	lea.l	2(a0),a0
	move.w	#15-1,d0
	lea.l	g_palette+2,a1			* グラフィック用のパレット
	lea.l	t_palette+32+2,a2		* スプライト用のパレット(0x10?0x1fを使用)
@@:	move.w	(a0),(a1)+
	move.w	(a0)+,(a2)+
	dbra	d0,@b

	rts*/
void update_palette_DEFAULT(ms_vdp_t* vdp) {
	int i;

	// 透明色は背景色が透けて見えるようにする
	X68_GR_PAL[0] = vdp->palette[vdp->back_color & 0xf];
	//X68_GR_PAL[0] = 0x0e01;

	for(i =1; i < 16; i++) {
		uint16_t color = vdp->palette[i];
		if (vdp->tx_active) {
			// 輝度を半分に落とす
			color &= 0b1111011110111100;
			color >>= 1;
		}
		X68_GR_PAL[i] = color;
		X68_SP_PAL_B1[i] = color;
	}
}

void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R02 に b17-b10が入っているのでシフトする
	vdp->pnametbl_baddr = (vdp->_r02 << 10) & 0x1ffff;
}

void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R03 に b13-b6
	// R10 に b16-b14
	vdp->colortbl_baddr = ((vdp->_r10 << 14) | (vdp->_r03 << 6)) & 0x1ffff;
}

void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R04 に b16-b11
	vdp->pgentbl_baddr = (vdp->_r04 << 11) & 0x1ffff;
}

void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp) {
	// R05 に b14-b7
	// R11 に b16-b15
	// MODE1では、b7まで使われ、b6-b0は0として扱われる
	uint32_t addr = ((vdp->_r11 << 15) | (vdp->_r05 << 7)) & 0x1ff80;
	if (vdp->sprattrtbl_baddr != addr) {
		// 更新されたら、全てのスプライトのアトリビュートを再生成する
		vdp->sprattrtbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void update_sprattrtbl_baddr_MODE2(ms_vdp_t* vdp) {
	// R05 に b14-b7
	// R11 に b16-b15

	// TODO: MODE2のb9-b7の扱いを改めて調べる必要がある
	// ese-vdpでは以下のようにしていた
	// アトリビュートテーブル: b9は書き込まれた値をそのまま使う、b8-b7は0として扱う
	// カラーテーブル		: b9はアトリビュートテーブルの反転、b8-b7は0として扱う	
	uint32_t addr = ((vdp->_r11 << 15) | (vdp->_r05 << 7)) & 0x1fe00;
	if (vdp->sprattrtbl_baddr != addr) {
		// 更新されたら、全てのスプライトのアトリビュートを再生成する
		vdp->sprattrtbl_baddr = addr;
		vdp->sprcolrtbl_baddr = addr ^ 0x200;
		ms_vdp_update_sprite_area(vdp);
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_CC;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp) {
	// R06 に b16-b11
	uint32_t addr = (vdp->_r06 << 11) & 0x1ffff;
	if( vdp->sprpgentbl_baddr != addr ) {
		// 更新されたら、全てのスプライトのパターンを再生成する
		vdp->sprpgentbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
	}
}

void update_sprpgentbl_baddr_MODE2(ms_vdp_t* vdp) {
	// R06 に b16-b11
	uint32_t addr = (vdp->_r06 << 11) & 0x1ffff;
	if( vdp->sprpgentbl_baddr != addr ) {
		// 更新されたら、全てのスプライトのパターンを再生成する
		vdp->sprpgentbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
		ms_vdp_update_sprite_area(vdp);
	}
}

/**
 * @brief VDPレジスタ R#7 の色設定
 * 
 * @param vdp 
 */
void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data) {
	vdp->text_color = data >> 4;
	vdp->back_color = data & 0x0f;

	X68_GR_PAL[0] = vdp->palette[vdp->back_color & 0xf];
}

char* get_mode_name_DEFAULT(ms_vdp_t* vdp) {
	return "DEFAULT";
}

void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd) {
	MS_LOG(MS_LOG_INFO,"%sのVDPコマンド0x%02xはまだ未実装です。\n", vdp->ms_vdp_current_mode->get_mode_name(vdp), cmd);
	// 念の為CEビットをクリア
	vdp->s02 &= 0xfe;
}

void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd) {
}

uint8_t vdp_command_read_NONE(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write_NONE(ms_vdp_t* vdp, uint8_t cmd) {
}

void update_resolution_DEFAULT(ms_vdp_t* vdp) {
	ms_vdp_update_resolution_COMMON(vdp, 1, 0, 0); // 512, 16色, BG不使用
}

void vsync_draw_NONE(ms_vdp_t* vdp) {
}