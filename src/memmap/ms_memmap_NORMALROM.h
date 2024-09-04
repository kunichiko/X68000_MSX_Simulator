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

void ms_memmap_did_attach_NORMALROM(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_NORMALROM(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_NORMALROM(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_NORMALROM(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif