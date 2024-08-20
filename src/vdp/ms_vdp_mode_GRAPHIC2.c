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
void vsync_draw_GRAPHIC2(ms_vdp_t* vdp);

void write_vram_GRAPHIC2_c(ms_vdp_t* vdp, uint8_t data);

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
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_GRAPHIC2,
	// sprite mode
	1
};

void write_pname_tbl_GRAPHIC2(ms_vdp_t* vdp, uint32_t addr, uint8_t data);
void _refresh_GRAPHIC2(ms_vdp_t* vdp);

int init_GRAPHIC2(ms_vdp_t* vdp) {
	set_GRAPHIC2_mac();
	update_palette_GRAPHIC2(vdp);
	_refresh_GRAPHIC2(vdp);
}

uint8_t read_vram_GRAPHIC2(ms_vdp_t* vdp) {
	return read_vram_TEXT1(vdp);
}

void write_vram_GRAPHIC2(ms_vdp_t* vdp, uint8_t data) {
	//write_vram_GRAPHIC2_c(vdp, data);
	w_GRAPHIC2_mac(data);
}

/*
	アセンブラのルーチンを使っているので使ってないが一応残している
 */
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
			write_pname_tbl_GRAPHIC2(vdp, addr, data);
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

void update_palette_GRAPHIC2(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

static uint32_t last_pnametbl_baddr = 0xffffffff;

void update_pnametbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
	printf("update_pnametbl_baddr_GRAPHIC2\n");
    update_pnametbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_pnametbl_baddr, vdp->pnametbl_baddr);
	if (last_pnametbl_baddr != vdp->pnametbl_baddr) {
		last_pnametbl_baddr = vdp->pnametbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

static uint32_t last_colortbl_baddr = 0xffffffff;

void update_colortbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
	printf("update_colortbl_baddr_GRAPHIC2\n");
	update_colortbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_colortbl_baddr, vdp->colortbl_baddr);
	if (last_colortbl_baddr != vdp->colortbl_baddr) {
		last_colortbl_baddr = vdp->colortbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

static uint32_t last_pgentbl_baddr = 0xffffffff;

void update_pgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
	printf("update_pgentbl_baddr_GRAPHIC2\n");
	update_pgentbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_pgentbl_baddr, vdp->pgentbl_baddr);
	if (last_pgentbl_baddr != vdp->pgentbl_baddr) {
		last_pgentbl_baddr = vdp->pgentbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

void update_sprattrtbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC2(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC2(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC2(ms_vdp_t* vdp) {
	return "GRAPHIC2";
}

void update_resolution_GRAPHIC2(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}

void write_pname_tbl_GRAPHIC2(ms_vdp_t* vdp, uint32_t addr, uint8_t data) {
	uint32_t block = (addr >> 8) & 0x3;
	uint32_t posx = addr & 0x1f;
	uint32_t posy = (addr >> 5) & 0x1f;
	uint16_t data16 = data;
	uint32_t color_addr = (vdp->colortbl_baddr & 0x1e000) + (block*256*8) + data16*8;
	color_addr &= 0b11110000000111111 | (vdp->colortbl_baddr & 0b00001111111000000);
	uint32_t pgene_addr = (vdp->pgentbl_baddr & 0x1e000) + (block*256*8) + data16*8;
	pgene_addr &= 0b11110011111111111 | (vdp->pgentbl_baddr  & 0b00001100000000000);

	uint16_t* gram = X68_GRAM + posy*8*512 + posx*8;
	int x,line;
	for(line=0;line<8;line++) {
		uint8_t pattern = vdp->vram[pgene_addr + line];
		uint16_t color = vdp->vram[color_addr + line];
		uint16_t f_color = color >> 4;
		uint16_t b_color = color & 0x0f;;
		for(x=0;x<8;x++) {
			if(pattern & 0x80) {
				*gram++ = f_color;
			} else {
				*gram++ = b_color;
			}
			pattern <<= 1;
		}
		gram += 512-8;
	}
}

static uint32_t rewrite_flag_buffer[8];
static uint32_t refresh_addr = 0;
static int num_refresh = 0;

void vsync_draw_GRAPHIC2(ms_vdp_t* vdp) {
	int i;
	if (refresh_addr == 0) {
		// ループの先頭
		for(i=0;i<8;i++) {
			rewrite_flag_buffer[i] = ms_vdp_rewrite_flag_tbl[i];
			ms_vdp_rewrite_flag_tbl[i] = 0;
		}
		refresh_addr = vdp->pnametbl_baddr & 0x1fc00;
		num_refresh = 0;
	}
	// 1回のvsyncで書き換える数の上限
	int refresh_count = 16;				// だいたい60フレーム = 1秒で 32文字x24行=768文字が書き換えられる計算
	// 1回のvsyncでチェックする数の上限
	int check_count = 128;
	while(refresh_count > 0 && check_count > 0) {
		uint8_t data = vdp->vram[refresh_addr];
		uint8_t group = (data & 0xe0) >> 5;
		uint32_t mask = 1 << (((int)data) & 0x1f);
		if (rewrite_flag_buffer[group] & mask) {
			refresh_count--;
			num_refresh++;
			write_pname_tbl_GRAPHIC2(vdp, refresh_addr, data);
		}
		check_count--;
		refresh_addr++;
		if ((refresh_addr & 0x3ff) == 0) {
			// 一周したので初期化
			refresh_addr = 0;
			return;
		}
	}
}

void _refresh_GRAPHIC2(ms_vdp_t* vdp){
	if(1) {
		// この場ですぐには書き換えずに、フラグだけセットして、vsync_drawで書き換える
		int i;
		for(i=0;i<8;i++) {
			ms_vdp_rewrite_flag_tbl[i] = 0xffffffff;
		}
		refresh_addr = 0;
		return;
	} else {
		// 現在のVRAMの状態を元に画面を再描画	
		int x,y;
		for(y=0;y<32;y++) {
			for(x=0;x<32;x++) {
				uint32_t addr = (vdp->pnametbl_baddr&0x1fc00)+y*32+x;
				uint8_t data = vdp->vram[addr];
				write_pname_tbl_GRAPHIC2(vdp, addr, data);
			}
		}
	}
}

