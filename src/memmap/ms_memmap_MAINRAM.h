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

void ms_memmap_did_attach_MAINRAM(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_MAINRAM(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_MAINRAM(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_MAINRAM(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif