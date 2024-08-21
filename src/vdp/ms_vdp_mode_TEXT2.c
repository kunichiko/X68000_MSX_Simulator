#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_TEXT2(ms_vdp_t* vdp);
uint8_t read_vram_TEXT2(ms_vdp_t* vdp);
void write_vram_TEXT2(ms_vdp_t* vdp, uint8_t data);
void update_palette_TEXT2(ms_vdp_t* vdp);
void update_pnametbl_baddr_TEXT2(ms_vdp_t* vdp);
void update_colortbl_baddr_TEXT2(ms_vdp_t* vdp);
void update_pgentbl_baddr_TEXT2(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_TEXT2(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_TEXT2(ms_vdp_t* vdp);
void update_r7_color_TEXT2(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_TEXT2(ms_vdp_t* vdp);
void update_resolution_TEXT2(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_TEXT2 = {
	// int init_TEXT2(ms_vdp_t* vdp);
	init_TEXT2,
	// uint8_t read_vram_TEXT2(ms_vdp_t* vdp);
	read_vram_TEXT2,
	// void write_vram_TEXT2(ms_vdp_t* vdp, uint8_t data);
	write_vram_TEXT2,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_TEXT2,
	// void update_pnametbl_baddr_TEXT2(ms_vdp_t* vdp);
	update_pnametbl_baddr_TEXT2,
	// void update_colortbl_baddr_TEXT2(ms_vdp_t* vdp);
	update_colortbl_baddr_TEXT2,
	// void update_pgentbl_baddr_TEXT2(ms_vdp_t* vdp);
	update_pgentbl_baddr_TEXT2,
	// void update_sprattrtbl_baddr_TEXT2(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_TEXT2,
	// void update_sprpgentbl_baddr_TEXT2(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_TEXT2,
	// void update_r7_color_TEXT2(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_TEXT2,
	// char* get_mode_name_TEXT2(ms_vdp_t* vdp);
	get_mode_name_TEXT2,
	// void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_NONE,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_NONE,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_TEXT2,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	0
};


int init_TEXT2(ms_vdp_t* vdp) {
	// TEXT2は、横6ドットのキャラクターが80文字なので、幅が480ドット
	// そのため、左右に16ドットずつ(X68000的にも16ドットずつ)の非表示領域があるので
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

	set_TEXT2_mac();
}

uint8_t read_vram_TEXT2(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
}

void write_vram_TEXT2(ms_vdp_t* vdp, uint8_t data) {
	write_vram_DEFAULT(vdp, data);
	//
	uint32_t area = vdp->vram_addr & 0x1f800; // 下位11ビットをクリア
	if (area == vdp->pgentbl_baddr) {
		w_p_gene_tx2_mac();
	} else {
		area &= 0x1f000; // 下位12ビットをクリア
		if (area == vdp->pnametbl_baddr) {
			w_p_name_tx2_mac(data);
		}
	}
	uint32_t addr_h = (vdp->vram_addr + 0) & 0xc000;
	uint32_t addr_l = (vdp->vram_addr + 1) & 0x3fff;
	vdp->vram_addr = (addr_h | addr_l);
}

void update_palette_TEXT2(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_TEXT2(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
	vdp->pnametbl_baddr &= 0x1f000;
}

void update_colortbl_baddr_TEXT2(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_TEXT2(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_TEXT2(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_TEXT2(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_TEXT2(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_TEXT1(vdp, data);
}

char* get_mode_name_TEXT2(ms_vdp_t* vdp) {
	return "TEXT2";
}

void update_resolution_TEXT2(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 1, 0, 0); // 512, 16色, スプライト不使用, BG不使用
}
