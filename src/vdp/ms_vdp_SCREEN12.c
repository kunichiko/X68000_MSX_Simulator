#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_SCREEN12(ms_vdp_t* vdp);
uint8_t read_vram_SCREEN12(ms_vdp_t* vdp);
void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data);
void update_pname_tbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
void update_patgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
void update_sprpatgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
void update_text_color_SCREEN12(ms_vdp_t* vdp);
void update_back_color_SCREEN12(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_SCREEN12 = {
	// int init_SCREEN12(ms_vdp_t* vdp);
	init_SCREEN12,
	// uint8_t read_vram_SCREEN12(ms_vdp_t* vdp);
	read_vram_SCREEN12,
	// void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data);
	write_vram_SCREEN12,
	// void update_pname_tbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
	update_pname_tbl_baddr_SCREEN12,
	// void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
	update_colortbl_baddr_SCREEN12,
	// void update_patgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
	update_patgentbl_baddr_SCREEN12,
	// void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
	update_sprattrtbl_baddr_SCREEN12,
	// void update_sprpatgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr);
	update_sprpatgentbl_baddr_SCREEN12,
	// void update_text_color_SCREEN12(ms_vdp_t* vdp);
	update_text_color_SCREEN12,
	// void update_back_color_SCREEN12(ms_vdp_t* vdp);
	update_back_color_SCREEN12
};


int init_SCREEN12(ms_vdp_t* vdp) {

}

uint8_t read_vram_SCREEN12(ms_vdp_t* vdp) {

}

void write_vram_SCREEN12(ms_vdp_t* vdp, uint8_t data) {

}

void update_pname_tbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr) {

}

void update_colortbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr) {

}

void update_patgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr) {

}

void update_sprattrtbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr) {

}

void update_sprpatgentbl_baddr_SCREEN12(ms_vdp_t* vdp, uint32_t addr) {

}

void update_text_color_SCREEN12(ms_vdp_t* vdp) {

}

void update_back_color_SCREEN12(ms_vdp_t* vdp) {

}
