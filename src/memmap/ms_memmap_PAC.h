#ifndef MS_MEMMAP_PAC_H
#define MS_MEMMAP_PAC_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_PAC {
	ms_memmap_driver_t base;

	int buffer_length;
	int sram_enabled;
} ms_memmap_driver_PAC_t;

ms_memmap_driver_PAC_t* ms_memmap_PAC_alloc();
void ms_memmap_PAC_init(ms_memmap_driver_PAC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, int buffer_length);
void ms_memmap_PAC_deinit(ms_memmap_driver_PAC_t* instance);

#endif