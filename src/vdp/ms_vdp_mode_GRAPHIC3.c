#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_GRAPHIC3(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC3(ms_vdp_t* vdp);
void write_vram_GRAPHIC3(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC3(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC3(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC3(ms_vdp_t* vdp);
void update_resolution_GRAPHIC3(ms_vdp_t* vdp);
void vsync_draw_GRAPHIC2(ms_vdp_t* vdp);
void _refresh_GRAPHIC2(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC3 = {
	// int init_GRAPHIC3(ms_vdp_t* vdp);
	init_GRAPHIC3,
	// uint8_t read_vram_GRAPHIC3(ms_vdp_t* vdp);
	read_vram_GRAPHIC3,
	// void write_vram_GRAPHIC3(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC3,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC3,
	// void update_pnametbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC3,
	// void update_colortbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC3,
	// void update_pgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC3,
	// void update_sprattrtbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC3,
	// void update_sprpgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC3,
	// void update_r7_color_GRAPHIC3(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC3,
	// char* get_mode_name_GRAPHIC3(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC3,
	// void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_NONE,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_NONE,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC3,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_GRAPHIC2,
	// sprite mode
	1
};

/* スプライトモード以外、GRAPHIC2と同じ */

int init_GRAPHIC3(ms_vdp_t* vdp) {
	set_GRAPHIC2_mac();
	update_palette_GRAPHIC2(vdp);
	_refresh_GRAPHIC2(vdp);}

uint8_t read_vram_GRAPHIC3(ms_vdp_t* vdp) {
	return read_vram_TEXT1(vdp);
}

void write_vram_GRAPHIC3(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC2_mac(data);
}

void update_palette_GRAPHIC3(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

static uint32_t last_pnametbl_baddr = 0xffffffff;

void update_pnametbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
	printf("update_pnametbl_baddr_GRAPHIC3\n");
    update_pnametbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_pnametbl_baddr, vdp->pnametbl_baddr);
	if (last_pnametbl_baddr != vdp->pnametbl_baddr) {
		last_pnametbl_baddr = vdp->pnametbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

static uint32_t last_colortbl_baddr = 0xffffffff;

void update_colortbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
	printf("update_colortbl_baddr_GRAPHIC3\n");
	update_colortbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_colortbl_baddr, vdp->colortbl_baddr);
	if (last_colortbl_baddr != vdp->colortbl_baddr) {
		last_colortbl_baddr = vdp->colortbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

static uint32_t last_pgentbl_baddr = 0xffffffff;

void update_pgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
	printf("update_pgentbl_baddr_GRAPHIC3\n");
	update_pgentbl_baddr_DEFAULT(vdp);
	printf("  %06x -> %06x\n", last_pgentbl_baddr, vdp->pgentbl_baddr);
	if (last_pgentbl_baddr != vdp->pgentbl_baddr) {
		last_pgentbl_baddr = vdp->pgentbl_baddr;
		_refresh_GRAPHIC2(vdp);
	}
}

// TODO: スプライトモード1と 2の棲み分け

void update_sprattrtbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC3(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC3(ms_vdp_t* vdp) {
	return "GRAPHIC3";
}

void update_resolution_GRAPHIC3(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}
