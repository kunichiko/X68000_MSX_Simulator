#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_GRAPHIC4(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp);
void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC4(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp);
void exec_vdp_command_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);

ms_vdp_mode_t ms_vdp_GRAPHIC4 = {
	// int init_GRAPHIC4(ms_vdp_t* vdp);
	init_GRAPHIC4,
	// uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp);
	read_vram_GRAPHIC4,
	// void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC4,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC4,
	// void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC4,
	// void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC4,
	// void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC4,
	// void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC4,
	// void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC4,
	// void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC4,
	// char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC4,
	// void exec_vdp_command_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_GRAPHIC4
};


int init_GRAPHIC4(ms_vdp_t* vdp) {
	set_GRAPHIC4_mac();
}

uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp) {
	return r_GRAPHIC4_mac();
}

void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC4_mac(data);
}

void update_palette_GRAPHIC4(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_pnametbl_baddr_DEFAULT(vdp);
	vdp->pnametbl_baddr &= 0x18000;
	vdp->gr_active = 1 << (vdp->pnametbl_baddr >> 15);
	update_VCRR_02();
}

void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp) {
	return "GRAPHIC4";
}

void exec_vdp_command_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd) {
	exec_vdp_command_DEFAULT(vdp, cmd);
}