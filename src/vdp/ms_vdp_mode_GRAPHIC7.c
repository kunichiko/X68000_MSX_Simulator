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
void update_pname_tbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
void update_text_color_GRAPHIC7(ms_vdp_t* vdp);
void update_back_color_GRAPHIC7(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC7 = {
	// int init_GRAPHIC7(ms_vdp_t* vdp);
	init_GRAPHIC7,
	// uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp);
	read_vram_GRAPHIC7,
	// void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC7,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC7,
	// void update_pname_tbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
	update_pname_tbl_baddr_GRAPHIC7,
	// void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
	update_colortbl_baddr_GRAPHIC7,
	// void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
	update_pgentbl_baddr_GRAPHIC7,
	// void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
	update_sprattrtbl_baddr_GRAPHIC7,
	// void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr);
	update_sprpgentbl_baddr_GRAPHIC7,
	// void update_text_color_GRAPHIC7(ms_vdp_t* vdp);
	update_text_color_GRAPHIC7,
	// void update_back_color_GRAPHIC7(ms_vdp_t* vdp);
	update_back_color_GRAPHIC7
};


int init_GRAPHIC7(ms_vdp_t* vdp) {
	set_GRAPHIC7_mac();
}

uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp) {

}

void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_GRAPHIC7(ms_vdp_t* vdp) {
	//パレットはない
}

void update_pname_tbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr) {
    update_pname_tbl_baddr_GRAPHIC6(vdp, addr);
}

void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr) {
    update_colortbl_baddr_DEFAULT(vdp, addr);
}

void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr) {
    update_pgentbl_baddr_DEFAULT(vdp, addr);
}

void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr) {
    update_sprattrtbl_baddr_DEFAULT(vdp, addr);
}

void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp, uint32_t addr) {
    update_sprpgentbl_baddr_DEFAULT(vdp, addr);
}

void update_text_color_GRAPHIC7(ms_vdp_t* vdp) {

}

void update_back_color_GRAPHIC7(ms_vdp_t* vdp) {

}
