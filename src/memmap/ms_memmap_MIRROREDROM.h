#ifndef MS_MEMMAP_MIRROREDROM_H
#define MS_MEMMAP_MIRROREDROM_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_MIRROREDROM {
	ms_memmap_driver_t base;

	int buffer_length;
} ms_memmap_driver_MIRROREDROM_t;

ms_memmap_driver_MIRROREDROM_t* ms_memmap_MIRROREDROM_alloc();
void ms_memmap_MIRROREDROM_init(ms_memmap_driver_MIRROREDROM_t* instance, ms_memmap_t* memmap, uint8_t* buffer, int buffer_length);
void ms_memmap_MIRROREDROM_deinit(ms_memmap_driver_MIRROREDROM_t* instance);

#endif