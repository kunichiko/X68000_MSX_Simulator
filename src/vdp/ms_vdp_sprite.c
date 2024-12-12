#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

/*
 	�X�v���C�g�̏���

	MSX�̃X�v���C�g�ɂ́A
	* 8x8�h�b�g
	* 16x16�h�b�g
	* 8x8�h�b�g(�g��)
	* 16x16�h�b�g(�g��)
	��4�̃T�C�Y������܂��B

	MSX�ɂ�256�h�b�g���[�h��512�h�b�g���[�h������܂����A�X�v���C�g�̌����ڂ̃T�C�Y�͕ς�炸�A
	��� 256�h�b�g���[�h�ŕ\������܂��B

	����AMS.X�́A�Č�����X�N���[�����[�h�ɂ���� 256�h�b�g���[�h�� 512�h�b�g���[�h��
	2�̃O���t�B�b�N��ʂ��g�������Ă��܂����AX68000�̃X�v���C�g�̃h�b�g�T�C�Y��
	�O���t�B�b�N��ʂƓ����ł��邽�߁A���̑g�ݍ��킹�����܂���������K�v������܂��B

	�g�ݍ��킹��񋓂���ƁA�ȉ��̂悤�ɂȂ�܂��B

	* MSX 8x8�h�b�g�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16��1/4���g�p)�ɑΉ�
	* MSX 8x8�h�b�g�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 16x16�h�b�g�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 16x16�h�b�g�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 8x8�h�b�g(�g��)�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 8x8�h�b�g(�g��)�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 16x16�h�b�g(�g��)�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 16x16�h�b�g(�g��)�X�v���C�g & X68000 512�h�b�g���[�h
		* (�Ή����Ȃ�)

	���̂悤�ɂ��Ă݂�ƁA�Ō�̃p�^�[�����T�|�[�g���Ȃ���΁AMSX��
	�X�v���C�g�v���[��1�͍ő��X68000��4�̃X�v���C�g�ɑΉ����邱�ƂɂȂ�܂��B
	MSX�͂ǂ̃T�C�Y�ł�32���̃X�v���C�g�v���[����\���ł��AX68000��16x16�̃X�v���C�g��
	128���\���ł���̂ŁA���傤�ǂ҂�������܂��Ă��܂��B
	�t�ɂ����Ɓu16x16�h�b�g(�g��)�X�v���C�g & X68000 512�h�b�g���[�h�v�́A
	X68000�̃X�v���C�g��������Ȃ����߁A�Ή��ł��܂���B�X�v���C�g�_�u�����g����
	�s�\�ł͂Ȃ���������܂��񂪁A���v������΁c�c�Ƃ��������ł��΂炭�̓T�|�[�g���܂���B

	�Ȃ��A���̂悤�ɉ�ʃ��[�h�ƃX�v���C�g�̃��[�h�ɂ���đΉ�������������Ă����A
	���񂪂炪��₷�����߁A�ȉ��̂悤�ɗp����`���܂��B

	* �V���O�����[�h / �}���`���[�h
		* 1�X�v���C�g�v���[����1�̃X�v���C�g�ɂȂ�P�[�X���V���O�����[�h�ƌĂ�
		* 1�X�v���C�g�v���[���������̃X�v���C�g�ɂȂ�P�[�X���}���`���[�h�ƌĂ�
	* D1X���[�h / D2X���[�h / D4X���[�h
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��1�h�b�g�ɂȂ�ꍇ�� D1X���[�h�ƌĂ�
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��2�h�b�g�ɂȂ�ꍇ�� D2X���[�h�ƌĂ�
			* �g��@�\��2�h�b�g�ɂȂ�ꍇ������Ɋ܂�
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��4�h�b�g�ɂȂ�ꍇ�� D4X���[�h�ƌĂ�
			* 512�h�b�g���[�h�Ŋg��X�v���C�g���g���ꍇ4�h�b�g�K�v�ɂȂ�



	MS.X�ł́A�ǂ̉�ʃ��[�h�ɂ����Ă��AMSX�̃X�v���C�g�v���[���ԍ���
	�ȉ��̂悤�Ƀ}�b�s���O���邱�Ƃɂ��Ă��܂��B

	MSX�̃X�v���C�g�v���[��  0�� -> X68000�̃X�v���C�g 0-3��
	MSX�̃X�v���C�g�v���[��  1�� -> X68000�̃X�v���C�g 4-7��
		:
	MSX�̃X�v���C�g�v���[�� 31�� -> X68000�̃X�v���C�g 124-127��

	���̂悤�ɁA1�̃X�v���C�g�ŊԂɍ����ꍇ(�V���O�����[�h)�ł��ԍ����l�߂��ɁA
	4��΂��Ŏg�p���܂��B�������Ă����ƁA��ʃ��[�h���s�����������ł��Ή��֌W��
	��v����̂ŁA�Ǘ����y�ɂȂ�̂ł͂Ǝv���Ă��܂��B

	��L�̓X�v���C�g�v���[���̘b�ł����A�X�v���C�g�p�^�[����`�̂ق��́AMSX�̃X�v���C�g��
	�ő�4�{�̒�`��PCG��ɓW�J���邱�Ƃ��ł��Ȃ����߁A���������PCG�p�^�[�������炩����
	�W�J���Ă���(PCG�o�b�t�@)�A�X�v���C�g�v���[���̕\�����ɓ]������悤�ɂ��Ă��܂��B

	���̎��APCG�o�b�t�@�ɂ́AMSX�̃X�v���C�g��`(8x8�P��)�̏��Ԓʂ�ɋl�߂Ċi�[���Ă����܂��B
	�����A���Ԓʂ�ƌ����Ă��AD2X���[�h��D4X���[�h�̏ꍇ�́AMSX��1�X�v���C�g��`���A
	�������ɖc���P�[�X������̂ŁA�������Ȃ��悤�ɂ��Ă��������B

	���X�v���C�g�p�^�[����`(PCG)�ɂ���

	��ʂ�256�h�b�g���[�h�A�X�v���C�g��16�h�b�g���[�h�A�g��Ȃ��̏ꍇ�Ƃ���ȊO�̏ꍇ��
	�X�v���C�g�p�^�[����`�̈�����ς��Ă��܂��B

	�� �u��ʕ�256�A�X�v���C�g16x16�A�g��Ȃ��v�̏ꍇ
	���̏ꍇ�AMSX�̃X�v���C�g��`�� 16x16�h�b�g�̒�`��64���܂��BX68000��PCG��
	16x16�h�b�g�̃X�v���C�g��128��`�ł���(BG���g��Ȃ���΂����Ƃ���)�̂ŁA
	MSX�̃X�v���C�g��`�����̂܂�PCG�̒�`�Ƀ}�b�s���O�ł��܂��B
	512�h�b�g���[�h��g�僂�[�h�ɂȂ��4�{�ɖc��Ă��܂��A���܂�Ȃ��Ȃ�܂��B�܂��A
	MSX��8x8�h�b�g���[�h�̏ꍇ�͒�`����256�ɂȂ�܂����AX68000��16x16�̃Z�b�g��
	������`�ł��Ȃ��̂ŁA��͂��`����4�{�ɖc��Ă��܂��A���܂�Ȃ��Ȃ�܂��B
	�ȑO�͂ǂ̃��[�h�ł����������i��q����u����ȊO�̏ꍇ�v�̂�肩���j�ł�����������
	���܂������A�u��ʕ�256�A�X�v���C�g16x16�A�g��Ȃ��v�̓Q�[���ōł��悭�g����
	���[�h�Ȃ̂ŁA���̃P�[�X�������ʈ������č��������邱�Ƃɂ��܂����B

	��̓I�ɂ́AMSX����64�̃X�v���C�g��`�ɐF���|�����킹������(�F������̃p�^�[��)��
	X68000��PCG��ɍő�32�W�J���܂��B���ꂼ��̃p�^�[�����AMSX�̂ǂ̃p�^�[���ԍ���
	�ǂ̐F���|�����킹�����̂���ʓr�L�^���Ă����܂��B���̂悤�ɂ��邱�ƂŁA�X�v���C�g
	�\�����ɂ��łɐF�����ς݂̂��̂�����΂�������̂܂܎g�����Ƃ��ł��邽�߁A������
	�ł��܂��B
	�X�v���C�g���[�h2���ƕ������̃p�^�[����g�ݍ��킹���F�������\�Ȃ̂ŁA�Ǘ��e�[�u��
	�͈ȉ��̂悤��32bit�̏���32���悤�ɂ��܂��B
    
	31-30 29-20�@19-10  9-0
	[ N ][3����][2����][1����]
	  N: ���������p�^�[���̐�.0�̎��͂��̃o�b�t�@�͖��g�p�ł��邱�Ƃ������܂�
	  1-3����: 10bit�̃f�[�^�ŁA���������p�^�[���̔ԍ��ƐF�̑g�ݍ��킹��\���܂�
	     9-6    5-0
        [Col][Pattern]
	
	���Ƃ���Mode1�̏ꍇ�́A�ꖇ�̍����������蓾�Ȃ��̂ŁA�ȉ��̂悤�ɂȂ�܂��B

	- 0b01_0000_000000_0000_000000_1111_000000
		�p�^�[���ԍ�0�ԂɐF�R�[�h0xf���|�����킹������

	Mode2�̏ꍇ�́A�ő�3���܂ł̐F�������Ǘ��ł��邽�߁A�ȉ��̂悤�ɂȂ�܂��B

	- 0b11_0100_000010_0010_000001_0001_000000
		�p�^�[���ԍ�0�ԂɐF�R�[�h0x1���|�����킹������
		�p�^�[���ԍ�1�ԂɐF�R�[�h0x2���|�����킹������
		�p�^�[���ԍ�2�ԂɐF�R�[�h0x4���|�����킹������
		�ȏ�3��������������

	Mode2�̓��C�����ƂɐF��ύX�ł���̂ł����A��\�F�Ƃ���8���C���ڂ̐F���g�����Ƃ�
	���܂��B���܂���8���C���ڂ̐F�������ő��̃��C�������F���قȂ�P�[�X�ɑΉ��ł��Ȃ�
	���Ƃ����蓾�܂����A���߂܂��B

	�g�ݍ��킹�̃o���G�[�V�����͔��ɑ����ł����A��������MSX�̓X�v���C�g�v���[����
	32�����Ȃ��̂ŁA�����ɑ��݂ł���g�ݍ��킹�͍��X32�ʂ�ɂȂ�܂��B
	PCG��127��`�ł���̂ő��߂Ɏ����Ƃ��\�ł����A���łɐF�����ς݂̂��̂����邩
	�ǂ����𒲂ׂ�ۂɃ��j�A�T�[�`���s�����߁A���܂葽������������Ƃ������Ēx���Ȃ�
	�\��������A32���x���K�؂��Ǝv���܂��B

	�ȏ�𓥂܂��A�����A�����Ԃ��I�����������̃^�C�~���O�ŁA�ȉ��̏��������{���܂��B
	0 : �p�^�[��������������ꂽ���̂�dirty�t���O�𗧂ĂĂ����܂�
	1 : �p�^�[���Ǘ��e�[�u�����X�L�������Adirty�ȃp�^�[�����g���Ă������̂�����΁A
		0�N���A���܂�
	2: 	�X�v���C�g�A�g���r���[�g�e�[�u����0���珇�ɃX�L�������܂�
		2-1: �X�v���C�g����\���ł���΃X�L�b�v���܂�
		2-2: �ȍ~�̃v���[������\���̏ꍇ�͂����Œ�~���A�c����\���ɂ��܂�
		2-3: �A������v���[�����������ACC=1�̂��̂��ő�2�܂Ō����܂�
			3�ȏ゠�����ꍇ�͖������܂�
		2-4: CC=1�̑g�ݍ��킹�ɉ����A32bit�̊Ǘ��t���O���v�Z���܂�
		2-5: �p�^�[���Ǘ��e�[�u����擪����X�L�������A�����l�������Ă�����̂�
			����΁A���̃p�^�[�����g�p���ăX�v���C�g��\�����܂��B�܂��A�p�^�[����
			�g�p�������Ƃ�32bit�̎g�p�ς݃t���O�Ŋo���Ă����܂�
		2-6: �p�^�[�������݂��Ȃ��ꍇ�́A���̏�ŐF�������A���ʂ���������ɓW�J���āA
			��̏����ɔ����܂�
		2-7: �����ς݃p�^�[�����g���ĕ\���ł��Ȃ��������̂�PCG��ɓW�J���܂�
			���̂Ƃ��A�g�p�ς݃t���O���݂Ȃ���A�g���Ȃ������p�^�[���𓪂���
			�㏑�����Ă����܂�

	���̂悤�ɂ��邱�ƂŁA���łɍ����ς݂̃p�^�[�����ė��p���Ȃ���A�����悭�X�v���C�g��
	�\�����邱�Ƃ��ł��܂��B


	�� ����ȊO�̏ꍇ
	X68000�̃X�v���C�g�ԍ� N�Ԃ� ��PCG��N�ł�Ή��Â���悤�ɂ��܂��B���̑Ή�
	�Â��͑S�Ẳ�ʃ��[�h�ŕς��Ȃ����߁A4��΂��Ŏg���邱�Ƃ�����܂��B

	����ƁAMSX��8x8���[�h�̎��́APCG�o�b�t�@��ɂ͘A�����ăp�^�[�����i�[����Ă��܂����A
	X68000�̎�PCG�ɓW�J����ۂ́A16��΂��œW�J���邱�ƂɂȂ�ȂǁA�Ή�����Έ��
	�Ȃ�Ȃ��P�[�X������̂Œ��ӂ��Ă��������B
	(PCG�o�b�t�@�͂����܂�PCG�����Ƃ��̑f�ނ��l�܂��Ă���Ǝv���Ă�������)
 */

/*
 	��������

  # �u�F�R�[�h0�v�̃X�v���C�g
  MSX�̓p�^�[���̒�`�ƐF�̒�`���ʂ�Ă��邽�߁A�u�p�^�[����1�����A�F�R�[�h��0�v��
  �X�v���C�g�Ƃ������̂��`��\�ł��B����AX68000�̓p�^�[���ƐF����̂ƂȂ��Ă��邽�߁A
  �F�R�[�h0�͓����F�Ƃ��Ĉ����܂��B���̂��߁AMS.X�ł́u�p�^�[����1�����A�F�R�[�h��0�v
  �̃X�v���C�g�͋����I�ɐF�R�[�h1�ɕϊ�����܂��B
  ���̂��߁A�F���ς���Č����Ă��܂����Ƃ�����܂��B

  # �X�v���C�g���[�h2�̐F����

  MSX�̃X�v���C�g���[�h2�́A�X�v���C�g�̐F�������s���郂�[�h�ł����AMS.X�ł�
  ����̏������ł����������\������܂���B��̓I�ɂ́A�ȉ��̏����𖞂����ꍇ�̂�
  �������\������܂��B

  * �d�Ȃ����X�v���C�g��XY���W���҂������v���Ă��邱��
  * �������s���X�v���C�g�v���[���ԍ����A�����Ă��邱��

 
 */

#define COL_SIZE			16				// MSX�̃X�v���C�g�J���[�e�[�u���̃T�C�Y(�o�C�g��)
#define SAT_SIZE			4				// MSX�̃X�v���C�g�A�g���r���[�g�e�[�u���̃T�C�Y(�o�C�g��)
#define PCG_SIZE			32				// X68k�̃X�v���C�g1��(16x16)������́APCG�p�^�[���̃��[�h��
#define SSR_SIZE			4				// X68k�̃X�v���C�g1������́ASSR���W�X�^�̃��[�h��
#define PCG_UNIT			(PCG_SIZE * 4)	// MSX�̃v���[��1������́APCG�̃��[�h��
#define SSR_UNIT			(SSR_SIZE * 4)	// MSX�̃v���[��1������́ASSR�̃��[�h��
#define PCG_BUF_UNIT_D1X	8				// D1X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��
#define PCG_BUF_UNIT_D2X	32				// D2X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��
#define PCG_BUF_UNIT_D4X	128				// D4X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��


// ��
uint16_t sprite_cc_flags[32]; // �X�v���C�g�J���[�e�[�u����CC�r�b�g�̃t���O

uint8_t sp_pattern_dirty[64];	// 16x16�h�b�g�̃p�^�[����64����


#define NUM_SP_PAT_BUF 64

uint32_t sp_composition_table[NUM_SP_PAT_BUF];

int sp_new_pattern_count; 
struct sp_pat_buf {
	int32_t plane_num;
	uint32_t composition_flag;
	uint32_t pattern[PCG_SIZE];
} sp_pat_buf[NUM_SP_PAT_BUF];

int lastused_sp_pat_buf = 0;

/*
 �X�v���C�g�̏�����
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCG�o�b�t�@�̏�����
	for ( i = 0; i < 256 * PCG_BUF_UNIT_D4X; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;

	// ����������
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_FULL;
	vdp->sprite_composition_flag = 0;

	for (i = 0; i < 64; i++) {
		sp_pattern_dirty[i] = 0;
	}
	for (i = 0; i < 32; i++) {
		sp_composition_table[i] = 0;
	}
	for (i=0; i < 32; i++) {
		sp_pat_buf[i].composition_flag = 0;
	}
	
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern);

/*
 �X�v���C�g�p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
	 offset: �p�^�[���W�F�l���[�^�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
	 pattern: �������ރp�^�[��(����8bit�̂ݎg�p)
*/
void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern, int32_t old_pattern) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_pattern_512(vdp, offset, pattern);
	} else {
		write_sprite_pattern_256(vdp, offset, pattern);
	}
	// �p�^�[���W�F�l���[�^�e�[�u�����ύX���ꂽ��A�A�g���r���[�g�e�[�u���̍X�V��������
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine; // MSX��1���C����X68000�ł�1���C��
	uint32_t pcg_pattern=0; // x68000��16x16�̃p�^�[���ɕϊ���������

	// �E�[�̃h�b�g���珈��
	for(i =0; i < 8; i++) {
		pcg_pattern >>= 4;
		if(pattern & 1) {
			pcg_pattern |= (0xf0000000);
		}
		pattern >>= 1;
	}
	// �p�^�[���W�F�l���[�^�e�[�u���̃o�b�t�@�ɏ�������
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D1X + pcgLine] = pcg_pattern;

	// �p�^�[�����ύX���ꂽ���Ƃ��L�^
	sp_pattern_dirty[ptNum / 4] = 1;	// 16x16�h�b�g�P�ʂŊǗ�
}

void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine * 2; // MSX��1���C����X68000�ł�2���C��
	uint32_t pLeft=0,pRight=0; // 1���C���̍�4�h�b�g�ƉE4�h�b�g�� X68000��8x8�̃p�^�[��2�ɕϊ�

	// �E�[�̃h�b�g���珈��
	for(i =0; i < 4; i++) {
		pRight >>= 8;
		if(pattern & 1) {
			pRight |= (0xff000000);
		}
		pattern >>= 1;
	}
	// �c��̍�4�h�b�g�̏���
	for(i =0; i < 4;i++) {
		pLeft >>= 8;
		if(pattern & 1) {
			pLeft |= (0xff000000);
		}
		pattern >>= 1;
	}
	// �p�^�[���W�F�l���[�^�e�[�u���̃o�b�t�@�ɏ�������
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void refresh_sprite_512(ms_vdp_t* vdp, int plNum);
void refresh_sprite_512_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_512_mode2(ms_vdp_t* vdp);
void refresh_sprite_256(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode2(ms_vdp_t* vdp);

inline int get_sprite_adjustx(ms_vdp_t* vdp) {
	return (vdp->r18 & 0x8) ? 8-(vdp->r18 & 0x7) : -(vdp->r18 & 0x7);
}

void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_attribute_512(vdp, offset, attribute, old_attribute);
	} else {
		write_sprite_attribute_256(vdp, offset, attribute, old_attribute);
	}
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	int i,j;
	int plNum = (((uint32_t)offset) / SAT_SIZE); // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % SAT_SIZE; // �����̎��
	int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;

	if (plNum >= 32) {
		return;
	}

	switch(type) {
		case 0: // Y���W
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
				return;
			// Y=208/216�̑Ή��̂��߁AY���W��208/216�̒l���ω�������A�X�v���C�g�̍Ĕz�u���s��
			int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
			if ( attribute == HY || old_attribute == HY) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 256���[�h��MSX��Y���W��1�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff) + 16;
			X68_SSR[plNum*SSR_UNIT+1] = y;
			break;
		case 1: // X���W
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
				return;
		 	// MSX��X���W��1�{, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			// SET ADJUST��X�����̕␳���s��
			int ec = 0;
			switch(spMode) {
			case 1:
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				break;
			case 2:
				uint8_t* pcolr = vdp->vram + vdp->sprcolrtbl_baddr;
				ec = (pcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ���C�����Ƃ�EC�̓T�|�[�g���Ȃ��̂�1���C���ڂ����݂�
				break;
			}
			int x = attribute & 0xff;
			int adjustx = get_sprite_adjustx(vdp);
			x = ((x - ec*32) + 16 + adjustx) & 0x3ff; // MSX��X���W��1�{
			X68_SSR[plNum*SSR_UNIT+0] = x;
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]�����A����������
			if(spMode == 1) {
				refresh_sprite_256_mode1(vdp, plNum);
				// EC��X���W�ɔ��f
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				int x = pattr[plNum*SAT_SIZE+1];
				int ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				int adjustx = get_sprite_adjustx(vdp);
				x = ((x - ec*32) + 16 + adjustx) & 0x3ff; // MSX��X���W��1�{
				X68_SSR[plNum*SSR_UNIT+0] = x;
			} else {
				// ���[�h2�͈����������ƑS�̂ɉe�����o��̂ŁA�����A�����Ԃɂ܂Ƃ߂ď���������
				// TODO: ����Ȃ�ɏd���̂ŁA���������͈͂����肵����
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
			}
			break;
		default:
			break;
	}
}

uint32_t colorex_tbl[16] = {
	0x00000000, 0x11111111, 0x22222222, 0x33333333,
	0x44444444, 0x55555555, 0x66666666, 0x77777777,
	0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb,
	0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff
};

void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	uint32_t* x68_pcg_buffer = vdp->x68_pcg_buffer;
	uint32_t ptNum = p[plNum*SAT_SIZE+2];
	ptNum &= (vdp->sprite_size == 0) ? 0xff : 0xfc;
	uint32_t color = p[plNum*SAT_SIZE+3] & 0xf;
	//color = color == 0 ? vdp->alt_color_zero : color; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
	// Spmode 1��coloe 0�͓����F
	// if (color == 0) {
	// 	// �F�R�[�h0�̃X�v���C�g�͕\�����Ȃ�
	// 	for( i=0; i<4; i++) {
	// 		X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// �X�v���C�g�͔�\��
	// 	}
	// 	return;
	// } else {
	// 	for( i=0; i<4; i++) {
	// 		X68_SSR[plNum*SSR_UNIT+i*4+3] = 3; // �X�v���C�g�\��
	// 	}
	// }
	//uint32_t colorex ;= color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	uint32_t colorex = colorex_tbl[color];
	if (vdp->sprite_size == 0) {
		// 8x8
		for( i = 0; i < 8; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = x68_pcg_buffer[ptNum*PCG_BUF_UNIT_D1X+i] & colorex;
		}
		for(;i < 32; i++) {
			X68_PCG[plNum*PCG_UNIT+i] = 0;
		}
	} else {
		// 16x16
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = x68_pcg_buffer[ptNum*PCG_BUF_UNIT_D1X+i] & colorex;
		}
	}
}

/*
�@�X�v���C�g���[�h2�̐F���������̎���
�@���[�h2�̐F�����̓h�b�g�P�ʂōs���邽�߁A�^�ʖڂɎ�������Ɣ��ɕ��G�ɂȂ�A���ׂ������̂ŁA
�@�ȗ������đΉ�����i�_���ȃP�[�X�������悤�Ȃ�ȍl�������j
�@�ʏ�̃Q�[���ȂǂŐF�������g���ꍇ�A�ȉ��̂悤�Ȏg���������Ă���͂��B
	* X,Y���W�͊��S�Ɉ�v������
	* �A������X�v���C�g�v���[�����g�p����
		* ���[�h2�̎g�p��́u���ꃉ�C����ɕ���ł��镨�̒��ō����v�Ȃ̂ŕK�������A�����Ă��Ȃ��Ă��ǂ����A
		* �X�v���C�g�����؂�ۂȂǂɔj�]����̂ŁA�ʏ�͘A��������͂�
	* �ő�3���̍����܂�
		* 4�r�b�g�����16�F�S�Ă��\���ł���̂�5���ȏ�d�˂�P�[�X�͂Ȃ��Ƒz��
		* ���ۂɂ�3���������8�F���g����̂ŁA�Ǘ��̓s���������A3���܂łƂ���
�@�ȏ��O��Ƃ��āA�ȉ��̂悤�ɍ�������B
	* n=0����J�n
	* �v���[��n�Ԃ̃X�v���C�g��XY���W���擾
	* �v���[��n+1�Ԃ���31�̃X�v���C�g�̂����AXY���W������̂��̂��ACC=1�̃��C������ł�������̂𒊏o(�A�����Ă��镨�̂�)
		* �A�������Ō�̔ԍ���m�Ƃ���
		* �������Ȃ��ꍇ�� m == n �ƂȂ�
	* ���C�����Ƃɍ������邽��y=0����ȉ����J��Ԃ�
	* �v���[��n�Ԃ�y���C���ڂ̃p�^�[���ɐF�R�[�h���|����
	* �v���[��n+1����m�܂ňȉ����J��Ԃ��A���ׂĂ�or�ō�������
		* y���C���ڂ̐F�f�[�^���擾���A���ꂼ��̐F�R�[�h���|����
	* y=y+1���ČJ��Ԃ�
	* �S�Ẵ��C��(8x8���[�h�̏ꍇ��8���C���A16x16���[�h�̏ꍇ��16���C��)�ɑ΂��ČJ��Ԃ�
	* n�� m+1�ɍX�V���An��31�𒴂���܂ŌJ��Ԃ�
�@���̂悤�ɂ���ƁA�ȉ��̂悤�ȗ�O�P�[�X�͏E���Ȃ����A������x�Č��ł���͂��B
	* �v���[��0�Ԃ�CC�r�b�g��1�������ꍇ�A�{���͕`�悵�Ȃ��͂��i���Ⴂ�ԍ����Ȃ��̂Łj�����A�\������Ă��܂�
	* XY�������ɂ���ďd�Ȃ��Ă���P�[�X�͑S�č�������Ȃ�
�@�Ȃ��ACC�r�b�g�̔�������������邽�߁A�ȉ��̎d�g�݂��p�ӂ���B
	* �e�v���[���� 16bit�̃r�b�g�}�b�v�t���O��p�ӂ���
	* �X�v���C�g�J���[�e�[�u�����X�V���ꂽ��ACC=1�̏ꍇ�̓��C���ԍ��ɑΉ�����r�b�g��1�ɂ��ACC=0�̏ꍇ��0�ɂ���
�@���̂悤�ɂ��Ă����ƁA����v���[���̃t���O��0�łȂ���΁A�����ꂩ�̃��C����CC=1�ɂȂ��Ă��邱�Ƃ��킩��
 */

int find_free_pcg(uint8_t* sp_pattern_used) {
	// ����V���ɍ����������̂��A�󂢂Ă���PCG�ɏ�������
	int i,j;
	int new=0;
/*	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_composition_table[i] != 0) {
			continue;
		}
		return i;
		// �󂢂Ă���Ƃ��낪���������̂Ńp�^�[����]������
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// �v���[���Ƀp�^�[�������蓖�Ă�
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}*/
	//�󂢂Ă���Ƃ��낪�Ȃ�������A����g��Ȃ������p�^�[�����N���A
	for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
		if( sp_pattern_used[i] ) {
			continue;
		}
		return i;
	}
}

void refresh_sprite_256_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;

	// �t���O�N���A
	vdp->sprite_composition_flag = 0;
	sp_new_pattern_count = 0;

	// dirty�t���O�̏���
	for (i=0;i<NUM_SP_PAT_BUF;i++) {
		uint32_t flag = sp_composition_table[i];
		if (flag == 0) {
			continue;
		}
		for(j=0;j<3;j++) {
			if ( j == (flag >> 30) ) {
				break;
			}
			int ptNum = flag & 0x3f;
			flag >>= 10;
			if (sp_pattern_dirty[ptNum]) {
				// �p�^�[��������������ꂽ�̂ŁA�����ς݂̃p�^�[�����N���A
				sp_composition_table[i] = 0;
				break;
			}
		}
	}
	// �_�[�e�B�t���O���N���A
	for (i = 0; i < 64; i++) {
		sp_pattern_dirty[i] = 0;
	}

	uint8_t sp_pattern_used[NUM_SP_PAT_BUF];
	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		sp_pattern_used[i] = 0;
	}

	uint8_t scroll_offset = vdp->r23; // �c�X�N���[����

	// �v���[�����Ƃɏ���
	for (plNum=0;plNum<32;) {
		if ( ((patr[plNum*SAT_SIZE+0] - scroll_offset) & 0xff) > 191) {
			// Y���W���͈͊O�̂��͖̂���
			plNum++;
			continue;
		}
		int m = plNum;
		uint32_t composition_flag = 0;
		uint32_t color = pcol[plNum*COL_SIZE+8] & 0xf; // 8���C���ڂ��\�F�Ƃ��Ďg�p
		if (color == 0) {
			//�F�R�[�h0�͖���
			plNum++;
			continue;
		}
		uint32_t ptnum = (patr[plNum*SAT_SIZE+2] / 4) & 0x3f; // 16x16�h�b�g���[�h�̂Ƃ���4��1�̃p�^�[��
		uint32_t flag = (color << 6) | ptnum;
		composition_flag = flag; 
		vdp->sprite_composition_flag &= ~(1 << plNum);
		for(i=1,n=plNum+1; i<3 && n<32; i++,n++) {
			// XY���W������̂��̂��ACC=1�̃��C������ł�������̂𒊏o(�A�����Ă��镨�̂�)
			if((patr[plNum*SAT_SIZE+0] == patr[n*SAT_SIZE+0]) && (patr[plNum*SAT_SIZE+1] == patr[n*SAT_SIZE+1]) && //
				(sprite_cc_flags[n] != 0)) {
				// �A���������̂����������̂ŁAm���X�V
				m = n;
				vdp->sprite_composition_flag |= 1 << m;
				color = pcol[plNum*COL_SIZE+8] & 0xf; // 8���C���ڂ��\�F�Ƃ��Ďg�p
				ptnum = patr[plNum*SAT_SIZE+2] / 4;	// 16x16�h�b�g���[�h�̂Ƃ���4��1�̃p�^�[��
				flag = ((color << 6) | (ptnum & 0x3f)) << (i*10);
				composition_flag |= flag;
			} else {
				break;
			}
		}
		composition_flag |= i << 30;	// ����������bit31-30�ɋL�^

		// ���̑g�ݍ��킹�����ł�PCG��ɑ��݂��邩�ǂ������m�F
		int found = 0;
		for(i=0;i<NUM_SP_PAT_BUF;i++) {
			if( sp_composition_table[i] == composition_flag) {
				// ���łɍ����ς݂̃p�^�[�������݂���̂ŁA������g��
				X68_SSR[plNum*SSR_UNIT+2] = 0x100 + i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�					
				X68_SSR[plNum*SSR_UNIT+3] = 3;	// �X�v���C�g�͕\��
				sp_pattern_used[i] = 1;
				found = 1;
				break;
			}
		}
		if (found) {
			plNum = m+1;
			continue;
		}

		// �����ς݂̂��̂�������Ȃ������ꍇ�́A�V���ɍ�������

		for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
			if( sp_pattern_used[i] ) {
				continue;
			}
			// �K����͋󂢂Ă���͂�
			break;
		}
		// ���̃p�^�[���ԍ��ɓW�J����
		int pat_num = i;
		lastused_sp_pat_buf = i;
		sp_pattern_used[pat_num] = 1;
		sp_composition_table[pat_num] = composition_flag;

		// sp_pat_buf[sp_new_pattern_count].plane_num = plNum;
		// sp_pat_buf[sp_new_pattern_count].composition_flag = composition_flag;

		// ���C�����Ƃ̍�������
		// 256���[�h�̏ꍇ�A16x16�̃X�v���C�g�����̂܂�16x16�̃X�v���C�g�ɂȂ�̂ŁA
		// �ȉ��̂悤�ɍ�������
		// ��̔��� X68000�� 8x8�̃p�^�[��(1���C����32bit)��\��
		// X68000�̓X�v���C�g�T�C�Y��16x16�����A��`��8x8�̔���4�A
		// ���と�������E�と�E���̏��ɏW�܂��Ă���̂Œ��ӂ��K�v
		//
		//  lr: 0   1  
		//    +---+---+	y=0		CC��0���C����
		//    | 0 | 2 |
		//  �@+---+---+	y=8		CC��8���C����
		//    | 1 | 3 |
		//    +---+---+	y=15	CC��15���C����
		// �� 8x8�̃X�v���C�g�̏ꍇ�͍����0�̕����������g����
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		int lrmax = vdp->sprite_size == 0 ? 1 : 2;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		uint16_t mask = 1;
		for	(y=0; y<ymax; y++, mask <<= 1) {
			// yybase �͏�L�p�^�[���̈�ԍ��̗�(0,15) �̒��Ō����AX68000���̃��C���ԍ�
			int yybase = y;
			i=plNum;
			while(i<=m) {
				uint32_t color = pcol[i*COL_SIZE+y] & 0xf;
				color = color == 0 ? vdp->alt_color_zero : color; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
				uint32_t colorex = colorex_tbl[color];
				uint32_t ptNum = patr[i*SAT_SIZE+2];
				uint32_t pattern0 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D1X+yybase+8*0 ] & colorex;	// lr=0
				uint32_t pattern1 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D1X+yybase+8*2 ] & colorex;	// lr=1

				j=i;
				while(j<=m) {
					if( j == m ) {
						// 1���C�������I��
//						sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*0] = pattern0;
//						sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*2] = (lrmax==2) ? pattern1 : 0;
						X68_PCG[pat_num*PCG_SIZE+yybase+8*0] = pattern0;
						X68_PCG[pat_num*PCG_SIZE+yybase+8*2] = (lrmax==2) ? pattern1 : 0;
						i=j+1;
						break;
					}
					j++; // j==m���ɔ��肵�Ă���̂ŁAj+1��m���I�[�o�[���邱�Ƃ͂Ȃ�
					// CC=1�̂��̂���������
					uint32_t color_add = pcol[j*COL_SIZE+y] & 0xf;
					color_add = color_add == 0 ? vdp->alt_color_zero : color_add; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
					//uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
					uint32_t colorex_add = colorex_tbl[color_add];
					uint32_t ptNum_add = patr[j*SAT_SIZE+2];
					uint32_t pattern_add0 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D1X+yybase+8*0 ] & colorex_add;
					uint32_t pattern_add1 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D1X+yybase+8*2 ] & colorex_add;
					pattern0 |= pattern_add0;
					pattern1 |= pattern_add1;
				}
			}
		}
		// 8x8�̎��͎c���0�Ŗ��߂�
		for	(; y<16; y++) {
			int yybase = y;
			i=plNum;
			//sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*0] = 0;
			//sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*2] = 0;
			X68_PCG[pat_num*PCG_SIZE+yybase+8*0] = 0;
			X68_PCG[pat_num*PCG_SIZE+yybase+8*2] = 0;
		}


		//sp_new_pattern_count++;

/*		// ���̃v���[����
		X68_SSR[plNum*SSR_UNIT+3] = 3;	// �X�v���C�g�͕\��
		vdp->sprite_composition_flag &= ~(1 << plNum);
		while(plNum < m) {
			plNum++;
			X68_SSR[plNum*SSR_UNIT+3] = 0;	// ���������X�v���C�g�͔�\��
			vdp->sprite_composition_flag |= 1 << plNum;
		}*/
		plNum = m+1;
	}
	// ����V���ɍ����������̂��A�󂢂Ă���PCG�ɏ�������
/*	int new=0;
	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_composition_table[i] != 0) {
			continue;
		}
		// �󂢂Ă���Ƃ��낪���������̂Ńp�^�[����]������
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// �v���[���Ƀp�^�[�������蓖�Ă�
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}
	//�󂢂Ă���Ƃ��낪�Ȃ�������A����g��Ȃ������p�^�[�����N���A
	for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_pattern_used[i] ) {
			continue;
		}
		// �󂢂Ă���Ƃ��낪���������̂Ńp�^�[����]������
		lastused_sp_pat_buf = i;
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// �v���[���Ƀp�^�[�������蓖�Ă�
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}*/


}

void refresh_sprite_512_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;
	// �X�v���C�g���[�h2�̐F�������s��
	for (plNum=0;plNum<32;plNum++) {
		int m = plNum;
		for(n=plNum+1;n<32;n++) {
			// XY���W������̂��̂��ACC=1�̃��C������ł�������̂𒊏o(�A�����Ă��镨�̂�)
			if((patr[plNum*SAT_SIZE+0] == patr[n*SAT_SIZE+0]) && (patr[plNum*SAT_SIZE+1] == patr[n*SAT_SIZE+1]) && //
				(sprite_cc_flags[n] != 0)) {
				m = n;
			} else {
				break;
			}
		}

		// ************
		//m = plNum; // �e�X�g�p
		// ************

		// TODO 512�h�b�g���[�h�ŃX�v���C�g���c��2�{�Ɉ����L�΂����Ƃ͂ł�����ۂ��̂ŁA���ꂪ�ł���Ə������ݗʂ������ɂȂ�@

		// ���C�����Ƃ̍�������
		// 512���[�h�̏ꍇ�A1���C����2���C���ɂȂ�̂ŁA�������C����2��`��
		// 16x16�̃X�v���C�g�̏ꍇ�A�ȉ��̂悤�ɍ�������
		// ��̔��� X68000�� 8x8�̃p�^�[��(1���C����32bit)��\��
		// X68000�̓X�v���C�g�T�C�Y��16x16�����A��`��8x8�̔���4�A
		// ���と�������E�と�E���̏��ɏW�܂��Ă���̂Œ��ӂ��K�v
		//
		//  lr: 0   1      2   3
		//    +---+---+  +---+---+	y=0		CC��0���C����
		//    | 0 | 2 |  | 8 | A |
		//  �@+---+---+  +---+---+	y=4		CC��4���C����
		//    | 1 | 3 |  | 9 | B |	
		//    +---+---+  +---+---+	y=7		CC��7���C����
		//  lr: 0   1      2   3
		//    +---+---+  +---+---+	y=8		CC��8���C����
		//    | 4 | 6 |  | C | E |
		//    +---+---+  +---+---+
		//    | 5 | 7 |  | D | F |
		//    +---+---+  +---+---+	y=15	CC��15���C����
		// �� 8x8�̃X�v���C�g�̏ꍇ�͍����0,1,2,3�̕����������g����

		// �ey�ɑ΂���X68000����2���C���ɂȂ邪�A���[�v��MSX���̃��C�����ł����Ȃ��A
		// X68000���ɓ����p�^�[����A���ŏ����悤�ɂ��Ă���B����Ă��� y�� 8 or 16��OK�B
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		// lr�� 8�h�b�g�T�C�Y�̏ꍇ��2��A16�h�b�g�T�C�Y�̏ꍇ��4��J��Ԃ�
		int lrmax = vdp->sprite_size == 0 ? 2 : 4;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		uint16_t ccmask = 1;
		for	(y=0; y<ymax; y++, ccmask <<= 1) {
			// yybase �͏�L�p�^�[���̈�ԍ��̗�(0,1,4,5) �̒��Ō����AX68000���̃��C���ԍ�
			// �����ŏo�Ă���yybase�͕K�������ɂȂ�܂��B
			int yybase = ( y & 0x8)/8*4*8 +	// �O�̏�i���i
						 ( y & 0x4)/4*1*8 +	// ���̏�i���i
						 ( y & 0x3)  *2;	// 512���[�h�̏ꍇ�A1���C����2���C���ɂȂ�
			i=plNum;
			while(i<=m) {
				uint32_t color = pcol[i*COL_SIZE+y] & 0xf;
				color = color == 0 ? vdp->alt_color_zero : color; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
				//uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
				uint32_t colorex = colorex_tbl[color];
				uint32_t ptNum = patr[i*SAT_SIZE+2];
				uint32_t pattern0 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*0 ] & colorex;	// lr=0
				uint32_t pattern1 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*2 ] & colorex;	// lr=1
				uint32_t pattern2 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*8 ] & colorex;	// lr=2
				uint32_t pattern3 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*10] & colorex;	// lr=3

				j=i;
				while(j<=m) {
					if( j == m ) {
						X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = pattern0;
						X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = pattern0;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = pattern1;
						X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = pattern1;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = (lrmax==4) ? pattern2 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = (lrmax==4) ? pattern2 : 0;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*10+0] = (lrmax==4) ? pattern3 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*10+1] = (lrmax==4) ? pattern3 : 0;	//����C�����������̂ɂȂ�(yy�͕K������)
						i=j+1;
						break;
					}
					j++; // j==m���ɔ��肵�Ă���̂ŁAi+1��m���I�[�o�[���邱�Ƃ͂Ȃ�
					if( (sprite_cc_flags[j]&ccmask) == 0) {
						// CC=0�ɑ���������A����ȍ~�͍������Ȃ�
						X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = pattern0;
						X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = pattern0;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = pattern1;
						X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = pattern1;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = (lrmax==4) ? pattern2 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = (lrmax==4) ? pattern2 : 0;	//����C�����������̂ɂȂ�(yy�͕K������)
						X68_PCG[i*PCG_UNIT+yybase+8*10+0] = (lrmax==4) ? pattern3 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*10+1] = (lrmax==4) ? pattern3 : 0;	//����C�����������̂ɂȂ�(yy�͕K������)
						i=j;
						break;
					}
					// CC=1�̂��̂����������̂ō�������
					uint32_t color_add = pcol[j*COL_SIZE+y] & 0xf;
					color_add = color_add == 0 ? vdp->alt_color_zero : color_add; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
					uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
					uint32_t ptNum_add = patr[j*SAT_SIZE+2];
					uint32_t pattern_add0 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*0 ] & colorex_add;
					uint32_t pattern_add1 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*2 ] & colorex_add;
					uint32_t pattern_add2 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*8 ] & colorex_add;
					uint32_t pattern_add3 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*10] & colorex_add;
					pattern0 |= pattern_add0;
					pattern1 |= pattern_add1;
					pattern2 |= pattern_add2;
					pattern3 |= pattern_add3;
				}
			}
		}
		// 8x8�̎��͎c���0�Ŗ��߂�
		for	(; y<16; y++) {
			int yybase = ( y & 0x8)/8*4*8 +	// �O�̏�i���i
						 ( y & 0x4)/4*1*8 +	// ���̏�i���i
						 ( y & 0x3)  *2;	// 512���[�h�̏ꍇ�A1���C����2���C���ɂȂ�
			i=plNum;
			X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*10+0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*10+1] = 0;
		}

		// ���̃v���[����
		X68_SSR[plNum*SSR_UNIT+4*0+3] = 3;	// �X�v���C�g�͕\��
		X68_SSR[plNum*SSR_UNIT+4*1+3] = 3;	// �X�v���C�g�͕\��
		X68_SSR[plNum*SSR_UNIT+4*2+3] = 3;	// �X�v���C�g�͕\��
		X68_SSR[plNum*SSR_UNIT+4*3+3] = 3;	// �X�v���C�g�͕\��
		vdp->sprite_composition_flag &= ~(1 << plNum);
		while(plNum < m) {
			plNum++;
			X68_SSR[plNum*SSR_UNIT+4*0+3] = 0;	// ���������X�v���C�g�͔�\��
			X68_SSR[plNum*SSR_UNIT+4*1+3] = 0;	// ���������X�v���C�g�͔�\��
			X68_SSR[plNum*SSR_UNIT+4*2+3] = 0;	// ���������X�v���C�g�͔�\��
			X68_SSR[plNum*SSR_UNIT+4*3+3] = 0;	// ���������X�v���C�g�͔�\��
			vdp->sprite_composition_flag |= 1 << plNum;
		}
	}
}

void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	int i,j;
	int plNum = (offset / SAT_SIZE) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % SAT_SIZE; // �����̎��

	switch(type) {
		case 0: // Y���W
			// Y=208/216�̑Ή��̂��߁AY���W��208/216�̒l���ω�������A�X�v���C�g�̍Ĕz�u���s��
			int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
			if ( attribute == HY || old_attribute == HY) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 512���[�h��MSX��Y���W��2�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff)*2 + 16;
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
			}
			break;
		case 1: // X���W
			int ec = 0;
			int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;
			switch(spMode) {
			case 1:
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				break;
			case 2:
				uint8_t* pcolr = vdp->vram + vdp->sprcolrtbl_baddr;
				ec = (pcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ���C�����Ƃ�EC�̓T�|�[�g���Ȃ��̂�1���C���ڂ����݂�
				break;
			}
			int x = attribute & 0xff;
			int adjustx = get_sprite_adjustx(vdp);
			x = ((x - ec*32)*2 + 16 + adjustx*2) & 0x3ff; // MSX��X���W��2�{
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
			}
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]��
			refresh_sprite_512(vdp, plNum);
			break;
		default:
			break;
	}
}


void refresh_sprite_512(ms_vdp_t* vdp, int plNum) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1) {
		refresh_sprite_512_mode1(vdp, plNum);
	} else {
		// ���[�h2�͈����������ƑS�̂ɉe�����o��̂ŁA�����A�����Ԃɂ܂Ƃ߂ď���������
		// TODO: ����Ȃ�ɏd���̂ŁA���������͈͂����肵����
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void refresh_sprite_512_mode1(ms_vdp_t* vdp, int plNum) {
	int i;
	uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;

	unsigned int ptNum = pattr[plNum*SAT_SIZE+2];
	uint32_t color = pattr[plNum*SAT_SIZE+3] & 0xf;
	color = color == 0 ? vdp->alt_color_zero : color; // �u�F�R�[�h0�v�̃X�v���C�g�������Ă��܂����ւ̎b��Ή�
	// MSX��4�h�b�g��(X68000����2�{��8�h�b�g�Ɋg��)
	//uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	uint32_t colorex = colorex_tbl[color];
	if (vdp->sprite_size == 0) { // 8x8
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*PCG_BUF_UNIT_D2X+i] & colorex;
		}
	} else { // 16x16
		for( i = 0; i < 32*4; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*PCG_BUF_UNIT_D2X+i] & colorex;
		}
	}
}

/*
 �X�v���C�g�J���[�e�[�u���ւ̏�������
	 offset: �J���[�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
	 pattern: �������ޒl(����8bit�̂ݎg�p)
*/
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color, int32_t old_color) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) != 2) {
		//�@�X�v���C�g���[�h2�ȊO�͉������Ȃ�
		return;
	} else {
		// ���񔽉f����̂͑�ςȂ̂ŁAVRAM�ɂ��������Ă����A�t���O�𗧂āA�����A�����Ԃɂ܂Ƃ߂���������
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_CC;

		// CC�r�b�g�̃r�b�g�}�b�v�t���O���X�V
		int plNum = offset / 16; // MSX�̃X�v���C�g�v���[���ԍ�
		int line = offset & 0x0f; // ����X�V���郉�C���ԍ�
		if (color & 0x40) { // CC�r�b�g������
			sprite_cc_flags[plNum] |= (1 << line);
		} else {
			sprite_cc_flags[plNum] &= ~(1 << line);
		}
	}
}

/**
 * @brief VSYNC���ԂɃX�v���C�g�̍X�V���܂Ƃ߂čs��
 * 
 * @param vdp 
 */
void ms_vdp_sprite_vsync_draw(ms_vdp_t* vdp) {
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;	// �S��������
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;	// �A�g���r���[�g�e�[�u���̂ݍČ���
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;	// �����I�ɁA�ʒu�����͖���s���悤�ɂ��Ă݂�

	if (!vdp->sprite_refresh_flag) {
		return;
	}

	int hostdebugmode = vdp->hostdebugmode;

	uint8_t* vram = vdp->vram;

	int spSize = vdp->sprite_size == 0 ? 8 : 16;
	int mag512 = (vdp->ms_vdp_current_mode->sprite_mode & 0x80) ? 2 : 1;
	int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;

	int i,j;
	int plNum;
	uint16_t flag = vdp->sprite_refresh_flag;
	if (flag & SPRITE_REFRESH_FLAG_FULL) {
		vdp->sprite_composition_flag = 1;
		for(plNum = 0; plNum < 32; plNum++) {
			if( mag512 == 2 && spSize == 16) {
				// 512�h�b�g���[�h�A16x16�T�C�Y�̎�
				for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+2] = 0x100 + plNum*4+i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
				}
			} else {
				// 256�h�b�g���[�h�̎��A512�h�b�g���[�h��8x8�T�C�Y�̎�
				X68_SSR[plNum*SSR_UNIT+2] = 0x100 + plNum*4; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�					
				for( i=1; i<4; i++) {
					X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// 8x8�̎��͌��3���̃X�v���C�g�͔�\��
				}
			}
		}
		flag |= SPRITE_REFRESH_FLAG_PGEN;	// �ȍ~�̏������S�čs��
	}
	if (flag & SPRITE_REFRESH_FLAG_PGEN) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 11;	// Green
		}
		// �p�^�[���W�F�l���[�^�e�[�u������PCG�o�b�t�@�̍č\�z���s���܂�
		uint32_t sprpgenaddr = vdp->sprpgentbl_baddr & 0x1fe00; // ����9�r�b�g���N���A
		for(i=0;i<256;i++) {
			for(j=0;j<8;j++) {
				write_sprite_pattern(vdp, i*8+j, vram[sprpgenaddr + i*8 + j], -1);
			}
		}
		flag |= SPRITE_REFRESH_FLAG_CC;	// CC�t���O�X�V�APCG�X�V�A�X�v���C�g�A�g���r���[�g�e�[�u���̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_CC) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 6;	// Red
		}
		// �X�v���C�g�J���[�e�[�u����CC�r�b�g�}�b�v�t���O���č쐬
		if (spMode == 2) {
			uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
			for(plNum=0; plNum<32; plNum++) {
				uint16_t ccflag = 0;
				for(i=0; i<16; i++) {
					if (*sprcolr++ & 0x40) { // CC�r�b�g������
						ccflag |= (1 << i);
					}
				}
				sprite_cc_flags[plNum] = ccflag;
			}
		}
		flag |= SPRITE_REFRESH_FLAG_ATTR;	// �X�v���C�g�A�g���r���[�g�e�[�u��(�p�^�[���ԍ�)�̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_ATTR) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 1;	// Blue
		}
		// PCG�X�V����
		if (mag512 == 2 ) {
			// 512�h�b�g���[�h�̎�
			if (spMode == 1) {
				for(i=0;i<32;i++) {
					refresh_sprite_512(vdp, i);
				}
			} else {
				refresh_sprite_512_mode2(vdp);
			}
		} else {
			// 256�h�b�g���[�h�̎�
			if (spMode == 1) {
				for(i=0;i<32;i++) {
					refresh_sprite_256_mode1(vdp, i);
				}
			} else {
				refresh_sprite_256_mode2(vdp);
			}
		}
		flag |= SPRITE_REFRESH_FLAG_COORD;	// �X�v���C�g�A�g���r���[�g�e�[�u��(�\���ʒu)�̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_COORD) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 11 | 0x1f << 6;	// Yellow
		}
		// �X�v���C�g�A�g���r���[�g�e�[�u����XY���W�݂̂̍X�V
		int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
		uint8_t* sprattr = vram + vdp->sprattrtbl_baddr;
		uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
		uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
		int visible_sprite_planes = 0;
		plNum = 0;
		int x,y,ec = 0;
		int adjustx = get_sprite_adjustx(vdp) * mag512;
		if ( (vdp->r08 & 0x02) == 0 ) {
			// �X�v���C�g��\���ł͂Ȃ���
			for(;plNum<32;plNum++) {
				y = sprattr[plNum*SAT_SIZE+0];
				x = sprattr[plNum*SAT_SIZE+1];
				if ( y == HY) {
					// ���̃v���[���ȍ~�͕`�悵�Ȃ�
					break;
				}
				if(spMode == 1) {
					if( (sprattr[plNum*SAT_SIZE+3] & 0x04) == 0) {
						// �F�R�[�h0�̃X�v���C�g�͕\�����Ȃ�
						for( i=0; i<4; i++) {
						//	X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// �X�v���C�g�͔�\��
						}
						//continue;
					}
					ec = (sprattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				} else {
					if(vdp->sprite_composition_flag & (1 << plNum)) {
						// �������ꂽ�X�v���C�g�v���[���͎�O�̃v���[���ɍ����ς݂Ȃ̂ŃX�L�b�v
						X68_SSR[plNum*SSR_UNIT+3] = 0; // �X�v���C�g�ߕ\��
						continue;
					}
					ec = (sprcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ���C�����Ƃ�EC�̓T�|�[�g���Ȃ��̂�1���C���ڂ����݂�
				}
				y = ((y + 1 - scroll_offset) & 0xff) * mag512 + 16;
				x = ((x  - ec*32)* mag512 + 16 + adjustx) & 0x3ff;
				if( mag512 == 2 && spSize == 16) {
					// 512�h�b�g���[�h�A16x16�T�C�Y�̎�
					for( i=0; i<4; i++) {
						X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+3] = 3; // �X�v���C�g�\��
					}
				} else {
					// 256�h�b�g���[�h�̎��A512�h�b�g���[�h��8x8�T�C�Y�̎�
					X68_SSR[plNum*SSR_UNIT+0] = x;
					X68_SSR[plNum*SSR_UNIT+1] = y;
					X68_SSR[plNum*SSR_UNIT+3] = 3; // �X�v���C�g�\��
				}
			}
		}
		visible_sprite_planes = plNum;
		// �ȉ��̃X�v���C�g�v���[���͔�\���ɂ���
		for(;plNum<32;plNum++) {
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+3] = 0; // �X�v���C�g��\��
			}
		}
		vdp->last_visible_sprite_planes = visible_sprite_planes;
		vdp->last_visible_sprite_size = vdp->sprite_size;
	}

	// �t���O�N���A
	vdp->sprite_refresh_flag = 0;
}