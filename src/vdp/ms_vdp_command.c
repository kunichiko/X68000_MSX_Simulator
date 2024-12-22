/*
	VDP �R�}���h����
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_vdp.h"

void ms_vdp_slice_exec(ms_vdp_t* vdp);

uint16_t get_Y_from_vaddr(ms_vdp_t* vdp, uint32_t vaddr) {
	switch(vdp->crt_mode) {
	case CRT_MODE_GRAPHIC4:
		return (vaddr & 0x1ff80) >> 7;
	case CRT_MODE_GRAPHIC5:
		return (vaddr & 0x1ff80) >> 7;
	case CRT_MODE_GRAPHIC6:
		return (vaddr & 0x1ff00) >> 8;
	case CRT_MODE_GRAPHIC7:
		return (vaddr & 0x1ff00) >> 8;
	}
	return 0;
}

void ms_vdp_update_sprite_area(ms_vdp_t* vdp) {
	uint16_t Y_start = 65535;
	uint16_t Y_end = 0;
	uint16_t Y;

	Y = get_Y_from_vaddr(vdp, (vdp->sprattrtbl_baddr & 0x1fe00) );
	if ( Y < Y_start ) Y_start = Y;
	Y = get_Y_from_vaddr(vdp, (vdp->sprattrtbl_baddr & 0x1fe00) + 4*32 - 1);
	if ( Y > Y_end ) Y_end = Y;

	Y = get_Y_from_vaddr(vdp, vdp->sprpgentbl_baddr);
	if ( Y < Y_start ) Y_start = Y;
	Y = get_Y_from_vaddr(vdp, vdp->sprpgentbl_baddr + 8*256 - 1);
	if ( Y > Y_end ) Y_end = Y;

	Y = get_Y_from_vaddr(vdp, vdp->sprcolrtbl_baddr);
	if ( Y < Y_start ) Y_start = Y;
	Y = get_Y_from_vaddr(vdp, vdp->sprcolrtbl_baddr + 16*32 - 1);
	if ( Y > Y_end ) Y_end = Y;

	vdp->cmd_ny_sprite_start = Y_start;
	vdp->cmd_ny_sprite_end = Y_end;

	MS_LOG(MS_LOG_TRACE,"ms_vdp_update_sprite_area: %d - %d\n", Y_start, Y_end);
}

/**
 * @brief 
 * 
 * @param vdp 
 * @param start_y VDP�R�}���h�̉e�����󂯂��ŏ���Y���W
 * @param num_y VDP�R�}���h�̉e�����󂯂����C����
 * @param DIY �]������ (0: Y����, 1: Y����)
 */
void rewrite_sprite_if_needed(ms_vdp_t* vdp, int start_y, int num_y, int DIY) {
	int end_y;
	if (DIY == 0) {
		end_y = start_y + num_y;
	} else {
		end_y = start_y - num_y;
	}
	if( (vdp->cmd_ny_sprite_start < end_y) && //
		(vdp->cmd_ny_sprite_end >= start_y)) {
		// TODO �p�^�[���W�F�l���[�^�e�[�u���A�J���[�e�[�u���A�A�g���r���[�g�e�[�u�������ꂼ��ʂɌ�������
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_FULL;
	}
}

void rewrite_sprite_if_needed_addr(ms_vdp_t* vdp, int addr) {
	// TODO �p�^�[���W�F�l���[�^�e�[�u���A�J���[�e�[�u���A�A�g���r���[�g�e�[�u�������ꂼ��ʂɌ�������
}

/*
	DX, DY���� VRAM�A�h���X�����߂�

	GRAPHIC4	: addr = DY�~128 + dx/2
	GRAPHIC5	: addr = DY�~128 + dx/4
	GRAPHIC6	: addr = DY�~256 + dx/2
	GRAPHIC7	: addr = DY�~256 + dx
*/
inline uint32_t get_vram_address(ms_vdp_t* vdp, uint32_t x, uint32_t y, int* mod) {
	switch(vdp->crt_mode) {
	case CRT_MODE_GRAPHIC4:
		if (mod != NULL) {
			*mod = x & 0x01;
		}
		return (y&0x3ff)*128 + (x&0xff)/2;
	case CRT_MODE_GRAPHIC5:
		if (mod != NULL) {
			*mod = x & 0x03;
		}
		return (y&0x3ff)*128 + (x&0x1ff)/4;
	case CRT_MODE_GRAPHIC6:
		if (mod != NULL) {
			*mod = x & 0x01;
		}
		return (y&0x1ff)*256 + (x&0x1ff)/2;
	case CRT_MODE_GRAPHIC7:
		if (mod != NULL) {
			*mod = 0;
		}
		return (y&0x1ff)*256 + (x&0xff);
	}
	MS_LOG(MS_LOG_ERROR,"get_vram_address: unknown CRT mode\n");
	return 0;
}

/*
	VRAM�A�h���X����X68000����GRAM�A�h���X�����߂�
*/
inline uint16_t* to_gram(ms_vdp_t* vdp, uint32_t vaddr, int mod) {
	uint16_t p;
	uint16_t y;
	uint16_t x;
	switch(vdp->crt_mode) {
	case CRT_MODE_GRAPHIC4:
		p = (vaddr & 0x18000) >> 15;
		y = (vaddr & 0x07f80) >> 7;
		x = (vaddr & 0x0007f);		// X���W��1/2�Ȃ̂Œ���
		return X68_GRAM + (p*512*512) + (y*512) + (x*2) + mod;
	case CRT_MODE_GRAPHIC5:
		p = (vaddr & 0x18000) >> 15;
		y = (vaddr & 0x07f80) >> 7;
		x = (vaddr & 0x0007f);		// X���W��1/4�Ȃ̂Œ���
		return X68_GRAM + (p*512*512) + (y*512)*2 + (x*4) + mod;
	case CRT_MODE_GRAPHIC6:
		p = (vaddr & 0x10000) >> 16;
		y = (vaddr & 0x0ff00) >> 8;
		x = (vaddr & 0x000ff);		// X���W��1/2�Ȃ̂Œ���
		return X68_GRAM + (p*512*512) + (y*512)*2 + (x*2) + mod;
	case CRT_MODE_GRAPHIC7:
		p = (vaddr & 0x10000) >> 16;
		y = (vaddr & 0x0ff00) >> 8;
		x = (vaddr & 0x000ff);		// X���W�ƈ�v
		return X68_GRAM + (p*512*512) + (y*512) + x;
	}
}

inline uint8_t logical_operation(uint8_t dst, uint8_t src, uint8_t logiop) {
	switch(logiop) {
	case 0x0:	// IMP
		dst = src;
		break;
	case 0x1:	// AND
		dst = src & dst;
		break;
	case 0x2:	// OR
		dst = src | dst;
		break;
	case 0x3:	// XOR
		dst = src ^ dst;
		break;
	case 0x4:	// NOT
		dst = !src;
		break;
	case 0x8:	// TIMP
		dst = src == 0 ? dst : src;
		break;
	case 0x9:	// TAND
		dst = src == 0 ? dst : src & dst;
		break;
	case 0xa:	// TOR
		dst = src == 0 ? dst : src | dst;
		break;
	case 0xb:	// TXOR
		dst = src == 0 ? dst : src ^ dst;
		break;
	case 0xc:	// TNOT
		dst = src == 0 ? dst : !src;
		break;
	}
	return dst;
}

inline uint8_t read_vram_logical(ms_vdp_t* vdp, uint32_t vaddr, uint16_t vamod) {
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	uint8_t value = vdp->vram[vaddr];
	value = (value >> ((dots_per_byte-1-vamod)*bits_per_dot)) & ((1<<bits_per_dot)-1);
	return value;
}
/**
 * @brief VRAM�ɘ_���]�����܂�
 * 
 * @param vdp 
 * @param vaddr 
 * @param vamod 
 * @param value 
 * @return uint8_t �_���s�N�Z���̏����������N�������ꍇ1�A�N��Ȃ������ꍇ0
 */
inline uint8_t write_vram_logical(ms_vdp_t* vdp, uint32_t vaddr, uint16_t vamod, uint8_t value) {
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	// VRAM�}�X�N�p�^�[��
	// GRAPHIC4: 0b11111111_00001111
	// GRAPHIC5: 0b11111111_00111111
	// GRAPHIC6: 0b11111111_00001111
	// GRAPHIC7: 0b11111111_00000000
	uint16_t mask = ~(((1<<bits_per_dot)-1) << (8-(bits_per_dot)));	// �����ʒu�͈�ԍ��̃s�N�Z��
	mask >>= vamod*bits_per_dot;							// ����������ꏊ��000������悤�ɃV�t�g
	uint8_t src = vdp->vram[vaddr];
	uint8_t val = value << ((8-(vamod+1)*bits_per_dot));	// �������ޒl���r�b�g�ʒu�ɍ��킹��
	if ((src & ~mask) == val) {
		// �ύX���Ȃ��Ȃ珑�������Ȃ�
		return 0;
	}
	uint8_t dst = src & mask;
	dst |= val;
	vdp->vram[vaddr] = dst;
	return 1;
}


/*
	PSET
*/
void cmd_PSET_exe(ms_vdp_t* vdp, uint16_t x, uint16_t y, uint8_t color, uint8_t logiop);

void cmd_PSET(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	uint8_t color = vdp->clr & ((1<<vdp->ms_vdp_current_mode->bits_per_dot)-1);
	cmd_PSET_exe(vdp, vdp->dx, vdp->dy, vdp->clr, logiop);
}

void cmd_PSET_exe(ms_vdp_t* vdp, uint16_t x, uint16_t y, uint8_t color, uint8_t logiop) {
	int vamod;
	uint32_t vaddr = get_vram_address(vdp, x, y, &vamod);

	// VRAM����f�[�^��ǂݏo��
	uint8_t dst = read_vram_logical(vdp, vaddr, vamod);

	// �_�����Z���s
	dst = logical_operation(dst, color, logiop);

	// VRAM�ɏ�������
	if( write_vram_logical(vdp, vaddr, vamod, dst) ) {
		// GRAM�ɏ�������
		uint16_t* gram = to_gram(vdp, vaddr, vamod);
		gram[0] = dst;
		if(vdp->ms_vdp_current_mode->crt_width == 256) gram[256*512] = dst;
	}
}

/*
	SRCH
*/
void cmd_SRCH(ms_vdp_t* vdp, uint8_t cmd) {
	int mod;
	uint32_t vram_addr;
	int i;
	int sx = vdp->sx;
	uint8_t color = vdp->clr & ((1<<vdp->ms_vdp_current_mode->bits_per_dot)-1);
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	uint8_t DIX = vdp->arg & 0x04;
	uint8_t EQ = vdp->arg & 0x02;

	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"SRCH START****\n");
		MS_LOG(MS_LOG_FINE,"  sx=0x%03x, sy=0x%03x\n", vdp->sx, vdp->sy);
		MS_LOG(MS_LOG_FINE,"  dix=0x%02x, eq=0x%1x clr=0x%02x\n", DIX, EQ, vdp->clr);
	}

	vdp->s02 = 0x00;
	while(1) {
		vram_addr = get_vram_address(vdp, sx, vdp->sy, &mod);
		uint8_t src = read_vram_logical(vdp, vram_addr, mod);
		if (EQ == 0) { // 0 = ���E�F�𔭌������Ƃ��Ɏ��s���I������B
			if (src == color) {
				vdp->s02 = 0x10;
				vdp->s08 = (sx & 0xff);
				vdp->s09 = (sx & 0xff00) >> 8;
				break;
			}
		} else { // 1 = ���E�F�ȊO�𔭌������Ƃ��Ɏ��s���I������B
			if (src != color) {
				vdp->s02 = 0x10;
				vdp->s08 = (sx & 0xff);
				vdp->s09 = (sx & 0xff00) >> 8;
				break;
			}
		}
		// DIX�ɏ]����VRAM�A�h���X���X�V
		if (DIX == 0) {
			// DIX=0�̎�
			if(sx == crt_width-1) {
				break;
			}
			sx++;
		} else {
			// DIX=1�̎�
			if (sx == 0) {
				break;
			}
			sx--;
		}
	}

}

/*
	LINE
*/
void cmd_LINE(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"LINE START****\n");
		MS_LOG(MS_LOG_FINE,"  sx=0x%03x, sy=0x%03x\n", vdp->sx, vdp->sy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
	}

	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	uint8_t color = vdp->clr & ((1<<vdp->ms_vdp_current_mode->bits_per_dot)-1);

	int x = vdp->dx;	// �J�n�_
	int y = vdp->dy;
	int maj = vdp->nx;	// ���ӂ̒���
	int min = vdp->ny;	// �Z�ӂ̒���
	uint32_t acc = 0;
	int i;
	if( (vdp->arg & 0x1) == 0 ) { //MAJ
		// ���ӂ�X���ƕ��s�ȏꍇ
		for(i=0;i<=maj;i++) {
			cmd_PSET_exe(vdp, x, y, color, logiop);
			acc += min;
			if (acc >= maj) {
				acc -= maj;
				y += 1;
			}
			x++;
		}
	} else {
		// ���ӂ�Y���ƕ��s�ȏꍇ
		for(i=0;i<=maj;i++) {
			cmd_PSET_exe(vdp, x, y, color, logiop);
			acc += min;
			if (acc >= maj) {
				acc -= maj;
				x += 1;
			}
			y++;
		}
	}
}


/*
	LMMV
*/
void cmd_LMMV(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"LMMV START****\n");
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
	}

	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	uint8_t DIX = vdp->arg & 0x04;
	uint8_t DIY = vdp->arg & 0x08;
	uint16_t nx = vdp->nx == 0 ? crt_width : vdp->nx;
	uint16_t ny = vdp->ny == 0 ? crt_width : vdp->ny;
	uint8_t clr = vdp->clr & ((1<<vdp->ms_vdp_current_mode->bits_per_dot)-1);

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	int dst_mod;
	uint32_t dst_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, &dst_mod);
	uint32_t dst_vram_addr_mod = dst_vram_addr * dots_per_byte + dst_mod;

	int x,y,i;
	for(y=0; y < ny; y++) {
		dst_vram_addr = dst_vram_addr_mod / dots_per_byte;
		dst_mod = dst_vram_addr_mod % dots_per_byte;
		uint16_t* gram = to_gram(vdp, dst_vram_addr, dst_mod);
		for(x=0; x < nx; x+=1) {	// 1�h�b�g������
			dst_vram_addr = dst_vram_addr_mod / dots_per_byte;
			dst_mod = dst_vram_addr_mod % dots_per_byte;
			// VRAM����f�[�^��ǂݏo��
			uint8_t dst = read_vram_logical(vdp, dst_vram_addr, dst_mod);
			// �_�����Z���s
			dst = logical_operation(dst, clr, logiop);
			// VRAM�ɏ�������
			if( write_vram_logical(vdp, dst_vram_addr, dst_mod, dst) ) {
				// GRAM�ɏ�������
				*gram = dst;
				if(crt_width == 256) gram[256*512] = dst;
			}
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if (DIX == 0) {
				// DIX=0�̎�
				dst_vram_addr_mod += 1;
				gram++;
			} else {
				// DIX=1�̎�
				dst_vram_addr_mod -= 1;
				gram--;
			}
		}
		if ( DIX == 0 ) {		// DIX
			dst_vram_addr_mod -= vdp->nx;
		} else {
			dst_vram_addr_mod += vdp->nx;
		}
		if ( DIY == 0 ) {		// DIY
			dst_vram_addr_mod += crt_width;
			vdp->dy += 1;						// DY�͏��������
		} else {
			dst_vram_addr_mod -= crt_width;
			vdp->dy -= 1;						// DY�͏��������
		}
	}
}


/*
	LMMM
*/
void cmd_LMMM(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"LMMM START****\n");
		MS_LOG(MS_LOG_FINE,"  sx=0x%03x, sy=0x%03x\n", vdp->sx, vdp->sy);
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
	}

	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	int src_mod;
	int dst_mod;
	uint32_t src_vram_addr = get_vram_address(vdp, vdp->sx, vdp->sy, &src_mod);
	uint32_t dst_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, &dst_mod);
	uint32_t src_vram_addr_mod = src_vram_addr * dots_per_byte + src_mod;
	uint32_t dst_vram_addr_mod = dst_vram_addr * dots_per_byte + dst_mod;

	int x,y,i;
	for(y=0; y < vdp->ny; y++) {
		dst_vram_addr = dst_vram_addr_mod / dots_per_byte;
		dst_mod = dst_vram_addr_mod % dots_per_byte;
		uint16_t* gram = to_gram(vdp, dst_vram_addr, dst_mod);
		for(x=0; x < vdp->nx; x+=1) {	// 1�h�b�g������
			src_vram_addr = src_vram_addr_mod / dots_per_byte;
			dst_vram_addr = dst_vram_addr_mod / dots_per_byte;
			src_mod = src_vram_addr_mod % dots_per_byte;
			dst_mod = dst_vram_addr_mod % dots_per_byte;
			// VRAM����f�[�^��ǂݏo��
			uint8_t src = read_vram_logical(vdp, src_vram_addr, src_mod);
			uint8_t dst = read_vram_logical(vdp, dst_vram_addr, dst_mod);
			// �_�����Z���s
			dst = logical_operation(dst, src, logiop);
			// VRAM�ɏ�������
			if(write_vram_logical(vdp, dst_vram_addr, dst_mod, dst)) {
				// GRAM�ɏ�������
				*gram = dst;
				if(vdp->ms_vdp_current_mode->crt_width == 256) gram[256*512] = dst;
			}
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if ((vdp->cmd_arg & 0x04) == 0) {
				// DIX=0�̎�
				src_vram_addr_mod += 1;
				dst_vram_addr_mod += 1;
				gram++;
			} else {
				// DIX=1�̎�
				src_vram_addr_mod -= 1;
				dst_vram_addr_mod -= 1;
				gram--;
			}
		}
		if ( (vdp->cmd_arg & 0x4) == 0 ) {		// DIX
			src_vram_addr_mod -= vdp->nx;
			dst_vram_addr_mod -= vdp->nx;
		} else {
			src_vram_addr_mod += vdp->nx;
			dst_vram_addr_mod += vdp->nx;
		}
		if ( (vdp->cmd_arg & 0x8) == 0 ) {		// DIY
			src_vram_addr_mod += crt_width;
			dst_vram_addr_mod += crt_width;
			vdp->sy += 1;						// DY�͏��������
			vdp->dy += 1;						// DY�͏��������
		} else {
			src_vram_addr_mod -= crt_width;
			dst_vram_addr_mod -= crt_width;
			vdp->sy -= 1;						// DY�͏��������
			vdp->dy -= 1;						// DY�͏��������
		}
	}
}


/*
	LMMC
*/
void cmd_LMMC_exe(ms_vdp_t* vdp, uint8_t value);

void cmd_LMMC(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	MS_LOG(MS_LOG_FINE,"LMMC START********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_logiop = logiop;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, &mod);
	vdp->cmd_vram_addr_mod = mod;

	vdp->cmd_nx_count = vdp->nx;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CE�r�b�g���Z�b�g
	cmd_LMMC_exe(vdp, vdp->clr);			// �ŏ���1�h�b�g����������
}

void cmd_LMMC_exe(ms_vdp_t* vdp, uint8_t value) {
	MS_LOG(MS_LOG_FINE,"LMMC exe: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CE�r�b�g���N���A
		vdp->cmd_current = 0;
		return;
	}
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	uint8_t logiop = vdp->cmd_logiop;
	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t vamod = vdp->cmd_vram_addr_mod;
	uint32_t vaddr_mod = vaddr * dots_per_byte + vamod;
	uint16_t* gram = to_gram(vdp, vaddr, vamod);

	// VRAM����f�[�^��ǂݏo��
	uint8_t dst = read_vram_logical(vdp, vaddr, vamod);

	// �_�����Z���s
	dst = logical_operation(dst, value, logiop);

	// VRAM�ɏ�������
	if(write_vram_logical(vdp, vaddr, vamod, dst)){
		// GRAM�ɏ�������
		*gram = dst;
		if(vdp->ms_vdp_current_mode->crt_width == 256) gram[256*512] = dst;
	}

	vdp->s02 |= 0x80;				// TR�r�b�g���Z�b�g
	vdp->cmd_nx_count-=1;
	// DIX�ɏ]����VRAM�A�h���X���X�V
	if ((vdp->cmd_arg & 0x4)==0) {
		vaddr_mod += 1;
	} else {
		vaddr_mod -= 1;
	}
	if(vdp->cmd_nx_count == 0) {
		// 1�s�I������玟�̍s��
		vdp->cmd_ny_count--;
		vdp->ny--;			// NY�͍X�V�����H
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx;
			if ( (vdp->cmd_arg & 0x4) == 0 ) {		// DIX
				vaddr_mod -= vdp->nx;
			} else {
				vaddr_mod += vdp->nx;
			}
			if ( (vdp->cmd_arg & 0x8) == 0 ) {		// DIY
				vaddr_mod += crt_width;
				vdp->dy += 1;						// DY�͏��������
			} else {
				vaddr_mod -= crt_width;
				vdp->dy -= 1;						// DY�͏��������
			}
		} else {
			// �S���I�������CE�r�b�g���N���A
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// �㏈��
	vdp->cmd_vram_addr = vaddr_mod / dots_per_byte;		// VRAM�A�h���X���X�V
	vdp->cmd_vram_addr_mod = vaddr_mod % dots_per_byte;	// VRAM�A�h���X��1�o�C�g���̈ʒu���X�V

	//rewrite_sprite_if_needed_addr(vdp, vdp->sy, vdp->ny, vdp->);
}


/*
	HMMV
*/
void cmd_HMMV(ms_vdp_t* vdp, uint8_t cmd) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"HMMV START********\n");
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
		MS_LOG(MS_LOG_FINE,"  arg=%02x\n", vdp->arg);
	}

	// �������̂��߂̃L���b�V��
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	uint8_t* vram = vdp->vram;
	uint8_t DIX = vdp->arg & 0x04;
	uint8_t DIY = vdp->arg & 0x08;
	uint16_t nx = vdp->nx == 0 ? crt_width : vdp->nx; // TODO �h���N�G2��NX=0���g���Ă���
	nx &= ~(dots_per_byte-1);	// 1�s�̃h�b�g����dots_per_byte�̔{���ɂ���
	uint16_t ny = vdp->ny;
	uint8_t clr = vdp->clr;

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint32_t dst_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, NULL);

	uint16_t start_dy = vdp->dy;
	int x,y,i;
	for(y=0; y < ny; y++) {
		uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
		for(x=0;x < nx; x+=dots_per_byte) {
			uint8_t data = clr;
			// VRAM�ɏ�������
			if( vram[dst_vram_addr] != data ) {
				vram[dst_vram_addr] = data;
				// GRAM�ɏ�������
				for(i=0; i < dots_per_byte; i++) {
					uint16_t dst = (data >> ((dots_per_byte-1-i)*bits_per_dot)) & ((1<<bits_per_dot)-1);
					gram[0+i] = dst;
					if(crt_width == 256) gram[256*512+i] = dst;
				}
			}
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if ( DIX == 0) {
				dst_vram_addr += 1;
				gram += dots_per_byte;
			} else {
				dst_vram_addr -= 1;
				gram -= dots_per_byte;
			}
		}
		if ( DIX == 0 ) {
			dst_vram_addr -= nx / dots_per_byte;
		} else {
			dst_vram_addr += nx / dots_per_byte;
		}
		if ( DIY == 0 ) {
			dst_vram_addr += crt_width / dots_per_byte;
			vdp->dy += 1;						// DY�͏��������
		} else {
			dst_vram_addr -= crt_width / dots_per_byte;
			vdp->dy -= 1;						// DY�͏��������
		}
	}

	rewrite_sprite_if_needed(vdp, start_dy, ny, DIY);
}

void cmd_YMMM(ms_vdp_t* vdp, uint8_t cmd) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"YMMM START********\n");
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, sy=0x%03x\n", vdp->dx, vdp->sy);
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"             ny=0x%03x\n",          vdp->ny);
	}
	// �������̂��߂̃L���b�V��
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	uint8_t* vram = vdp->vram;
	uint8_t DIX = vdp->arg & 0x04;
	uint8_t DIY = vdp->arg & 0x08;

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint32_t src_vram_addr = get_vram_address(vdp, vdp->dx, vdp->sy, NULL);
	uint32_t dst_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, NULL);

	// DIX�ɂ����X�����̂ǂ���̉�ʒ[�܂œ]�����邩���ς��̂�nx���ω�����
	int nx = DIX == 0 ? (crt_width-vdp->dx) : vdp->dx+1;
	uint16_t ny = vdp->ny;

	uint16_t start_dy = vdp->dy;
	int x,y,i;
	for(y=0; y < ny; y++) {
		uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
		for(x=0; x < nx; x+=dots_per_byte) {	// dots_per_byte �h�b�g������(1�o�C�g����)����
			uint8_t data = vram[src_vram_addr];
			// VRAM�ɏ�������
			if( vram[dst_vram_addr] != data ) {
				vram[dst_vram_addr] = data;
				// GRAM�ɏ�������
				for(i=0; i < dots_per_byte; i++) {
					uint16_t dst = (data >> ((dots_per_byte-1-i)*bits_per_dot)) & ((1<<bits_per_dot)-1);
					gram[0+i] = dst;
					if(crt_width == 256) gram[256*512+i] = dst;
				}
			}
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if ((vdp->cmd_arg & 0x04) == 0) {
				// DIX=0�̎�
				src_vram_addr += 1;
				dst_vram_addr += 1;
				gram += dots_per_byte;
			} else {
				// DIX=1�̎�
				src_vram_addr -= 1;
				dst_vram_addr -= 1;
				gram -= dots_per_byte;
			}
		}
		if ( DIX == 0 ) {
			src_vram_addr -= nx / dots_per_byte;
			dst_vram_addr -= nx / dots_per_byte;
		} else {
			src_vram_addr += nx / dots_per_byte;
			dst_vram_addr += nx / dots_per_byte;
		}
		if ( DIY == 0 ) {
			src_vram_addr += crt_width / dots_per_byte;
			dst_vram_addr += crt_width / dots_per_byte;
			vdp->sy += 1;						// DY�͏��������
			vdp->dy += 1;						// DY�͏��������
		} else {
			src_vram_addr -= crt_width / dots_per_byte;
			dst_vram_addr -= crt_width / dots_per_byte;
			vdp->sy -= 1;						// DY�͏��������
			vdp->dy -= 1;						// DY�͏��������
		}
	}

	rewrite_sprite_if_needed(vdp, start_dy, ny, DIY);
}

int cmd_HMMM_exe(ms_vdp_t* vdp);

void cmd_HMMM(ms_vdp_t* vdp, uint8_t cmd) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"HMMM START****\n");
		MS_LOG(MS_LOG_FINE,"  sx=0x%03x, sy=0x%03x\n", vdp->sx, vdp->sy);
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
	}

	// �������̂��߂̃L���b�V��
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;
	uint8_t* vram = vdp->vram;
	uint8_t DIX = vdp->arg & 0x04;
	uint8_t DIY = vdp->arg & 0x08;
	uint16_t nx = vdp->nx == 0 ? crt_width : vdp->nx; // TODO �h���N�G2��NX=0���g���Ă���
	nx &= ~(dots_per_byte-1);	// 1�s�̃h�b�g����dots_per_byte�̔{���ɂ���
	uint16_t ny = vdp->ny;

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint32_t src_vram_addr = get_vram_address(vdp, vdp->sx, vdp->sy, NULL);
	uint32_t dst_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, NULL);

	vdp->cmd_context.y = 0;
	vdp->cmd_context.nx = nx;
	vdp->cmd_context.ny = ny;
	vdp->cmd_context.nxbyte = nx / dots_per_byte;
	vdp->cmd_context.widthbyte = crt_width / dots_per_byte;
	vdp->cmd_context.src_vram_addr = src_vram_addr;
	vdp->cmd_context.dst_vram_addr = dst_vram_addr;

	vdp->s02 |= 0x01;						// CE�r�b�g���Z�b�g
	vdp->current_command_exec = cmd_HMMM_exe;

	// 1�s���������s
	ms_vdp_slice_exec(vdp);
}


/**
 * @brief 
 * 
 * @param vdp 
 * 
 * @return int 0: �]���I��, 1: �]���r��
 */
int cmd_HMMM_exe(ms_vdp_t* vdp) {
	ms_vdp_cmd_ctx_t* context = &vdp->cmd_context;
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	uint8_t* vram = vdp->vram;
	uint8_t DIX = vdp->cmd_arg & 0x04;
	uint8_t DIY = vdp->cmd_arg & 0x08;

	int i;
	int x;
	int y = context->y;
	int nx = context->nx;
	int ny = context->ny;
	uint32_t src_vram_addr = context->src_vram_addr;
	uint32_t dst_vram_addr = context->dst_vram_addr;
	uint16_t dotmask = (1<<bits_per_dot)-1;
	int nxbyte = context->nxbyte;
	int widthbyte = context->widthbyte;

	uint16_t start_dy = vdp->dy;	// dy�͖��ƍX�V����Ă���̂ŁA���̎��_�ł͍����exe�ɂ��]���J�n�s�ɂȂ��Ă���
	uint32_t byte_count = 0;
	for(; y < ny; y++) {
		if (byte_count >= 0x100) {
			rewrite_sprite_if_needed(vdp, start_dy, y - context->y, DIY);
			context->y = y;
			context->src_vram_addr = src_vram_addr;
			context->dst_vram_addr = dst_vram_addr;
			return 1;
		}
		uint16_t* gram = to_gram(vdp, dst_vram_addr, 0);
		for(x=0; x < nx; x+=dots_per_byte) {	// dots_per_byte �h�b�g������(1�o�C�g����)����
			byte_count++;
			uint8_t data = vram[src_vram_addr];
			// VRAM�ɏ�������
			if( vram[dst_vram_addr] != data ) {
				vram[dst_vram_addr] = data;
				// GRAM�ɏ�������
				for(i=dots_per_byte-1; i >= 0; i--) {
					//uint16_t dst = (data >> ((dots_per_byte-1-i)*bits_per_dot)) & dotmask;
					uint16_t dst = data & dotmask;
					data >>= bits_per_dot;
					gram[0+i] = dst;
					if(crt_width == 256) gram[256*512+i] = dst;
				}
			}
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if (DIX == 0) {
				// DIX=0�̎�
				src_vram_addr += 1;
				dst_vram_addr += 1;
				gram += dots_per_byte;
			} else {
				// DIX=1�̎�
				src_vram_addr -= 1;
				dst_vram_addr -= 1;
				gram -= dots_per_byte;
			}
		}
		if ( DIX == 0 ) {
			src_vram_addr -= nxbyte;
			dst_vram_addr -= nxbyte;
		} else {
			src_vram_addr += nxbyte;
			dst_vram_addr += nxbyte;
		}
		if ( DIY == 0 ) {
			src_vram_addr += widthbyte;
			dst_vram_addr += widthbyte;
			vdp->sy += 1;						// DY�͏��������
			vdp->dy += 1;						// DY�͏��������
		} else {
			src_vram_addr -= widthbyte;
			dst_vram_addr -= widthbyte;
			vdp->sy -= 1;						// DY�͏��������
			vdp->dy -= 1;						// DY�͏��������
		}
	}

	rewrite_sprite_if_needed(vdp, start_dy, y - context->y, DIY);

	return 0;
}

/*
	HMMC
*/
void cmd_HMMC_exe(ms_vdp_t* vdp, uint8_t value);
static uint8_t debug_count = 0;

void cmd_HMMC(ms_vdp_t* vdp, uint8_t cmd) {
	if(MS_LOG_FINE_ENABLED) {
		MS_LOG(MS_LOG_FINE,"HMMC START********\n");
		MS_LOG(MS_LOG_FINE,"  dx=0x%03x, dy=0x%03x\n", vdp->dx, vdp->dy);
		MS_LOG(MS_LOG_FINE,"  nx=0x%03x, ny=0x%03x\n", vdp->nx, vdp->ny);
	}

	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_address(vdp, vdp->dx, vdp->dy, NULL);

	vdp->cmd_nx_count = vdp->nx/dots_per_byte;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CE�r�b�g���Z�b�g
	cmd_HMMC_exe(vdp, vdp->clr);			// �ŏ���1�o�C�g����������
}

void cmd_HMMC_exe(ms_vdp_t* vdp, uint8_t value) {
	//MS_LOG(MS_LOG_FINE,"HMMC exe G4: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CE�r�b�g���N���A
		vdp->cmd_current = 0;
		//rewrite_sprite_if_needed(vdp);
		return;
	}
	int	crt_width = vdp->ms_vdp_current_mode->crt_width;
	int dots_per_byte = vdp->ms_vdp_current_mode->dots_per_byte;
	int bits_per_dot = vdp->ms_vdp_current_mode->bits_per_dot;

	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t* gram = to_gram(vdp, vaddr, 0);

	//value = debug_count;
	//debug_count = (debug_count + 1) % 16;

	// VRAM�ɏ�������
	vdp->vram[vaddr] = value; 	// vdp->clr �Ɠ����l�̂͂�
	// GRAM�ɏ�������
	int i;
	for(i=0; i < dots_per_byte; i++) {
		uint16_t dst = (value >> ((dots_per_byte-1-i)*bits_per_dot)) & ((1<<bits_per_dot)-1);
		gram[0] = dst;
		if(vdp->ms_vdp_current_mode->crt_width == 256) gram[256*512] = dst;
		gram++;
	}

	vdp->s02 |= 0x80;				// TR�r�b�g���Z�b�g
	vdp->cmd_nx_count-=1;
	// DIX�ɏ]����VRAM�A�h���X���X�V
	if ((vdp->cmd_arg & 0x4)==0) {
		vaddr += 1;
	} else {
		vaddr -= 1;
	}
	if(vdp->cmd_nx_count == 0) {
		// 1�s�I������玟�̍s��
		vdp->cmd_ny_count--;
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx / dots_per_byte;
			if ( (vdp->cmd_arg & 0x4) == 0 ) {		// DIX
				vaddr -= vdp->nx / dots_per_byte;
			} else {
				vaddr += vdp->nx / dots_per_byte;
			}
			if ( (vdp->cmd_arg & 0x8) == 0 ) {		// DIY
				vaddr += crt_width / dots_per_byte;
				vdp->dy += 1;						// DY�͏��������
			} else {
				vaddr -= crt_width / dots_per_byte;
				vdp->dy -= 1;						// DY�͏��������
			}
		} else {
			// �S���I�������CE�r�b�g���N���A
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// �㏈��
	vdp->cmd_vram_addr = vaddr;		// VRAM�A�h���X���X�V
}

void ms_vdp_slice_exec(ms_vdp_t* vdp) {
	uint16_t X68_TX_PAL_ORG = X68_TX_PAL[0];
	if(vdp->current_command_exec) {
		if(vdp->hostdebugmode) {
			X68_TX_PAL[0] = (0x1f << 6) | (0x1f << 1);	// Purple
		}
		int ret = vdp->current_command_exec(vdp);
		if(vdp->hostdebugmode) {
			X68_TX_PAL[0] = X68_TX_PAL_ORG;	// �߂�
		}
		if (ret == 0) {
			vdp->s02 &= 0xfe;							// CE�r�b�g���N���A
			vdp->current_command_exec = NULL;
			return;
		}
	}
}

void vdp_command_exec(ms_vdp_t* vdp, uint8_t cmd) {
	MS_LOG(MS_LOG_FINE,"vdp_command_exec: %02x\n", cmd);
	vdp->current_command_exec = NULL;

	uint8_t command = (cmd & 0b11110000) >> 4;
	int logiop = cmd & 0b00001111;
	switch(command){
	case 0b0000: // STOP
	case 0b0001:
	case 0b0010:
	case 0b0011:
		break;
	// case 0b0100: // POINT
	// 	break;
	case 0b0101: // PSET
		cmd_PSET(vdp, command, logiop);
		break;
	case 0b0110: // SRCH
		cmd_SRCH(vdp, command);
		break;
	case 0b0111: // LINE
		cmd_LINE(vdp, command, logiop);
		break;
	case 0b1000: // LMMV
		cmd_LMMV(vdp, command, logiop);
		break;
	case 0b1001: // LMMM
		cmd_LMMM(vdp, command, logiop);
		break;
	// case 0b1010: // LMCM
	// 	break;
	case 0b1011: // LMMC
		cmd_LMMC(vdp, command, logiop);
		break;
	case 0b1100: // HMMV
	 	cmd_HMMV(vdp, command);
	 	break;
	case 0b1101: // HMMM
	 	cmd_HMMM(vdp, command);
	 	break;
	case 0b1110: // YMMM
		cmd_YMMM(vdp, command);
	 	break;
	case 0b1111: // HMMC
		cmd_HMMC(vdp, command);
		break;
	default:
		vdp_command_exec_DEFAULT(vdp, cmd);
		break;
	}
	return;
}

uint8_t vdp_command_read(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write(ms_vdp_t* vdp, uint8_t value) {
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
		cmd_LMMC_exe(vdp, value);
		break;
	case 0b1100: // HMMV
		break;
	case 0b1101: // HMMM
		break;
	case 0b1110: // YMMM
		break;
	case 0b1111: // HMMC
		cmd_HMMC_exe(vdp, value);
		break;
	default:
		break;
	}
}