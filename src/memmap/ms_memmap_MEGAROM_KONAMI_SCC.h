#ifndef MS_MEMMAP_MEGAROM_KONAMI_SCC_H
#define MS_MEMMAP_MEGAROM_KONAMI_SCC_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

#define MEGAROM_KONAMI_SCC_SIZE (128*1024)

/*
*/
typedef struct ms_memmap_driver_MEGAROM_KONAMI_SCC {
	ms_memmap_driver_t base;
	// extended properties

	// SCC���[�h
	uint8_t scc_mode;	// 0: SCC, 1: SCC+

	// SCC���W�X�^�Z�O�����g(�Z�O�����g�ԍ�0x3f���Z�b�g����ƃA�N�Z�X�\�ɂȂ�)
	uint8_t* scc_segment;

	int num_segments;
	int selected_segment[4];	// region 0-3�̂��ꂼ��̃Z�O�����g�I�����
} ms_memmap_driver_MEGAROM_KONAMI_SCC_t;

ms_memmap_driver_MEGAROM_KONAMI_SCC_t* ms_memmap_MEGAROM_KONAMI_SCC_alloc();
void ms_memmap_MEGAROM_KONAMI_SCC_init(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length);
void ms_memmap_MEGAROM_KONAMI_SCC_deinit(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* instance);

#endif