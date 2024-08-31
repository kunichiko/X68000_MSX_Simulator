#ifndef MS_DISK_CONTROLLER_TC8566AF_H
#define MS_DISK_CONTROLLER_TC8566AF_H

#include <stdint.h>

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
} ms_disk_controller_TC8566AF_t;


#endif