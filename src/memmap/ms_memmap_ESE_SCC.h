#ifndef MS_MEMMAP_ESE_SCC_H
#define MS_MEMMAP_ESE_SCC_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_ESE_SCC {
	ms_memmap_driver_t base;
	// extended properties

	// SCC���W�X�^�Z�O�����g(�Z�O�����g�ԍ�0x3f���Z�b�g����ƃA�N�Z�X�\�ɂȂ�)
	uint8_t* scc_segment;

	int num_segments;

	// region 0-3�̂��ꂼ��̃Z�O�����g�I�����
	// region �͈ȉ��̂悤�Ɋ���U���Ă��āA�I�t�Z�b�g�����Ă���̂Œ���
	// 0 : 0x4000-0x5FFF (0xc000-0xdFFF�ɂ��~���[)
	// 1 : 0x6000-0x7FFF (0xE000-0xFFFF�ɂ��~���[)
	// 2 : 0x8000-0x9FFF (0x0000-0x1FFF�ɂ��~���[)
	// 3 : 0xA000-0xBFFF (0x2000-0x3FFF�ɂ��~���[)
	int selected_segment[4];

	// SRAM�̏������ݗL���t���O
	// ��ʃo���N�I�����W�X�^(0x7ffe, 0x7fff)��bit4��1�̂Ƃ��ɗL��
	int write_enable;

	// �Z�O�����g 0x40-0x7f�̗L���t���O
	// ��ʃo���N�I�����W�X�^(0x7ffe, 0x7fff)��bit6��1�̂Ƃ��ɗL��
	// region 0�ł̂ݗL���Ȃ̂Œ���
	int upper_bank_enable;

	// SRAM�t�@�C���̃p�X
	uint8_t file_path[256];
} ms_memmap_driver_ESE_SCC_t;

ms_memmap_driver_ESE_SCC_t* ms_memmap_ESE_SCC_alloc();
void ms_memmap_ESE_SCC_init(ms_memmap_driver_ESE_SCC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path);
void ms_memmap_ESE_SCC_deinit(ms_memmap_driver_ESE_SCC_t* instance);

#endif