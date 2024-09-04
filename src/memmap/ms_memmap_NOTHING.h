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

void ms_memmap_did_attach_NOTHING(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_NOTHING(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_NOTHING(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_NOTHING(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_NOTHING(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_NOTHING(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_NOTHING(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif