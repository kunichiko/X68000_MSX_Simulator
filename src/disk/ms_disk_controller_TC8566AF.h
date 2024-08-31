#ifndef MS_DISK_CONTROLLER_TC8566AF_H
#define MS_DISK_CONTROLLER_TC8566AF_H

#include <stdint.h>

#include "ms_disk_drive_floppy.h"

#define TC8566AF_CMD_READ_DATA 0x06
#define TC8566AF_CMD_WRITE_DATA 0x05
#define TC8566AF_CMD_WRITE_DELETED_DATA 0x09
#define TC8566AF_CMD_READ_DELETED_DATA 0x0c
#define TC8566AF_CMD_READ_DIAGNOSTIC 0x02
#define TC8566AF_CMD_READ_ID 0x0a
#define TC8566AF_CMD_FORMAT 0x0d
#define TC8566AF_CMD_SCAN_EQUAL 0x11
#define TC8566AF_CMD_SCAN_LOW_OR_EQUAL 0x19
#define TC8566AF_CMD_SCAN_HIGH_OR_EQUAL 0x1d
#define TC8566AF_CMD_SEEK 0x0f
#define TC8566AF_CMD_RECALIBRATE 0x07
#define TC8566AF_CMD_SENSE_INTERRUPT_STATUS 0x08
#define TC8566AF_CMD_SPECIFY 0x03
#define TC8566AF_CMD_SENSE_DEVICE_STATUS 0x04
#define TC8566AF_CMD_INVALID 0xff

#define TC8566AF_PHASE_IDLE 0
#define TC8566AF_PHASE_COMMAND 1
#define TC8566AF_PHASE_DATA_TRANSFER 2		// Execution Phase
#define TC8566AF_PHASE_RESULT 3

/**
 * @brief 
 * 	Address	R/W	Feature
 *	0x3FF8	W	レジスタ2 を更新 (write only)
 *	0x3FF9	W	レジスタ3 を更新 (write only)
 *	0x3FFA	R/W	レジスタ4 を参照・更新
 *	0x3FFB	R/W	レジスタ5 を参照・更新
 */
typedef struct ms_disk_controller_TC8566AF {
	// methods
	void (*write_reg2)(struct ms_disk_controller_TC8566AF* d, uint8_t data);
	void (*write_reg3)(struct ms_disk_controller_TC8566AF* d, uint8_t data);
	uint8_t (*read_reg4)(struct ms_disk_controller_TC8566AF* d);
	void (*write_reg4)(struct ms_disk_controller_TC8566AF* d, uint8_t data);
	uint8_t (*read_reg5)(struct ms_disk_controller_TC8566AF* d);
	void (*write_reg5)(struct ms_disk_controller_TC8566AF* d, uint8_t data);

	// references
	ms_disk_drive_floppy_t drive;

	// registers
	uint8_t phase;
	uint8_t command;
	uint8_t command_params[8];
	uint8_t command_params_index;
	uint8_t command_params_rest;
	uint16_t transfer_datas_rest;
	uint8_t result_datas[8];
	uint8_t result_datas_rest;
	// drive status
	uint8_t status0;
	uint8_t status1;
	uint8_t status2;
	uint8_t status3;
	uint8_t led1;
	uint8_t led2;
	uint8_t driveId;
} ms_disk_controller_TC8566AF_t;

ms_disk_controller_TC8566AF_t* ms_disk_controller_TC8566AF_alloc();
void ms_disk_controller_TC8566A_init(ms_disk_controller_TC8566AF_t* instance, ms_disk_container_t* container);
void ms_disk_controller_TC8566A_deinit(ms_disk_controller_TC8566AF_t* instance);

void ms_fdc_execute_TC8566AF(ms_disk_controller_TC8566AF_t* d);

#endif