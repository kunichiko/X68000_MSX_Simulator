#ifndef MS_MEMMAP_NOTHING_H
#define MS_MEMMAP_NOTHING_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

typedef struct ms_memmap_driver_NOTHING ms_memmap_driver_NOTHING_t;

/*
*/
typedef struct ms_memmap_driver_NOTHING {
	ms_memmap_driver_t base;
} ms_memmap_driver_NOTHING_t;

ms_memmap_driver_NOTHING_t* ms_memmap_NOTHING_alloc();
void ms_memmap_NOTHING_init(ms_memmap_driver_NOTHING_t* instance, ms_memmap_t* memmap);
void ms_memmap_NOTHING_deinit(ms_memmap_driver_NOTHING_t* instance);

#endif