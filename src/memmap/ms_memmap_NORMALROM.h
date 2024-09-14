#ifndef MS_MEMMAP_NORMALROM_H
#define MS_MEMMAP_NORMALROM_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_NORMALROM {
	ms_memmap_driver_t base;
} ms_memmap_driver_NORMALROM_t;

ms_memmap_driver_NORMALROM_t* ms_memmap_NORMALROM_alloc();
void ms_memmap_NORMALROM_init(ms_memmap_driver_NORMALROM_t* instance, ms_memmap_t* memmap, uint8_t* buffer, int page);
void ms_memmap_NORMALROM_deinit(ms_memmap_driver_NORMALROM_t* instance);

#endif