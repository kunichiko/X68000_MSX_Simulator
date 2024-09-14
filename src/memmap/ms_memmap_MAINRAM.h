#ifndef MS_MEMMAP_MAINRAM_H
#define MS_MEMMAP_MAINRAM_H

#include "ms_memmap_driver.h"

#define MAINRAM_SIZE (128*1024)

/*
*/
typedef struct ms_memmap_driver_MAINRAM {
	ms_memmap_driver_t base;
	// extended properties
	int num_segments;
	int selected_segment[4];	// Page0-3のそれぞれのセグメント選択状態
} ms_memmap_driver_MAINRAM_t;

ms_memmap_driver_MAINRAM_t* ms_memmap_MAINRAM_alloc();
void ms_memmap_MAINRAM_init(ms_memmap_driver_MAINRAM_t* instance, ms_memmap_t* memmap);
void ms_memmap_MAINRAM_deinit(ms_memmap_driver_MAINRAM_t* instance);

#endif