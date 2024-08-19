#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_GRAPHIC1(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC1(ms_vdp_t* vdp);
void write_vram_GRAPHIC1(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC1(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC1(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC1(ms_vdp_t* vdp);
void update_resolution_GRAPHIC1(ms_vdp_t* vdp);
void vsync_draw_GRAPHIC1(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC1 = {
	// int init_GRAPHIC1(ms_vdp_t* vdp);
	init_GRAPHIC1,
	// uint8_t read_vram_GRAPHIC1(ms_vdp_t* vdp);
	read_vram_GRAPHIC1,
	// void write_vram_GRAPHIC1(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC1,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC1,
	// void update_pnametbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC1,
	// void update_colortbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC1,
	// void update_pgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC1,
	// void update_sprattrtbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC1,
	// void update_sprpgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC1,
	// void update_r7_color_GRAPHIC1(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC1,
	// char* get_mode_name_GRAPHIC1(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC1,
	// void exec_vdp_command_NONE(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC1,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_GRAPHIC1,
	// sprite mode
	1
};

void write_pname_tbl_GRAPHIC1(ms_vdp_t* vdp, uint32_t addr, uint8_t data);
void _refresh_GRAPHIC1(ms_vdp_t* vdp);

int init_GRAPHIC1(ms_vdp_t* vdp) {
	set_GRAPHIC1_mac();
	update_palette_GRAPHIC1(vdp);
	_refresh_GRAPHIC1(vdp);}

uint8_t read_vram_GRAPHIC1(ms_vdp_t* vdp) {
	return read_vram_TEXT1(vdp);
}

void write_vram_GRAPHIC1(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC1_mac(data);
}

void update_palette_GRAPHIC1(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC1(ms_vdp_t* vdp) {
	update_pnametbl_baddr_DEFAULT(vdp);
	_refresh_GRAPHIC1(vdp);
}

void update_colortbl_baddr_GRAPHIC1(ms_vdp_t* vdp) {
	update_colortbl_baddr_DEFAULT(vdp);
	_refresh_GRAPHIC1(vdp);
}

void update_pgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp) {
	update_pgentbl_baddr_DEFAULT(vdp);
	_refresh_GRAPHIC1(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC1(ms_vdp_t* vdp) {
	update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC1(ms_vdp_t* vdp) {
	update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC1(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC1(ms_vdp_t* vdp) {
	return "GRAPHIC1";
}

void update_resolution_GRAPHIC1(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}


void write_pname_tbl_GRAPHIC1(ms_vdp_t* vdp, uint32_t addr, uint8_t data) {
	uint32_t posx = addr & 0x1f;
	uint32_t posy = (addr >> 5) & 0x1f;
	uint16_t data16 = data;
	uint32_t color_addr = (vdp->colortbl_baddr & 0x1ffc0) + (data16>>3);
	uint32_t pgene_addr = (vdp->pgentbl_baddr  & 0x1f800) + (data16*8);

	uint16_t* gram = X68_GRAM + posy*8*512 + posx*8;
	int x,line;
	uint8_t f_color = vdp->vram[color_addr] >> 4;
	uint8_t b_color = vdp->vram[color_addr] & 0x0f;;
	for(line=0;line<8;line++) {
		uint8_t pattern = vdp->vram[pgene_addr + line];
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

void vsync_draw_GRAPHIC1(ms_vdp_t* vdp) {
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
	int refresh_count = 32;
	// 1回のvsyncでチェックする数の上限
	int check_count = 128;
	while(refresh_count > 0 && check_count > 0) {
		uint8_t data = vdp->vram[refresh_addr];
		uint8_t group = (data & 0xe0) >> 5;
		uint32_t mask = 1 << (((int)data) & 0x1f);
		if (rewrite_flag_buffer[group] & mask) {
			refresh_count--;
			num_refresh++;
			write_pname_tbl_GRAPHIC1(vdp, refresh_addr, data);
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

void _refresh_GRAPHIC1(ms_vdp_t* vdp){
	// この場ですぐには書き換えずに、フラグだけセットして、vsync_drawで書き換える
	int i;
	for(i=0;i<8;i++) {
		ms_vdp_rewrite_flag_tbl[i] = 0xffffffff;
	}
	return;
}
