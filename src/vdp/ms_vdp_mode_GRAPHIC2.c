#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_GRAPHIC2(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC2(ms_vdp_t* vdp);
void write_vram_GRAPHIC2(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC2(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC2(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC2(ms_vdp_t* vdp);
void update_resolution_GRAPHIC2(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC2 = {
	// int init_GRAPHIC2(ms_vdp_t* vdp);
	init_GRAPHIC2,
	// uint8_t read_vram_GRAPHIC2(ms_vdp_t* vdp);
	read_vram_GRAPHIC2,
	// void write_vram_GRAPHIC2(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC2,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC2,
	// void update_pnametbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC2,
	// void update_colortbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC2,
	// void update_pgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC2,
	// void update_sprattrtbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC2,
	// void update_sprpgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC2,
	// void update_r7_color_GRAPHIC2(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC2,
	// char* get_mode_name_GRAPHIC2(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC2,
	// void exec_vdp_command_NONE(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC2,
	// sprite mode
	1
};

void write_pname_tbl(ms_vdp_t* vdp, uint32_t addr, uint8_t data);

int init_GRAPHIC2(ms_vdp_t* vdp) {
	set_GRAPHIC2_mac();

	// 現在のVRAMの状態を元に画面を再描画	
	int x,y;
	for(y=0;y<32;y++) {
		for(x=0;x<32;x++) {
			uint32_t addr = vdp->pnametbl_baddr+y*32+x;
			uint8_t data = vdp->vram[addr];
			write_pname_tbl(vdp, addr, data);
		}
	}
}

uint8_t read_vram_GRAPHIC2(ms_vdp_t* vdp) {
	return read_vram_TEXT1(vdp);
}

void write_vram_GRAPHIC2(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC2_mac(data);
}

void write_vram_GRAPHIC2_c(ms_vdp_t* vdp, uint8_t data) {
	write_vram_DEFAULT(vdp, data);
	//
	uint32_t area = vdp->vram_addr;
	area &= 0x1ff80;	// 下位7ビットをクリア
	if (area == vdp->sprattrtbl_baddr) {
		write_sprite_attribute(vdp, vdp->vram_addr - area, data);
	} else {
		area &= 0x1fc00; // 下位10ビットをクリア
		if (area == vdp->pnametbl_baddr) {
			uint32_t addr = vdp->vram_addr & 0x03ff;
			write_pname_tbl(vdp, addr, data);
		} else {
		 	area &= 0x1f800; // 下位11ビットをクリア
			if (area == vdp->sprpgentbl_baddr) {
				write_sprite_pattern(vdp, vdp->vram_addr - area, data);
			} else {
			 	area &= 0x1e000; // 下位13ビットをクリア
			 	if (area == vdp->colortbl_baddr) {
					//write_color_tbl(vdp, data);
				} else if (area == vdp->pgentbl_baddr) {
					//write_pgen_tbl(vdp, data);
				}
			}
		}
	}
		
	uint32_t addr_h = (vdp->vram_addr + 0) & 0xc000;
	uint32_t addr_l = (vdp->vram_addr + 1) & 0x3fff;
	vdp->vram_addr = (addr_h | addr_l);
}

void write_pname_tbl(ms_vdp_t* vdp, uint32_t addr, uint8_t data) {
	uint32_t posx = addr & 0x1f;
	uint32_t posy = (addr >> 5) & 0x1f;
	uint32_t block = (posy >> 3) & 0x3;
	uint32_t color_addr = (vdp->colortbl_baddr & 0xe000) + block*256*8 + data*8;
	uint32_t pattern_addr = (vdp->pgentbl_baddr & 0xe000) + block*256*8 + data*8;

	uint16_t* gram = X68_GRAM + posy*8*512 + posx*8;
	int x,y;
	for(y=0;y<8;y+=2) {
		uint8_t color = vdp->vram[color_addr + y];
		uint8_t pattern = vdp->vram[pattern_addr + y];
		for(x=0;x<8;x++) {
			if(pattern & 0x80) {
				*gram++ = (color >> 4);
			} else {
				*gram++ = (color & 0x0f);
			}
			pattern <<= 1;
		}
		gram += 512*2-8;
	}
}

void update_palette_GRAPHIC2(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
	update_colortbl_baddr_DEFAULT(vdp);
	vdp->colortbl_baddr &= 0x1e000;
}

void update_pgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
	update_pgentbl_baddr_DEFAULT(vdp);
	vdp->pgentbl_baddr &= 0x1e000;
}

void update_sprattrtbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC2(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_GRAPHIC2(ms_vdp_t* vdp) {
	return "GRAPHIC2";
}

void update_resolution_GRAPHIC2(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}
