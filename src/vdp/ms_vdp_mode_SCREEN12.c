#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_SCREEN12(ms_vdp_t* vdp);
uint8_t read_vram_SCREEN12(ms_vdp_t* vdp);
void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data);
void update_palette_SCREEN12(ms_vdp_t* vdp);
void update_pnametbl_baddr_SCREEN12(ms_vdp_t* vdp);
void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp);
void update_pgentbl_baddr_SCREEN12(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_SCREEN12(ms_vdp_t* vdp);
void update_r7_color_SCREEN12(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_SCREEN12(ms_vdp_t* vdp);
void update_resolution_SCREEN12(ms_vdp_t* vdp);
void vdp_command_exec_SCREEN12(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_SCREEN12(ms_vdp_t* vdp);
void vdp_command_write_SCREEN12(ms_vdp_t* vdp, uint8_t cmd);

ms_vdp_mode_t ms_vdp_SCREEN12 = {
	// int init_SCREEN12(ms_vdp_t* vdp);
	init_SCREEN12,
	// uint8_t read_vram_SCREEN12(ms_vdp_t* vdp);
	read_vram_SCREEN12,
	// void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data);
	write_vram_SCREEN12,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_SCREEN12,
	// void update_pnametbl_baddr_SCREEN12(ms_vdp_t* vdp);
	update_pnametbl_baddr_SCREEN12,
	// void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp);
	update_colortbl_baddr_SCREEN12,
	// void update_pgentbl_baddr_SCREEN12(ms_vdp_t* vdp);
	update_pgentbl_baddr_SCREEN12,
	// void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_SCREEN12,
	// void update_sprpgentbl_baddr_SCREEN12(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_SCREEN12,
	// void update_r7_color_SCREEN12(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_SCREEN12,
	// char* get_mode_name_SCREEN12(ms_vdp_t* vdp);
	get_mode_name_SCREEN12,
	// void vdp_command_exec_SCREEN12(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_SCREEN12,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_SCREEN12,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_SCREEN12,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_SCREEN12,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	2,
	// crt_width
	256,
	// dots_per_byte
	1,
	// bits_per_dot
	8
};


int init_SCREEN12(ms_vdp_t* vdp) {

}

uint8_t read_vram_SCREEN12(ms_vdp_t* vdp) {

}

void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_SCREEN12(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_SCREEN12(ms_vdp_t* vdp) {
	update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp) {
	update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_SCREEN12(ms_vdp_t* vdp) {
	update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp) {
	update_sprattrtbl_baddr_MODE2(vdp);
}

void update_sprpgentbl_baddr_SCREEN12(ms_vdp_t* vdp) {
	update_sprpgentbl_baddr_MODE2(vdp);
}

void update_r7_color_SCREEN12(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_SCREEN12(ms_vdp_t* vdp) {
	return "SCREEN12";
}

void vdp_command_exec_SCREEN12(ms_vdp_t* vdp, uint8_t cmd) {
	vdp_command_exec_DEFAULT(vdp, cmd);
}

uint8_t vdp_command_read_SCREEN12(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write_SCREEN12(ms_vdp_t* vdp, uint8_t cmd) {
}

void update_resolution_SCREEN12(ms_vdp_t* vdp) {
	ms_vdp_update_resolution_COMMON(vdp, 1, 3, 0); // 512, 65536色, BG不使用
}