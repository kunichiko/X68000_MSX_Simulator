#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_DEFAULT(ms_vdp_t* vdp);
uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
void update_palette_DEFAULT(ms_vdp_t* vdp);
void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp);
void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_DEFAULT(ms_vdp_t* vdp);
void update_resolution_DEFAULT(ms_vdp_t* vdp);
void vsync_draw_DEFAULT(ms_vdp_t* vdp);


ms_vdp_mode_t ms_vdp_DEFAULT = {
	// int init_DEFAULT(ms_vdp_t* vdp);
	init_DEFAULT,
	// uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
	read_vram_DEFAULT,
	// void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
	write_vram_DEFAULT,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_DEFAULT,
	// void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_pnametbl_baddr_DEFAULT,
	// void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_colortbl_baddr_DEFAULT,
	// void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp);
	update_pgentbl_baddr_DEFAULT,
	// void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_MODE1,
	// void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_MODE1,
	// void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_DEFAULT,
	// char* get_mode_name_DEFAULT(ms_vdp_t* vdp);
	get_mode_name_DEFAULT,
	// void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_NONE,
	// uint8_t vdp_command_read_DEFAULT(ms_vdp_t* vdp);
	vdp_command_read_NONE,
	// void vdp_command_write_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_NONE,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_DEFAULT,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	0
};


int init_DEFAULT(ms_vdp_t* vdp) {
}

uint8_t read_vram_DEFAULT(ms_vdp_t* vdp) {
	uint8_t ret = vdp->vram[vdp->vram_addr];
	vdp->vram_addr = (vdp->vram_addr + 1) & 0x1ffff;
	return ret;
}

void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data) {
	vdp->vram[vdp->vram_addr] = data;
	vdp->vram_addr = (vdp->vram_addr + 1) & 0x1ffff;
}

/*
*
*	�p���b�g�O�̓o�b�N�h���b�v�Ɏg�p���ꂽ���̂ݗL���B�|���a�f�̃p���b�g�P�ɃZ�b�g����B
*	��TODO �o�b�N�h���b�v��BG���g���̂���߂��̂ōl������
*

	lea.l	P_0,a0
	lea.l	$E82220+2,a1			* �a�f�p�̃p���b�g

	move.w	back_color,d0
	add.w	d0,d0
	move.w	0(a0,d0.w),(a1)			* a0 �� P_0 �̃A�h���X�������Ă���

	lea.l	2(a0),a0
	move.w	#15-1,d0
	lea.l	g_palette+2,a1			* �O���t�B�b�N�p�̃p���b�g
	lea.l	t_palette+32+2,a2		* �X�v���C�g�p�̃p���b�g(0x10?0x1f���g�p)
@@:	move.w	(a0),(a1)+
	move.w	(a0)+,(a2)+
	dbra	d0,@b

	rts*/
inline uint16_t diff_color(uint16_t color1, uint16_t color2) {
	uint16_t diff = 0;
	uint16_t r1,g1,b1;
	uint16_t r2,g2,b2;
	uint16_t r_diff,g_diff,b_diff;
	g1 = (color1 & 0b1111100000000000) >> 11;
	r1 = (color1 & 0b0000011111000000) >> 6;
	b1 = (color1 & 0b0000000000111110) >> 1;
	g2 = (color2 & 0b1111100000000000) >> 11;
	r2 = (color2 & 0b0000011111000000) >> 6;
	b2 = (color2 & 0b0000000000111110) >> 1;
	g_diff = g1 > g2 ? g1 - g2 : g2 - g1;
	r_diff = r1 > r2 ? r1 - r2 : r2 - r1;
	b_diff = b1 > b2 ? b1 - b2 : b2 - b1;
	return g_diff + r_diff + b_diff;
}

void update_palette_DEFAULT(ms_vdp_t* vdp) {
	int i;
	uint16_t back_palette = vdp->palette[vdp->back_color & 0xf];
	int alt_color = 1;
	uint16_t alt_color_diff = diff_color(back_palette, vdp->palette[alt_color]);
	uint16_t color;

	// �����F�͔w�i�F�������Č�����悤�ɂ���
	if (vdp->tx_active) {
		// �P�x�𔼕��ɗ��Ƃ�
		color = back_palette & 0b1111011110111100;
		color >>= 1;
	} else {
		color = back_palette;
	}
	X68_GR_PAL[0] = color;

	// �J���[�R�[�h0�Ƀp���b�g0�̐F�𓖂Ă͂߂�ꍇ�A
	// X68000�̃X�v���C�g�̓J���[�R�[�h0��`��ł��Ȃ��̂Ŏ���15�F���������Ȃ���肪����
	// ���̖��ɑΉ����邽�߂ɁA�w�i�F�Ƃ̍����ł��������F��w�i�F�̑�֐F�Ƃ��đI��
	for(i =1; i < 16; i++) {
		color = vdp->palette[i];
		uint16_t diff = diff_color(back_palette, color);
		if (diff < alt_color_diff) {
			alt_color = i;
			alt_color_diff = diff;
		}
		if (vdp->tx_active) {
			// �P�x�𔼕��ɗ��Ƃ�
			color &= 0b1111011110111100;
			color >>= 1;
		}
		X68_GR_PAL[i] = color;
		X68_SP_PAL_B1[i] = color;
	}
	vdp->alt_color_zero = alt_color;
}

void update_pnametbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R02 �� b17-b10�������Ă���̂ŃV�t�g����
	vdp->pnametbl_baddr = (vdp->_r02 << 10) & 0x1ffff;
}

void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R03 �� b13-b6
	// R10 �� b16-b14
	vdp->colortbl_baddr = ((vdp->_r10 << 14) | (vdp->_r03 << 6)) & 0x1ffff;
}

void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp) {
	// R04 �� b16-b11
	vdp->pgentbl_baddr = (vdp->_r04 << 11) & 0x1ffff;
}

void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp) {
	// R05 �� b14-b7
	// R11 �� b16-b15
	// MODE1�ł́Ab7�܂Ŏg���Ab6-b0��0�Ƃ��Ĉ�����
	uint32_t addr = ((vdp->_r11 << 15) | (vdp->_r05 << 7)) & 0x1ff80;
	if (vdp->sprattrtbl_baddr != addr) {
		// �X�V���ꂽ��A�S�ẴX�v���C�g�̃A�g���r���[�g���Đ�������
		vdp->sprattrtbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void update_sprattrtbl_baddr_MODE2(ms_vdp_t* vdp) {
	// R05 �� b14-b7
	// R11 �� b16-b15

	// TODO: MODE2��b9-b7�̈��������߂Ē��ׂ�K�v������
	// ese-vdp�ł͈ȉ��̂悤�ɂ��Ă���
	// �A�g���r���[�g�e�[�u��: b9�͏������܂ꂽ�l�����̂܂܎g���Ab8-b7��0�Ƃ��Ĉ���
	// �J���[�e�[�u��		: b9�̓A�g���r���[�g�e�[�u���̔��]�Ab8-b7��0�Ƃ��Ĉ���	
	uint32_t addr = ((vdp->_r11 << 15) | (vdp->_r05 << 7)) & 0x1fe00;
	if (vdp->sprattrtbl_baddr != addr) {
		// �X�V���ꂽ��A�S�ẴX�v���C�g�̃A�g���r���[�g���Đ�������
		vdp->sprattrtbl_baddr = addr;
		vdp->sprcolrtbl_baddr = addr ^ 0x200;
		ms_vdp_update_sprite_area(vdp);
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_CC;
	}
}

void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp) {
	// R06 �� b16-b11
	uint32_t addr = (vdp->_r06 << 11) & 0x1ffff;
	if( vdp->sprpgentbl_baddr != addr ) {
		// �X�V���ꂽ��A�S�ẴX�v���C�g�̃p�^�[�����Đ�������
		vdp->sprpgentbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
	}
}

void update_sprpgentbl_baddr_MODE2(ms_vdp_t* vdp) {
	// R06 �� b16-b11
	uint32_t addr = (vdp->_r06 << 11) & 0x1ffff;
	if( vdp->sprpgentbl_baddr != addr ) {
		// �X�V���ꂽ��A�S�ẴX�v���C�g�̃p�^�[�����Đ�������
		vdp->sprpgentbl_baddr = addr;
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
		ms_vdp_update_sprite_area(vdp);
	}
}

/**
 * @brief VDP���W�X�^ R#7 �̐F�ݒ�
 * 
 * @param vdp 
 */
void update_r7_color_DEFAULT(ms_vdp_t* vdp, uint8_t data) {
	vdp->text_color = data >> 4;
	vdp->back_color = data & 0x0f;

	X68_GR_PAL[0] = vdp->palette[vdp->back_color & 0xf];
}

char* get_mode_name_DEFAULT(ms_vdp_t* vdp) {
	return "DEFAULT";
}

void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd) {
	MS_LOG(MS_LOG_INFO,"%s��VDP�R�}���h0x%02x�͂܂��������ł��B\n", vdp->ms_vdp_current_mode->get_mode_name(vdp), cmd);
	// �O�̈�CE�r�b�g���N���A
	vdp->s02 &= 0xfe;
}

void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd) {
}

uint8_t vdp_command_read_NONE(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write_NONE(ms_vdp_t* vdp, uint8_t cmd) {
}

void update_resolution_DEFAULT(ms_vdp_t* vdp) {
	ms_vdp_update_resolution_COMMON(vdp, 1, 0, 0); // 512, 16�F, BG�s�g�p
}

void vsync_draw_NONE(ms_vdp_t* vdp) {
}