#ifndef MS_MEMMAP_ESE_RAM_H
#define MS_MEMMAP_ESE_RAM_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_ESE_RAM {
	ms_memmap_driver_t base;
	// extended properties
	int num_segments;

	// region 0-3�̂��ꂼ��̃Z�O�����g�I�����
	// region �͈ȉ��̂悤�Ɋ���U���Ă��āA�I�t�Z�b�g�����Ă���̂Œ���
	// 0 : 0x4000-0x5FFF (0xc000-0xdFFF�ɂ��~���[)
	// 1 : 0x6000-0x7FFF (0xE000-0xFFFF�ɂ��~���[)
	// 2 : 0x8000-0x9FFF (0x0000-0x1FFF�ɂ��~���[)
	// 3 : 0xA000-0xBFFF (0x2000-0x3FFF�ɂ��~���[)
	int selected_segment[4];

	// SRAM�t�@�C���̃p�X
	uint8_t file_path[256];
} ms_memmap_driver_ESE_RAM_t;

ms_memmap_driver_ESE_RAM_t* ms_memmap_ESE_RAM_alloc();
void ms_memmap_ESE_RAM_init(ms_memmap_driver_ESE_RAM_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path);
void ms_memmap_ESE_RAM_deinit(ms_memmap_driver_ESE_RAM_t* instance);

#endif