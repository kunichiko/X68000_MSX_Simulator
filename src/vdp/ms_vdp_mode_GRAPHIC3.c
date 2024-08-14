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
	get_mode_name_GRAPHIC3
};


int init_GRAPHIC3(ms_vdp_t* vdp) {
	set_GRAPHIC3_mac();
}

uint8_t read_vram_GRAPHIC3(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
}

void write_vram_GRAPHIC3(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC3_mac(data);
}

void update_palette_GRAPHIC3(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC3(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC3(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_GRAPHIC3(ms_vdp_t* vdp) {
	return "GRAPHIC3";
}
