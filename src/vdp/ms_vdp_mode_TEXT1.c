#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_TEXT1(ms_vdp_t* vdp);
uint8_t read_vram_TEXT1(ms_vdp_t* vdp);
void write_vram_TEXT1(ms_vdp_t* vdp, uint8_t data);
void update_palette_TEXT1(ms_vdp_t* vdp);
void update_pnametbl_baddr_TEXT1(ms_vdp_t* vdp);
void update_colortbl_baddr_TEXT1(ms_vdp_t* vdp);
void update_pgentbl_baddr_TEXT1(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_TEXT1(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_TEXT1(ms_vdp_t* vdp);
void update_r7_color_TEXT1(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_TEXT1(ms_vdp_t* vdp);
void update_resolution_TEXT1(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_TEXT1 = {
	// int init_TEXT1(ms_vdp_t* vdp);
	init_TEXT1,
	// uint8_t read_vram_TEXT1(ms_vdp_t* vdp);
	read_vram_TEXT1,
	// void write_vram_TEXT1(ms_vdp_t* vdp, uint8_t data);
	write_vram_TEXT1,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_TEXT1,
	// void update_pnametbl_baddr_TEXT1(ms_vdp_t* vdp);
	update_pnametbl_baddr_TEXT1,
	// void update_colortbl_baddr_TEXT1(ms_vdp_t* vdp);
	update_colortbl_baddr_TEXT1,
	// void update_pgentbl_baddr_TEXT1(ms_vdp_t* vdp);
	update_pgentbl_baddr_TEXT1,
	// void update_sprattrtbl_baddr_TEXT1(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_TEXT1,
	// void update_sprpgentbl_baddr_TEXT1(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_TEXT1,
	// void update_r7_color_TEXT1(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_TEXT1,
	// char* get_mode_name_TEXT1(ms_vdp_t* vdp);
	get_mode_name_TEXT1,
	// void exec_vdp_command_NONE(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_TEXT1
};


int init_TEXT1(ms_vdp_t* vdp) {
	// TEXT1は、横6ドットのキャラクターが40文字なので、幅が240ドット
	// そのため、左右に8ドットずつ(X68000的には16ドットずつ)の非表示領域があるので
	// そこをクリアする
	int i,j;
	for(i=0;i<512;i++) {
		uint32_t *addr = (uint32_t*)(0xc00000 + i*1024);
		for(j=0;j<8;j++) { // 32ビット(2ドット分)ずつクリアするので8回でOK
			*addr = 0;
			*(addr+256-8) = 0;
			addr++;
		}
	}

	//	VDP レベルでは、GRAPHIC7にしても他のモードのパレットは保存されるが、
	//	６８で２５６色モードにすると破壊されてしまうので、再度設定し直す必要がある
	vdp->gr_active = 1;
	update_VCRR_02();

	set_TEXT1_mac();
}

uint8_t read_vram_TEXT1(ms_vdp_t* vdp) {
	// 	movea.l	VideoRAM,a0
	// move.l	vram_add,d1
	// move.b	0(a0,d1.l),d0
	// move.w	d1,d2			* アドレスカウンタのインクリメント
	// addq.w	#1,d2			* TMS9918互換モードなので、A16～A14は
	// andi.w	#%00111111_11111111,d2	* インクリメントされない
	// andi.w	#%11000000_00000000,d1	* ワードでＯＫ	
	// or.w	d2,d1
	// move.l	d1,vram_add
	// rts
	uint8_t ret = vdp->vram[vdp->vram_addr];
	uint32_t addr_h = (vdp->vram_addr + 0) & 0xc000;
	uint32_t addr_l = (vdp->vram_addr + 1) & 0x3fff;
	vdp->vram_addr = (addr_h | addr_l);
	return ret;
}

void write_vram_TEXT1(ms_vdp_t* vdp, uint8_t data) {
	write_vram_DEFAULT(vdp, data);
	//
	uint32_t area = vdp->vram_addr & 0x1fc00; // 下位10ビットをクリア
	if (area == vdp->pnametbl_baddr) {
		w_p_name_tx1_mac(data);
	} else if (area == vdp->pgentbl_baddr) {
		w_p_gene_tx1_mac();
	} 
	uint32_t addr_h = (vdp->vram_addr + 0) & 0xc000;
	uint32_t addr_l = (vdp->vram_addr + 1) & 0x3fff;
	vdp->vram_addr = (addr_h | addr_l);
}

void update_palette_TEXT1(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_TEXT1(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_TEXT1(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_TEXT1(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_TEXT1(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_TEXT1(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_TEXT1(ms_vdp_t* vdp, uint8_t data) {
	vdp->text_color = data >> 4;
	vdp->back_color = data & 0x0f;

	// バックドロップとして設定されているBG面の色を変更する
	// TODO BGを使わないようにする
	uint16_t palette = vdp->palette[vdp->text_color];
	uint16_t *p = (uint16_t*)(0xe82220 + 2);
	*p = palette;
}

char* get_mode_name_TEXT1(ms_vdp_t* vdp) {
	return "TEXT1";
}

void update_resolution_TEXT1(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 1, 0); // 512, 16色
}

//
void write_pattern_name_TEXT1(ms_vdp_t* vdp, uint32_t addr, uint8_t data) {
	uint32_t offset = addr & 0x1fc00;
	uint32_t index = offset >> 5;
	uint32_t x = (addr >> 3) & 0x3;
	uint32_t y = addr & 0x7;
	uint32_t *p = (uint32_t*)&vdp->vram[offset];
	uint32_t mask = 0x80000000 >> x;
	if (data) {
		p[y] |= mask;
	} else {
		p[y] &= ~mask;
	}
}