#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
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
void update_resolution_GRAPHIC4(ms_vdp_t* vdp);
void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_GRAPHIC4(ms_vdp_t* vdp);
void vdp_command_write_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);

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
	// void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_GRAPHIC4,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_GRAPHIC4,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_GRAPHIC4,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC4,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	2,
	// crt_width
	256,
	// dots_per_byte
	2,
	// bits_per_dot
	4
};


int init_GRAPHIC4(ms_vdp_t* vdp) {
	set_GRAPHIC4_mac();
	update_palette_GRAPHIC4(vdp);
	ms_vdp_update_sprite_area(vdp);
}

uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
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
	switch(vdp->pnametbl_baddr >> 15) {
	case 0:
		vdp->gr_active = 0b0001;
		vdp->gr_active_interlace = 0b0001;
		break;
	case 1:
		vdp->gr_active = 0b0010;
		vdp->gr_active_interlace = 0b0010;		// GRAPHIC4のインターレースモードはまだ未対応
		break;
	case 2:
		vdp->gr_active = 0b0100;
		vdp->gr_active_interlace = 0b0100;
		break;
	case 3:
		vdp->gr_active = 0b1000;
		vdp->gr_active_interlace = 0b1000;		// GRAPHIC4のインターレースモードはまだ未対応
		break;
	default:
		vdp->gr_active = 0b0001;
		vdp->gr_active_interlace = 0b0001;		// GRAPHIC4のインターレースモードはまだ未対応
		break;
	}
	ms_vdp_update_visibility(vdp);
}

void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_sprattrtbl_baddr_MODE2(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_sprpgentbl_baddr_MODE2(vdp);
}

void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp) {
	return "GRAPHIC4";
}

void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd) {
	//MS_LOG(MS_LOG_DEBUG,"GRAPHIC4: vdp_command_exec: %02x\n", cmd);
	vdp_command_exec(vdp, cmd);
}

uint8_t vdp_command_read_GRAPHIC4(ms_vdp_t* vdp) {
	vdp_command_read(vdp);
}

void vdp_command_write_GRAPHIC4(ms_vdp_t* vdp, uint8_t value) {
	vdp_command_write(vdp, value);
}

void update_resolution_GRAPHIC4(ms_vdp_t* vdp) {
	ms_vdp_update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}