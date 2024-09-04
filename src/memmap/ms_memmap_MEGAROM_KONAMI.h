#ifndef MS_MEMMAP_MEGAROM_KONAMI_H
#define MS_MEMMAP_MEGAROM_KONAMI_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

#define MEGAROM_KONAMI_SIZE (128*1024)

/*
*/
typedef struct ms_memmap_driver_MEGAROM_KONAMI {
	ms_memmap_driver_t base;
	// extended properties
	int num_segments;
	int selected_segment[4];	// Page0-3のそれぞれのセグメント選択状態
} ms_memmap_driver_MEGAROM_KONAMI_t;

ms_memmap_driver_MEGAROM_KONAMI_t* ms_memmap_MEGAROM_KONAMI_alloc();
void ms_memmap_MEGAROM_KONAMI_init(ms_memmap_driver_MEGAROM_KONAMI_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length);
void ms_memmap_MEGAROM_KONAMI_deinit(ms_memmap_driver_MEGAROM_KONAMI_t* instance);

void ms_memmap_did_attach_MEGAROM_KONAMI(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_MEGAROM_KONAMI(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_MEGAROM_KONAMI(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_MEGAROM_KONAMI(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_MEGAROM_KONAMI(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_MEGAROM_KONAMI(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_MEGAROM_KONAMI(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif