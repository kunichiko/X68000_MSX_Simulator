#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_SCREEN10(ms_vdp_t* vdp);
uint8_t read_vram_SCREEN10(ms_vdp_t* vdp);
void write_vram_SCREEN10(ms_vdp_t* vdp, uint8_t data);
void update_palette_SCREEN10(ms_vdp_t* vdp);
void update_pnametbl_baddr_SCREEN10(ms_vdp_t* vdp);
void update_colortbl_baddr_SCREEN10(ms_vdp_t* vdp);
void update_pgentbl_baddr_SCREEN10(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_SCREEN10(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_SCREEN10(ms_vdp_t* vdp);
void update_r7_color_SCREEN10(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_SCREEN10(ms_vdp_t* vdp);
void exec_vdp_command_SCREEN10(ms_vdp_t* vdp, uint8_t cmd);

ms_vdp_mode_t ms_vdp_SCREEN10 = {
	// int init_SCREEN10(ms_vdp_t* vdp);
	init_SCREEN10,
	// uint8_t read_vram_SCREEN10(ms_vdp_t* vdp);
	read_vram_SCREEN10,
	// void write_vram_SCREEN10(ms_vdp_t* vdp, uint8_t data);
	write_vram_SCREEN10,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_SCREEN10,
	// void update_pnametbl_baddr_SCREEN10(ms_vdp_t* vdp);
	update_pnametbl_baddr_SCREEN10,
	// void update_colortbl_baddr_SCREEN10(ms_vdp_t* vdp);
	update_colortbl_baddr_SCREEN10,
	// void update_pgentbl_baddr_SCREEN10(ms_vdp_t* vdp);
	update_pgentbl_baddr_SCREEN10,
	// void update_sprattrtbl_baddr_SCREEN10(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_SCREEN10,
	// void update_sprpgentbl_baddr_SCREEN10(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_SCREEN10,
	// void update_r7_color_SCREEN10(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_SCREEN10,
	// char* get_mode_name_SCREEN10(ms_vdp_t* vdp);
	get_mode_name_SCREEN10,
	// void exec_vdp_command_SCREEN10(ms_vdp_t* vdp, uint8_t cmd);
	exec_vdp_command_SCREEN10
};


int init_SCREEN10(ms_vdp_t* vdp) {

}

uint8_t read_vram_SCREEN10(ms_vdp_t* vdp) {

}

void write_vram_SCREEN10(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_SCREEN10(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_SCREEN10(ms_vdp_t* vdp) {
    update_pnametbl_baddr_DEFAULT(vdp);
}

void update_colortbl_baddr_SCREEN10(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_SCREEN10(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_SCREEN10(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_SCREEN10(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_SCREEN10(ms_vdp_t* vdp, uint8_t data) {
}

char* get_mode_name_SCREEN10(ms_vdp_t* vdp) {
	return "SCREEN10";
}

void exec_vdp_command_SCREEN10(ms_vdp_t* vdp, uint8_t cmd) {
	exec_vdp_command_DEFAULT(vdp, cmd);
}