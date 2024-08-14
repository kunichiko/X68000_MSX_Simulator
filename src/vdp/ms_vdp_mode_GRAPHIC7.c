#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

/*
	GRAPHIC7 (SCREEN 8) は 256×212モードで、ドットごとに256色画指定可能
 */

int init_GRAPHIC7(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp);
void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC7(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC7 = {
	// int init_GRAPHIC7(ms_vdp_t* vdp);
	init_GRAPHIC7,
	// uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp);
	read_vram_GRAPHIC7,
	// void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC7,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC7,
	// void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC7,
	// void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC7,
	// void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC7,
	// void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC7,
	// void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC7,
	// void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC7,
	// char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC7
};


int init_GRAPHIC7(ms_vdp_t* vdp) {
	set_GRAPHIC7_mac();
}

uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp) {
	return r_GRAPHIC7_mac();
}

void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC7_mac(data);
}

void update_palette_GRAPHIC7(ms_vdp_t* vdp) {
	//パレットはない
}

void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
    update_pnametbl_baddr_GRAPHIC6(vdp);
}

void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp) {
	return "GRAPHIC7";
}
