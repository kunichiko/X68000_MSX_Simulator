#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_MULTICOLOR(ms_vdp_t* vdp);
uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp);
void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
void update_palette_MULTICOLOR(ms_vdp_t* vdp);
void update_pnametbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
void update_r7_color_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_MULTICOLOR(ms_vdp_t* vdp);
void update_resolution_MULTICOLOR(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_MULTICOLOR = {
	// int init_MULTICOLOR(ms_vdp_t* vdp);
	init_MULTICOLOR,
	// uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp);
	read_vram_MULTICOLOR,
	// void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
	write_vram_MULTICOLOR,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_MULTICOLOR,
	// void update_pnametbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
	update_pnametbl_baddr_MULTICOLOR,
	// void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
	update_colortbl_baddr_MULTICOLOR,
	// void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
	update_pgentbl_baddr_MULTICOLOR,
	// void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_MULTICOLOR,
	// void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_MULTICOLOR,
	// void update_r7_color_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_MULTICOLOR,
	// char* get_mode_name_MULTICOLOR(ms_vdp_t* vdp);
	get_mode_name_MULTICOLOR,
	// void exec_vdp_command_NONE(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_MULTICOLOR
};


int init_MULTICOLOR(ms_vdp_t* vdp) {
	set_MULTICOLOR_mac();	
}

uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp) {
	return read_vram_TEXT1(vdp);
}

void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data) {
	write_vram_DEFAULT(vdp, data);
	// まだマルチカラーモードは未実装
}

void update_palette_MULTICOLOR(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_MULTICOLOR(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_MULTICOLOR(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_MULTICOLOR(ms_vdp_t* vdp) {
	return "MULTICOLOR";
}

void update_resolution_MULTICOLOR(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 1, 0); // 512, 16色
}