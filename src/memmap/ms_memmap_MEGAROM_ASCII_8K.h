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

	// region 0-3のそれぞれのセグメント選択状態
	// region は以下のように割り振っていて、オフセットがついているので注意
	// 0 : 0x4000-0x5FFF (0xc000-0xdFFFにもミラー)
	// 1 : 0x6000-0x7FFF (0xE000-0xFFFFにもミラー)
	// 2 : 0x8000-0x9FFF (0x0000-0x1FFFにもミラー)
	// 3 : 0xA000-0xBFFF (0x2000-0x3FFFにもミラー)
	int selected_segment[4];
} ms_memmap_driver_MEGAROM_ASCII_8K_t;

ms_memmap_driver_MEGAROM_ASCII_8K_t* ms_memmap_MEGAROM_ASCII_8K_alloc();
void ms_memmap_MEGAROM_ASCII_8K_init(ms_memmap_driver_MEGAROM_ASCII_8K_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length);
void ms_memmap_MEGAROM_ASCII_8K_deinit(ms_memmap_driver_MEGAROM_ASCII_8K_t* instance);

#endif