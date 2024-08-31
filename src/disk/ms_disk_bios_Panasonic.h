#ifndef MS_MEMMAP_DISKBIOS_PANASONIC_H
#define MS_MEMMAP_DISKBIOS_PANASONIC_H

#include "ms_memmap.h"

#define DISKBIOS_PANASONIC_SIZE (16*1024)

#define DISKBIOS_PANASONIC_CMD_READ_DATA 0x06
#define DISKBIOS_PANASONIC_CMD_WRITE_DATA 0x05
#define DISKBIOS_PANASONIC_CMD_WRITE_DELETED_DATA 0x09
#define DISKBIOS_PANASONIC_CMD_READ_DELETED_DATA 0x0c
#define DISKBIOS_PANASONIC_CMD_READ_DIAGNOSTIC 0x02
#define DISKBIOS_PANASONIC_CMD_READ_ID 0x0a
#define DISKBIOS_PANASONIC_CMD_FORMAT 0x0d
#define DISKBIOS_PANASONIC_CMD_SCAN_EQUAL 0x11
#define DISKBIOS_PANASONIC_CMD_SCAN_LOW_OR_EQUAL 0x19
#define DISKBIOS_PANASONIC_CMD_SCAN_HIGH_OR_EQUAL 0x1d
#define DISKBIOS_PANASONIC_CMD_SEEK 0x0f
#define DISKBIOS_PANASONIC_CMD_RECALIBRATE 0x07
#define DISKBIOS_PANASONIC_CMD_SENSE_INTERRUPT_STATUS 0x08
#define DISKBIOS_PANASONIC_CMD_SPECIFY 0x03
#define DISKBIOS_PANASONIC_CMD_SENSE_DEVICE_STATUS 0x04
#define DISKBIOS_PANASONIC_CMD_INVALID 0xff

#define DISKBIOS_PANASONIC_PHASE_IDLE 0
#define DISKBIOS_PANASONIC_PHASE_COMMAND 1
#define DISKBIOS_PANASONIC_PHASE_RESULT 2

/**
 * @brief TC8566AFを使用する Panasonicの DISK BIOS ROMのドライバ
 * 
 * メモリマップドI/Oを持ち、そのI／Oを通じて TC8566AFコントローラーを操作します
 */
typedef struct ms_memmap_driver_DISKBIOS_PANASONIC {
	ms_memmap_driver_t base;
	// extended properties
	int length;
	// registers
	uint8_t phase;
	uint8_t command;
	uint8_t command_params[8];
	uint8_t command_params_index;
	uint8_t command_params_rest;
	uint8_t result_datas[8];
	uint8_t result_datas_rest;
	// drive status
	uint8_t led1;
	uint8_t led2;
	uint8_t driveId;
} ms_memmap_driver_DISKBIOS_PANASONIC_t;

ms_memmap_driver_DISKBIOS_PANASONIC_t* ms_disk_bios_Panasonic_alloc();
void ms_disk_bios_Panasonic_init(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length);
void ms_disk_bios_Panasonic_deinit(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance);

void ms_memmap_deinit_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);
void ms_memmap_did_attach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);
int ms_memmap_will_detach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver);

void ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

uint8_t ms_memmap_read8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr);
void ms_memmap_write16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

#endif