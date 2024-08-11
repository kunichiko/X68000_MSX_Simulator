#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_MULTICOLOR(ms_vdp_t* vdp);
uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp);
void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
void update_palette_MULTICOLOR(ms_vdp_t* vdp);
void update_pname_tbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
void update_text_color_MULTICOLOR(ms_vdp_t* vdp);
void update_back_color_MULTICOLOR(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_MULTICOLOR = {
	// int init_MULTICOLOR(ms_vdp_t* vdp);
	init_MULTICOLOR,
	// uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp);
	read_vram_MULTICOLOR,
	// void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data);
	write_vram_MULTICOLOR,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_MULTICOLOR,
	// void update_pname_tbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
	update_pname_tbl_baddr_MULTICOLOR,
	// void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
	update_colortbl_baddr_MULTICOLOR,
	// void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
	update_pgentbl_baddr_MULTICOLOR,
	// void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
	update_sprattrtbl_baddr_MULTICOLOR,
	// void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr);
	update_sprpgentbl_baddr_MULTICOLOR,
	// void update_text_color_MULTICOLOR(ms_vdp_t* vdp);
	update_text_color_MULTICOLOR,
	// void update_back_color_MULTICOLOR(ms_vdp_t* vdp);
	update_back_color_MULTICOLOR
};


int init_MULTICOLOR(ms_vdp_t* vdp) {
	set_MULTICOLOR_mac();	
}

uint8_t read_vram_MULTICOLOR(ms_vdp_t* vdp) {

}

void write_vram_MULTICOLOR(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_MULTICOLOR(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pname_tbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr) {
    update_pname_tbl_baddr_DEFAULT(vdp, addr);
}

void update_colortbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr) {
    update_colortbl_baddr_DEFAULT(vdp, addr);
}

void update_pgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr) {
    update_pgentbl_baddr_DEFAULT(vdp, addr);
}

void update_sprattrtbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr) {
    update_sprattrtbl_baddr_DEFAULT(vdp, addr);
}

void update_sprpgentbl_baddr_MULTICOLOR(ms_vdp_t* vdp, uint32_t addr) {
    update_sprpgentbl_baddr_DEFAULT(vdp, addr);
}

void update_text_color_MULTICOLOR(ms_vdp_t* vdp) {

}

void update_back_color_MULTICOLOR(ms_vdp_t* vdp) {

}
