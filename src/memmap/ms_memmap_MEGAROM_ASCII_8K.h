#ifndef MS_MEMMAP_MEGAROM_ASCII_8K_H
#define MS_MEMMAP_MEGAROM_ASCII_8K_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

#define MEGAROM_ASCII_8K_SIZE (128*1024)

/*
*/
typedef struct ms_memmap_driver_MEGAROM_ASCII_8K {
	ms_memmap_driver_t base;
	// extended properties
	int num_segments;
	int selected_segment[4];	// Page0-3のそれぞれのセグメント選択状態
} ms_memmap_driver_MEGAROM_ASCII_8K_t;

ms_memmap_driver_MEGAROM_ASCII_8K_t* ms_memmap_MEGAROM_ASCII_8K_alloc();
void ms_memmap_MEGAROM_ASCII_8K_init(ms_memmap_driver_MEGAROM_ASCII_8K_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length);
void ms_memmap_MEGAROM_ASCII_8K_deinit(ms_memmap_driver_MEGAROM_ASCII_8K_t* instance);

#endif