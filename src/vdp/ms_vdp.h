#ifndef MS_VDP_H
#define MS_VDP_H

#include <stdint.h>
#include "../ms.h"

// extern uint16_t * const X68_GR_PAL;
// extern uint16_t * const X68_TX_PAL;
// extern uint16_t * const X68_SP_PAL_B0;
// extern uint16_t * const X68_SP_PAL_B1;
// extern uint16_t * const X68_SP_PAL_B2;

// uint16_t * const X68_GR_PAL = (uint16_t *)0xE82000;
// uint16_t * const X68_TX_PAL = (uint16_t *)0xE82200;
// uint16_t * const X68_SP_PAL_B0 = (uint16_t *)0xE82200;	// �u���b�N0�̓e�L�X�g�Ƌ��p
// uint16_t * const X68_SP_PAL_B1 = (uint16_t *)0xE82220;	// �u���b�N1�̓X�v���C�g�p���b�g�Ɏg�p
// uint16_t * const X68_SP_PAL_B2 = (uint16_t *)0xE82240;	// �u���b�N2�ȍ~�͎g�p���Ă��Ȃ�

#define X68_GRAM	((uint16_t *)0xc00000)
#define X68_GRAM_LEN	0x100000	// ���[�h��
#define X68_GR_PAL	((uint16_t *)0xE82000)
#define X68_TX_PAL	((uint16_t *)0xE82200)
#define X68_SP_PAL_B0	((uint16_t *)0xE82200)	// �u���b�N0�̓e�L�X�g�Ƌ��p
#define X68_SP_PAL_B1	((uint16_t *)0xE82220)	// �u���b�N1�̓X�v���C�g�p���b�g�Ɏg�p
#define X68_SP_PAL_B2	((uint16_t *)0xE82240)	// �u���b�N2�ȍ~�͎g�p���Ă��Ȃ�

#define CRTR		((volatile uint16_t *)0xe80000)		// CRTC���W�X�^
#define CRTR_00		(*(volatile uint16_t *)0xe80000)	// CRTC���W�X�^0
#define CRTR_01		(*(volatile uint16_t *)0xe80002)	// CRTC���W�X�^1
#define CRTR_02		(*(volatile uint16_t *)0xe80004)	// CRTC���W�X�^2
#define CRTR_03		(*(volatile uint16_t *)0xe80006)	// CRTC���W�X�^3
#define CRTR_04		(*(volatile uint16_t *)0xe80008)	// CRTC���W�X�^4
#define CRTR_05		(*(volatile uint16_t *)0xe8000a)	// CRTC���W�X�^5
#define CRTR_06		(*(volatile uint16_t *)0xe8000c)	// CRTC���W�X�^6
#define CRTR_07		(*(volatile uint16_t *)0xe8000e)	// CRTC���W�X�^7
#define CRTR_08		(*(volatile uint16_t *)0xe80010)	// CRTC���W�X�^8
#define CRTR_09		(*(volatile uint16_t *)0xe80012)	// CRTC���W�X�^9
#define CRTR_10		(*(volatile uint16_t *)0xe80014)	// CRTC���W�X�^10 (�e�L�X�g�X�N���[��X)
#define CRTR_11		(*(volatile uint16_t *)0xe80016)	// CRTC���W�X�^11 (�e�L�X�g�X�N���[��Y)
#define CRTR_SCR_p	((volatile uint16_t *)0xe80018)		// CRTC�X�N���[�����W�X�^�ւ̃|�C���^
#define CRTR_12		(*(volatile uint16_t *)0xe80018)	// CRTC���W�X�^12 (GR0�̃X�N���[��X�j
#define CRTR_13		(*(volatile uint16_t *)0xe8001a)	// CRTC���W�X�^13 (GR0�̃X�N���[��Y)
#define CRTR_14		(*(volatile uint16_t *)0xe8001c)	// CRTC���W�X�^14 (GR1�̃X�N���[��X�j
#define CRTR_15		(*(volatile uint16_t *)0xe8001e)	// CRTC���W�X�^15 (GR1�̃X�N���[��Y)

#define CRTR_20		(*(volatile uint16_t *)0xe80028)	// CRTC���W�X�^20
#define CRTR_21		(*(volatile uint16_t *)0xe8002a)	// CRTC���W�X�^21
#define CRTR_23		(*(volatile uint16_t *)0xe8002e)	// CRTC���W�X�^23

#define VCRR_00		(*(volatile uint16_t *)0xe82400)	// �r�f�I�R���g���[�����W�X�^0
#define VCRR_01		(*(volatile uint16_t *)0xe82500)	// �r�f�I�R���g���[�����W�X�^1
#define VCRR_02		(*(volatile uint16_t *)0xe82600)	// �r�f�I�R���g���[�����W�X�^2

#define SPCON_BGCON	(*(volatile uint16_t *)0xeb0808)	// BG�R���g���[�����W�X�^
#define SPCON_HTOTAL (*(volatile uint16_t *)0xeb080a)	// �����g�[�^�����W�X�^
#define SPCON_HDISP	(*(volatile uint16_t *)0xeb080c)	// �����𑜓x�ݒ背�W�X�^
#define SPCON_VSISP	(*(volatile uint16_t *)0xeb080e)	// �����𑜓x�ݒ背�W�X�^
#define SPCON_RES	(*(volatile uint16_t *)0xeb0810)	// �X�v���C�g�𑜓x�ݒ背�W�X�^

#define X68_SSR		((volatile uint16_t *)0xeb0000)	// �X�v���C�g�X�N���[�����W�X�^
#define X68_PCG		((volatile uint32_t *)0xeb8000)	// �X�v���C�g�p�^�[��

#define CRT_MODE_GRAPHIC1	0x00
#define CRT_MODE_TEXT1		0x01
#define CRT_MODE_MULTICOLOR	0x02
#define CRT_MODE_GRAPHIC2	0x04
#define CRT_MODE_GRAPHIC3	0x08
#define CRT_MODE_TEXT2		0x09
#define CRT_MODE_GRAPHIC4	0x0c
#define CRT_MODE_GRAPHIC5	0x10
#define CRT_MODE_GRAPHIC6	0x14
#define CRT_MODE_GRAPHIC7	0x1c

extern uint32_t* ms_vdp_rewrite_flag_tbl;

typedef struct ms_vdp_mode ms_vdp_mode_t;

/**
 * @brief VDP�N���X�̒�`
 * 
 */
typedef struct ms_vdp {
	uint8_t	r00;	// Mode register 0
	uint8_t	r01;	// Mode register 1
	uint8_t	_r02;	// R02�̐��̒l�B���ۂ̃A�N�Z�X�ł� pnametbl_baddr ���g��
	uint8_t _r03;	// R03�̐��̒l�B���ۂ̃A�N�Z�X�ł� R10 �ƌ������� colortbl_baddr ���g��
	uint8_t _r04;	// R04�̐��̒l�B���ۂ̃A�N�Z�X�ł� pgentbl_baddr ���g��
	uint8_t _r05;	// R05�̐��̒l�B���ۂ̃A�N�Z�X�ł� R11 �ƌ������� sprattrtbl_baddr ���g��
	uint8_t _r06;	// R06�̐��̒l�B���ۂ̃A�N�Z�X�ł� sprpgentbl_baddr ���g��
	uint8_t _r07;	// R07�̐��̒l�B���ۂ̃A�N�Z�X�ł� text_color / back_color ���g��
	uint8_t r08;	// Mode register 2
	uint8_t r09;	// Mode register 3
	uint8_t _r10;	// R10�̐��̒l�B���ۂ̃A�N�Z�X�ł� R03 �ƌ������� colortbl_baddr ���g��
	uint8_t _r11;	// R11�̐��̒l�B���ۂ̃A�N�Z�X�ł� R05 �ƌ������� sprattrtbl_baddr ���g��
	uint8_t r12;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t r16;
	uint8_t r17;
	uint8_t r18;
	uint8_t r19;
	uint8_t r20;
	uint8_t r21;
	uint8_t r22;
	uint8_t r23;
	uint8_t r24;
	uint8_t r25;
	uint8_t r26;
	uint8_t r27;
	uint8_t r28;
	uint8_t r29;
	uint8_t r30;
	uint8_t r31;
	uint16_t sx;	// 32,33 (���т��t�ɂȂ�̂Œ���)
	uint16_t sy;	// 34,35
	uint16_t dx;	// 36,37
	uint16_t dy;	// 38,39
	uint16_t nx;	// 40,41
	uint16_t ny;	// 42,43
	uint8_t clr;	// 44
	uint8_t arg;	// 45
	uint8_t r46;

	uint8_t dummy1;	// 47

	// Status Registers
	uint8_t s00;	// offset = +48
	uint8_t s01;
	uint8_t s02;
	uint8_t s03;
	uint8_t s04;
	uint8_t s05;
	uint8_t s06;
	uint8_t s07;
	uint8_t s08;
	uint8_t s09;	// offset = +57

	// Palette Registers
	uint16_t palette[16];	// offset = +58
	uint8_t alt_color_zero; // �J���[�R�[�h0�̐F�ɍł��߂��F�����p���b�g�ԍ��B�X�v���C�g���J���[�R�[�h0�ŕ`��ł�����ւ̑Ώ�
	uint8_t dummy2;

	// Special Control Registers
	// 	����R���g���[�����W�X�^
	//  R_00:	.dc.b	0		* [ 0 ][ DG][IE2][IE1][ M5][ M4][ M3][ 0 ]
	//	R_01:	.dc.b	0		* [ 0 ][ BL][IE0][ M1][ M2][ 0 ][ SI][MAG]
	uint8_t crt_mode;		// R_0,R_1��[ M5]�`[ M0]
	uint8_t sprite_size;	// R_1��[ SI]
	uint8_t sprite_zoom;	// R_1��[MAG]
	uint8_t dummy3;

	// base address registers
	uint32_t pnametbl_baddr;		// R02: Pattern name table base address
	uint32_t colortbl_baddr;		// R03: Color table base address
	uint32_t pgentbl_baddr;		// R04: Pattern generator table base address
	uint32_t sprattrtbl_baddr;		// R05: Sprite attribute table bse address
	uint32_t sprcolrtbl_baddr;		// Sprite attibute table base address - 512
	uint32_t sprpgentbl_baddr;	// R06: Sprite pattern generator table base address
	uint16_t text_color;			// R07: Text color
	uint16_t back_color;			// R07: Back color

	//
	uint32_t vram_addr;
	uint32_t gram_addr;

	//
	uint8_t disablehsyncint;
	uint8_t dummy4;

	//
	ms_vdp_mode_t *ms_vdp_current_mode;

	//
	uint16_t tx_active;
	uint16_t gr_active;
	uint16_t gr_active_interlace;

	// X68000���Ɋm�ۂ���VRAM�̐擪�A�h���X
	uint8_t* vram;

	// X68000��PCG�ɓ]�����邽�߂̃o�b�t�@�̈�
	uint32_t* x68_pcg_buffer;
	int last_visible_sprite_planes;
	int last_visible_sprite_size;

	// �X�v���C�g���t���b�V���v���t���O
	// �ǂ̃r�b�g��1�ɂȂ��Ă��邩�ŁA�ĕ`�悷��͈͂����肵�܂��B
	// �r�b�g�ԍ������̂��̂قǌy�������ŁA��ɂȂ���v�������Ƃ��Ă��܂��B
	// ����Œ�`���Ă���͈̂ȉ��̒ʂ�ł��B
	// bit 0: �X�v���C�g�̍��W��A�\����\���݂̂̕ύX
	// bit 1: �X�v���C�g�A�g���r���[�g�e�[�u��/�J���[�e�[�u���̕ύX (�p�^�[���ԍ���F�̕ύX�݂̂ŁA�p�^�[�����̂ɕύX�͂Ȃ�)
	// bit 2: CC�r�b�g�r�b�g�}�b�v�t���O�̕ύX
	// bit 3: �p�^�[���W�F�l���[�^�e�[�u���̕ύX
	//
	// bit0���Z�b�g����ƁA�X�v���C�g��\���r�b�g�̕ύX��A�X�v���C�g��Y=208�ɂ���\���AR#23�̏c�X�N���[���ɂ��
	// �ʒu�ω��Ȃǂ��X�V���܂��B
	// �ʃX�v���C�g�� X���W�AY���W�̕ύX�͑����ɔ��f�����Ă���̂ŁA������Z�b�g���Ȃ��Ă��A
	// �X�v���C�g�̍��W�͕ω����܂��B
	// �i������������A�����AXY���W���x�����t���b�V��������悤�ɂ��邩���H�j
	//
	// bit1���Z�b�g����ƁA�S�X�v���C�g�v���[���ɑΉ�����PCG���X�V���܂��B
	// ��PCG�͎��O�Ɍv�Z����PCG�o�b�t�@���g���Ă��邽�߁AVRAM��̃p�^�[�����ύX����Ă���ꍇ�́A
	// bit3�̕����Z�b�g���Ȃ��ƒǏ]���܂���B
	// bit1�͂��Ƃ��΁AVDP�Œ��ڃX�v���C�g�A�g���r���[�g�e�[�u���������ׂ��ꂽ�ꍇ��A�X�v���C�g�A�g���r���[�g
	// �e�[�u���x�[�X�A�h���X���W�X�^�ɕύX���������ꍇ�Ɏg�p���܂��B���̃r�b�g�������Ă����ꍇ�́A
	// �L���b�V������Ă���PCG�o�b�t�@������ X68000���̃X�v���C�g�p�^�[����S�čĕ`�悵�܂����A
	// MSX���ŐF���ύX���ꂽ�����ł��AX68000���̓J���[�R�[�h�����������Ȃ��Ƃ����Ȃ����߁A
	// ���̏������K�v�ł�
	//
	// bit2���Z�b�g����ƁA�X�v���C�g���[�h2��CC�r�b�g��S�������āA�r�b�g�}�b�v�t���O�č쐬���܂��B
	// �X�v���C�g�J���[�e�[�u���x�[�X�A�h���X���ύX���ꂽ���ɃZ�b�g����K�v������܂��B
	//
	// bit3�́AVDP�Œ��ڃX�v���C�g�p�^�[���W�F�l���[�^�e�[�u���������ׂ��ꂽ�ꍇ��A�X�v���C�g�p�^�[��
	// �W�F�l���[�^�e�[�u���x�[�X�A�h���X���W�X�^�ɕύX���������ꍇ�Ɏg�p���܂��B
	// ���̃r�b�g�������Ă����ꍇ�́AMSX�̃p�^�[���W�F�l���[�^�e�[�u���̒l������PCG�o�b�t�@���č\�z������ŁA
	// X68000���̃X�v���C�g�p�^�[����S�čĕ`�悵�Ȃ����܂��B
	uint16_t sprite_refresh_flag;

	// VDP�R�}���h�̃��[�N�G���A
	// ����ȍ~�� ms_vdp_mac.has �ɒ�`����Ă���I�t�Z�b�g�ɉe�����Ȃ��̂ő������Ă�OK
	uint8_t cmd_current;
	uint8_t cmd_logiop;
	uint8_t cmd_arg;
	uint32_t cmd_vram_addr;
	uint16_t cmd_vram_addr_mod;		// �_���]�����Avram_addr��1�o�C�g���̈ʒu GRAPHIC4,6�̏ꍇ�� 0,1�AGRAPHIC5�̏ꍇ�� 0,1,2,3
	uint16_t cmd_nx_count;
	uint16_t cmd_ny_count;
	uint16_t cmd_ny_sprite_start;	// VDP�ŃX�v���C�g�̈������������ꂽ���̌��o�p
	uint16_t cmd_ny_sprite_end;		// VDP�ŃX�v���C�g�̈������������ꂽ���̌��o�p


} ms_vdp_t;

#define SPRITE_REFRESH_FLAG_COORD	0x01
#define SPRITE_REFRESH_FLAG_ATTR	0x02
#define SPRITE_REFRESH_FLAG_CC		0x04
#define SPRITE_REFRESH_FLAG_PGEN	0x08

/*
 MSX�̉�ʃ��[�h���Ƃɐ؂�ւ��鏈���Q
 
 * init
 	* X68000����CRTC�p�����[�^�ݒ�Ȃǂ��s��
 * VDP���W�X�^�̏��������ɑ΂��鏈��
 * VRAM�ǂݍ���
 * VRAM��������
	* VDP���W�X�^�̒l�ƃA�h���X���r���A�ȉ��𕪊�
		* ��ʏ��������n
		* �X�v���C�g�̏�������
 * VDP�R�}���h�̎��s

 */
typedef struct ms_vdp_mode {
	int (*init)(ms_vdp_t* vdp);
	uint8_t (*read_vram)(ms_vdp_t* vdp);
	void (*write_vram)(ms_vdp_t* vdp, uint8_t data);
	void (*update_palette)(ms_vdp_t* vdp);
	void (*update_pnametbl_baddr)(ms_vdp_t* vdp);
	void (*update_colortbl_baddr)(ms_vdp_t* vdp);
	void (*update_pgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprattrtbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprpgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_r7_color)(ms_vdp_t* vdp, uint8_t data);
	char* (*get_mode_name)(ms_vdp_t* vdp);
	void (*vdp_command_exec)(ms_vdp_t* vdp, uint8_t cmd);
	uint8_t (*vdp_command_read)(ms_vdp_t* vdp);
	void (*vdp_command_write)(ms_vdp_t* vdp, uint8_t data);
	void (*update_resolution)(ms_vdp_t* vdp);
	void (*vsync_draw)(ms_vdp_t* vdp);
	int sprite_mode; // 0x00: ���g�p, 0x01: MODE1, 0x02: MODE2, bit7: 0=256�h�b�g, 1=512�h�b�g
	int crt_width; // MSX�̉�ʕ�
	int dots_per_byte; // 1�o�C�g������̃h�b�g�� (VDP�R�}���h�p)
	int bits_per_dot; // 1�h�b�g������̃r�b�g�� (VDP�R�}���h�p)
} ms_vdp_mode_t;

// singleton instance
ms_vdp_t* ms_vdp_shared_instance();
void ms_vdp_shared_deinit();

void ms_vdp_set_mode(ms_vdp_t* vdp, int mode);
void ms_vdp_vsync_draw(ms_vdp_t* vdp, int hostdebugmode);

void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern, int32_t old_pattern);
void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color, int32_t old_color);

void update_sprattrtbl_baddr_MODE1(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_MODE2(ms_vdp_t* vdp);

void update_sprpgentbl_baddr_MODE1(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_MODE2(ms_vdp_t* vdp);

void vsync_draw_NONE(ms_vdp_t* vdp);

void vdp_command_exec_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
void vdp_command_exec_NONE(ms_vdp_t* vdp, uint8_t cmd);
void vdp_command_exec(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_NONE(ms_vdp_t* vdp);
uint8_t vdp_command_read(ms_vdp_t* vdp);
void vdp_command_write_NONE(ms_vdp_t* vdp, uint8_t data);
void vdp_command_write(ms_vdp_t* vdp, uint8_t data);

/**
 * 
 * @param vdp 
 * @param res 0=256�h�b�g, 1=512�h�b�g
 * @param color 0=16�F, 1=256�F, 3=65536�F
 * @param bg 0=��\��, 1=�\��
 */
void ms_vdp_update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg);
void ms_vdp_update_visibility(ms_vdp_t* vdp);
void ms_vdp_update_sprite_area(ms_vdp_t* vdp);
void ms_vdp_sprite_vsync_draw(ms_vdp_t* vdp, int hostdebugmode);

#endif
