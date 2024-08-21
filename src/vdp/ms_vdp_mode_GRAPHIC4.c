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
	2
};


int init_GRAPHIC4(ms_vdp_t* vdp) {
	set_GRAPHIC4_mac();
	update_palette_GRAPHIC4(vdp);
}

uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
}

void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
	vdp->vram[vdp->vram_addr] = data;
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
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp) {
	return "GRAPHIC4";
}

/*
	DX, DYから VRAMアドレスを求める

	addr = DY×128 + dx/2
*/
uint32_t get_vram_add_G4(ms_vdp_t* vdp, uint32_t x, uint32_t y, int* mod) {
	if (mod != NULL) {
		*mod = x & 1;
	}
	return (y&0x3ff)*128 + x/2;
}

uint16_t* to_gram(ms_vdp_t* vdp, uint32_t vaddr, int mod) {
	uint16_t p = (vaddr & 0x18000) >> 15;
	uint16_t y = (vaddr & 0x07f80) >> 7;
	uint16_t x = (vaddr & 0x0007f);		// X座標の1/2なので注意
 	return X68_GRAM + (p*512*512) + (y*512) + (x*2) + mod;
}

void _PSET_G4(ms_vdp_t* vdp, uint8_t cmd) {
	int mod;
	uint32_t vaddr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);

	// VRAMに書き込む
	if (mod) {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0xf0) | (vdp->clr & 0x0f);
	} else {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0x0f) | (vdp->clr << 4);
	}
	uint16_t* gram = to_gram(vdp, vaddr, mod);
	*gram = vdp->clr;
}

void _LMMC_exe_G4(ms_vdp_t* vdp, uint8_t value);

void _LMMC_G4(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	printf("LMMC START G4********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_logiop = logiop;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);
	vdp->cmd_vram_addr_mod = mod;

	vdp->cmd_nx_count = vdp->nx;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CEビットをセット
	_LMMC_exe_G4(vdp, vdp->clr);			// 最初の1ドットを書き込む
}

void _LMMC_exe_G4(ms_vdp_t* vdp, uint8_t value) {
	//printf("LMMC exe G4: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CEビットをクリア
		vdp->cmd_current = 0;
		return;
	}
	uint8_t logiop = vdp->cmd_logiop;
	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t vamod = vdp->cmd_vram_addr_mod;
	uint32_t vaddr_mod = vaddr * 2 + vamod;
	uint16_t* gram = to_gram(vdp, vaddr, vamod);

	uint8_t dst = vdp->vram[vaddr];
	if (vamod == 0) {
		dst >>= 4;
	} else {
		dst &= 0x0f;
	}
	switch(logiop) {
	case 0x0:	// IMP
		dst = value;
		break;
	case 0x1:	// AND
		dst = value & dst;
		break;
	case 0x2:	// OR
		dst = value | dst;
		break;
	case 0x3:	// XOR
		dst = value ^ dst;
		break;
	case 0x4:	// NOT
		dst = !value;
		break;
	case 0x8:	// TIMP
		dst = value == 0 ? dst : value;
		break;
	case 0x9:	// TAND
		dst = value == 0 ? dst : value & dst;
		break;
	case 0xa:	// TOR
		dst = value == 0 ? dst : value | dst;
		break;
	case 0xb:	// TXOR
		dst = value == 0 ? dst : value ^ dst;
		break;
	case 0xc:	// TNOT
		dst = value == 0 ? dst : !value;
		break;
	}

	// VRAMに書き込む
	if (vamod == 0) {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0x0f) + (dst << 4);
	} else {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0xf0) + (dst & 0xf);
	}

	*gram = dst;

	vdp->s02 |= 0x80;				// TRビットをセット
	vdp->cmd_nx_count-=1;
	// DIXに従ってVRAMアドレスを更新
	if ((vdp->cmd_arg & 0x4)==0) {
		vaddr_mod += 1;
	} else {
		vaddr_mod -= 1;
	}
	if(vdp->cmd_nx_count == 0) {
		// 1行終わったら次の行へ
		vdp->cmd_ny_count--;
		vdp->ny--;			// NYは更新される？
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx;
			if ( (vdp->cmd_arg & 0x4) == 0 ) {
				vaddr_mod -= vdp->nx;
			} else {
				vaddr_mod += vdp->nx;
			}
			if ( (vdp->cmd_arg & 0x8) == 0 ) {
				vaddr_mod += 256;
			} else {
				vaddr_mod -= 256;
			}
		} else {
			// 全部終わったらCEビットをクリア
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// 後処理
	vdp->cmd_vram_addr = vaddr_mod / 2;		// VRAMアドレスを更新
	vdp->cmd_vram_addr_mod = vaddr_mod % 2;	// VRAMアドレスの1バイト内の位置を更新
}

void _HMMV_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMV START G4********\n");

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint16_t dst_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, NULL);

	int x,y;
	for(y=0;y<vdp->ny;y++) {
		for(x=0;x<vdp->nx;x+=2) {
			uint8_t data = vdp->clr;
			// VRAMに書き込む
			vdp->vram[dst_vram_addr] = data;
			uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
			gram[0] = data >> 4;
			gram[1] = data & 0x0f;
			// DIXに従ってVRAMアドレスを更新
			if ((vdp->cmd_arg & 0x04) == 0) {
				dst_vram_addr += 1;
			} else {
				dst_vram_addr -= 1;
			}
		}
		if ( (vdp->cmd_arg & 0x4) == 0 ) {
			dst_vram_addr -= vdp->nx / 2;
		} else {
			dst_vram_addr += vdp->nx / 2;
		}
		if ( (vdp->cmd_arg & 0x8) == 0 ) {
			dst_vram_addr += 128;
		} else {
			dst_vram_addr -= 128;
		}
	}
}

void _YMMM_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("YMMM START G4********\n");

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint16_t src_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->sy, NULL);
	uint16_t dst_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, NULL);

	// DIXによってX方向のどちらの画面端まで転送するかが変わるのでnxが変化する
	int nx = (vdp->cmd_arg & 0x04) == 0 ? (256-vdp->dx) : vdp->dx;

	int x,y;
	for(y=0; y < vdp->ny; y++) {
		uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
		for(x=0; x < nx; x+=2) {	// 2ドットずつ処理
			uint8_t data = vdp->vram[src_vram_addr];
			// VRAMに書き込む
			vdp->vram[dst_vram_addr] = data;
			*gram++ = data >> 4;
			*gram++ = data & 0x0f;
			// DIXに従ってVRAMアドレスを更新
			if ((vdp->cmd_arg & 0x04) == 0) {
				// DIX=0の時
				src_vram_addr += 1;
				dst_vram_addr += 1;
			} else {
				// DIX=1の時
				src_vram_addr -= 1;
				dst_vram_addr -= 1;
			}
		}
		if ( (vdp->cmd_arg & 0x4) == 0 ) {
			src_vram_addr -= vdp->nx / 2;
			dst_vram_addr -= vdp->nx / 2;
		} else {
			src_vram_addr += vdp->nx / 2;
			dst_vram_addr += vdp->nx / 2;
		}
		if ( (vdp->cmd_arg & 0x8) == 0 ) {
			src_vram_addr += 128;
			dst_vram_addr += 128;
		} else {
			src_vram_addr -= 128;
			dst_vram_addr -= 128;
		}
	}
}

void _HMMM_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMM START G4**\n");
	printf("  sx=0x%03x, sy=0x%03x\n", vdp->sx, vdp->sy);
	printf("  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
	printf("  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint16_t src_vram_addr = get_vram_add_G4(vdp, vdp->sx, vdp->sy, NULL);
	uint16_t dst_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, NULL);

	int x,y;
	for(y=0; y < vdp->ny; y++) {
		uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
		for(x=0; x < vdp->nx; x+=2) {	// 2ドットずつ処理
			uint8_t data = vdp->vram[src_vram_addr];
			//data = 055;	// テスト用
			// VRAMに書き込む
			vdp->vram[dst_vram_addr] = data;
			gram[0] = data >> 4;
			gram[1] = data & 0x0f;
			gram[512*256] = data >> 4;				// 円筒スクロール仮対応
			gram[512*256+1] = data & 0x0f;			// 
			gram += 2;
			// DIXに従ってVRAMアドレスを更新
			if ((vdp->cmd_arg & 0x04) == 0) {
				// DIX=0の時
				src_vram_addr += 1;
				dst_vram_addr += 1;
			} else {
				// DIX=1の時
				src_vram_addr -= 1;
				dst_vram_addr -= 1;
			}
		}
		if ( (vdp->cmd_arg & 0x4) == 0 ) {
			src_vram_addr -= vdp->nx / 2;
			dst_vram_addr -= vdp->nx / 2;
		} else {
			src_vram_addr += vdp->nx / 2;
			dst_vram_addr += vdp->nx / 2;
		}
		if ( (vdp->cmd_arg & 0x8) == 0 ) {
			src_vram_addr += 128;
			dst_vram_addr += 128;
		} else {
			src_vram_addr -= 128;
			dst_vram_addr -= 128;
		}
	}
}

void _HMMC_exe_G4(ms_vdp_t* vdp, uint8_t value);
static uint8_t debug_count = 0;

void _HMMC_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMC START G4********\n");

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, NULL);

	vdp->cmd_nx_count = vdp->nx/2;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CEビットをセット
	_HMMC_exe_G4(vdp, vdp->clr);			// 最初の1バイトを書き込む
}

void _HMMC_exe_G4(ms_vdp_t* vdp, uint8_t value) {
	//printf("HMMC exe G4: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CEビットをクリア
		vdp->cmd_current = 0;
		return;
	}
	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t* gram = to_gram(vdp, vaddr, 0);

	//value = debug_count;
	//debug_count = (debug_count + 1) % 16;

	// VRAMに書き込む
	vdp->vram[vaddr] = value; 	// vdp->clr と同じ値のはず
	gram[0] = value >> 4;
	gram[1] = value & 0x0f;

	vdp->s02 |= 0x80;				// TRビットをセット
	vdp->cmd_nx_count-=1;
	// DIXに従ってVRAMアドレスを更新
	// DIXに従ってVRAMアドレスを更新
	if ((vdp->cmd_arg & 0x4)==0) {
		vaddr += 1;
	} else {
		vaddr -= 1;
	}
	if(vdp->cmd_nx_count == 0) {
		// 1行終わったら次の行へ
		vdp->cmd_ny_count--;
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx/2;
			if ( (vdp->cmd_arg & 0x4) == 0 ) {
				vaddr -= vdp->nx / 2;
			} else {
				vaddr += vdp->nx / 2;
			}
			if ( (vdp->cmd_arg & 0x8) == 0 ) {
				vaddr += 128;
			} else {
				vaddr -= 128;
			}
		} else {
			// 全部終わったらCEビットをクリア
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// 後処理
	vdp->cmd_vram_addr = vaddr;		// VRAMアドレスを更新
}


void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd) {
	uint8_t command = (cmd & 0b11110000) >> 4;
	int logiop = cmd & 0b00001111;
	switch(command){
	// case 0b0100: // POINT
	// 	break;
	case 0b0101: // PSET
		_PSET_G4(vdp, command);
		break;
	// case 0b0110: // SRCH
	// 	break;
	// case 0b0111: // LINE
	// 	break;
	// case 0b1000: // LMMV
	// 	break;
	// case 0b1001: // LMMM
	// 	break;
	// case 0b1010: // LMCM
	// 	break;
	case 0b1011: // LMMC
		_LMMC_G4(vdp, command, logiop);
		break;
	case 0b1100: // HMMV
	 	_HMMV_G4(vdp, command);
	 	break;
	case 0b1101: // HMMM
	 	_HMMM_G4(vdp, command);
	 	break;
	case 0b1110: // YMMM
		_YMMM_G4(vdp, command);
	 	break;
	case 0b1111: // HMMC
		_HMMC_G4(vdp, command);
		break;
	default:
		vdp_command_exec_DEFAULT(vdp, cmd);
		break;
	}
	return;
}

uint8_t vdp_command_read_GRAPHIC4(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write_GRAPHIC4(ms_vdp_t* vdp, uint8_t value) {
	switch(vdp->cmd_current) {
	case 0b0100: // POINT
		break;
	case 0b0101: // PSET
		break;
	case 0b0110: // SRCH
		break;
	case 0b0111: // LINE
		break;
	case 0b1000: // LMMV
		break;
	case 0b1001: // LMMM
		break;
	case 0b1010: // LMCM
		break;
	case 0b1011: // LMMC
		_LMMC_exe_G4(vdp, value);
		break;
	case 0b1100: // HMMV
		break;
	case 0b1101: // HMMM
		break;
	case 0b1110: // YMMM
		break;
	case 0b1111: // HMMC
		_HMMC_exe_G4(vdp, value);
		break;
	default:
		break;
	}
}

void update_resolution_GRAPHIC4(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16色, BG不使用
}