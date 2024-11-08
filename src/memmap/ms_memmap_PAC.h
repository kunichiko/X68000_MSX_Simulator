#ifndef MS_MEMMAP_PAC_H
#define MS_MEMMAP_PAC_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_PAC {
	ms_memmap_driver_t base;

	uint8_t file_path[256];
	int sram_enabled;
	uint8_t _5ffe;
	uint8_t _5fff;
	uint8_t disabled_buffer[8192];
} ms_memmap_driver_PAC_t;

ms_memmap_driver_PAC_t* ms_memmap_PAC_alloc();
void ms_memmap_PAC_init(ms_memmap_driver_PAC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t buf_length, uint32_t file_length, uint8_t* file_path);
void ms_memmap_PAC_deinit(ms_memmap_driver_PAC_t* instance);

#endif