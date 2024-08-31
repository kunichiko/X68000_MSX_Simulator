/*
	TC8566AFのI/O

	Address	R/W	Feature
	0x3FF8	W	レジスタ2 を更新 (write only)
	0x3FF9	W	レジスタ3 を更新 (write only)
	0x3FFA	R/W	レジスタ4 を参照・更新
	0x3FFB	R/W	レジスタ5 を参照・更新

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk_controller_TC8566AF.h"
#include "../memmap/ms_memmap.h"

#define THIS ms_disk_controller_TC8566AF_t

THIS* ms_disk_controller_TC8566AF_alloc() {
	return (THIS*)new_malloc(sizeof(ms_disk_controller_TC8566AF_t));
}

void ms_disk_controller_TC8566A_init(ms_disk_controller_TC8566AF_t* instance, ms_disk_container_t* container) {
	ms_disk_drive_floppy_init(&instance->drive, container);
}

void ms_disk_controller_TC8566A_deinit(ms_disk_controller_TC8566AF_t* instance) {

}


/**
 * @brief 
 * 
 * 0x3FF8 - レジスタ2 - ドライブ選択 (Write Only)
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
 * ?		?		LED2	LED1	?		DriveId	DriveId	DriveId
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg2_write(THIS* d, uint8_t data) {
	d->led1 = (data >> 4) & 0x01;
	d->led2 = (data >> 5) & 0x01;
	d->driveId = data & 0x07;
}

/**
 * @brief TC8566AFのレジスタ3に書き込む
 * 
 * 何も起こらない
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg3_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AFのレジスタ4を読む
 * 
 * ステータスレジスタ
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bit-1	Bit-0
 * RQM		DIO		NDM		CB		D3B		D2B		D1B		D0B
 * 
 * RQM; Request for Master
 *	1: FDCがデータ受け付け中、またはデータ送信準備完了
 *	0: FDCがデータ受け取り不可、またはデータ送信準備未完了
 * DIO; Data Input/Output
 *	1: FDC→ホスト方向転送
 *	0: ホスト→FDC方向転送
 * NDM; NonDMA Mode
 *	1: FDCがNonDMAモードでデータ転送中でサービスを要求している
 *	0: FDCのサービス要求なし
 * CB; FDC Busy
 *	1: FDCがコマンドを実行中
 *	0: FDCはコマンドを受け付け可能
 * D3B?D0B; FDD 3~0 Busy
 *	1: FDD 0?3 がシーク中
 * 	0: FDD 0?3 がシーク中ではない
 * @param d 
 * @return uint8_t 
 */
uint8_t _TC8556AF_reg4_read(THIS* d) {
	int request_for_master = 0;
	int data_input_output = 0;
	int non_dma_mode = 0;
	int fdc_busy = 0;
	int fdd_busy = 0;
	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		// Perform idle operation
		break;
	case TC8566AF_PHASE_COMMAND:
		// Perform d->command operation
		fdc_busy = 1;
		break;
	case TC8566AF_PHASE_RESULT:
		fdc_busy = 1;
		request_for_master = 1;
		switch(d->command) {
		case TC8566AF_CMD_READ_DATA:
			// Perform read data operation
			data_input_output = 1; // FDC -> Host
			break;
		case TC8566AF_CMD_WRITE_DATA:
			// Perform write data operation
			data_input_output = 0; // Host -> FDC
			break;
		case TC8566AF_CMD_WRITE_DELETED_DATA:
			// Perform write deleted data operation
			break;
		case TC8566AF_CMD_READ_DELETED_DATA:
			// Perform read deleted data operation
			break;
		case TC8566AF_CMD_READ_DIAGNOSTIC:
			// Perform read diagnostic operation
			break;
		case TC8566AF_CMD_READ_ID:
			// Perform read ID operation
			break;
		case TC8566AF_CMD_FORMAT:
			// Perform format operation
			break;
		case TC8566AF_CMD_SCAN_EQUAL:
			// Perform scan equal operation
			break;
		case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
			// Perform scan low or equal operation
			break;
		case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
			// Perform scan high or equal operation
			break;
		case TC8566AF_CMD_SEEK:
			// Perform seek operation
			break;
		case TC8566AF_CMD_RECALIBRATE:
			// Perform recalibrate operation
			break;
		case TC8566AF_CMD_SENSE_INTERRUPT_STATUS:
			// Perform sense interrupt status operation
			break;
		case TC8566AF_CMD_SPECIFY:
			// Perform specify operation
			break;
		case TC8566AF_CMD_SENSE_DEVICE_STATUS:
			// Perform sense device status operation
			break;
		}
	}
	return 0;
}

/**
 * @brief TC8566AFのレジスタ4に書き込む
 * 
 * 何も起こらない
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg4_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AFのレジスタ5を読む
 * 
 * データレジスタです
 * 
 * @param d 
 * @return uint8_t 
 */
uint8_t _TC8556AF_reg5_read(THIS* d) {
	return 0;
}

/**
 * @brief TC8566AFのレジスタ5に書き込む
 * 
 * データレジスタです。TC8566AFのステート(Phase)によって動作が異なります。
 * 
 * ## 1. Idle Phase
 * アイドルフェーズでは実行するコマンドの種類を書き込みます。
 * 
 * if ((value & 0x1f) == 0x06) d->command = CMD_READ_DATA;
 * if ((value & 0x3f) == 0x05) d->command = CMD_WRITE_DATA;
 * if ((value & 0x3f) == 0x09) d->command = CMD_WRITE_DELETED_DATA;
 * if ((value & 0x1f) == 0x0c) d->command = CMD_READ_DELETED_DATA;
 * if ((value & 0xbf) == 0x02) d->command = CMD_READ_DIAGNOSTIC;
 * if ((value & 0xbf) == 0x0a) d->command = CMD_READ_ID;
 * if ((value & 0xbf) == 0x0d) d->command = CMD_FORMAT;
 * if ((value & 0x1f) == 0x11) d->command = CMD_SCAN_EQUAL;
 * if ((value & 0x1f) == 0x19) d->command = CMD_SCAN_LOW_OR_EQUAL;
 * if ((value & 0x1f) == 0x1d) d->command = CMD_SCAN_HIGH_OR_EQUAL;
 * if ((value & 0xff) == 0x0f) d->command = CMD_SEEK;
 * if ((value & 0xff) == 0x07) d->command = CMD_RECALIBRATE;
 * if ((value & 0xff) == 0x08) d->command = CMD_SENSE_INTERRUPT_STATUS;
 * if ((value & 0xff) == 0x03) d->command = CMD_SPECIFY;
 * if ((value & 0xff) == 0x04) d->command = CMD_SENSE_DEVICE_STATUS;
 *
 * ## 2. d->command Phase
 * コマンドフェーズでは、コマンドに必要なパラメータを書き込みます。コマンドの種類によって何回か繰り返しwrite）
 * 
 * ## 3. Result Phase
 * データ読み込み or 書き込みを行うフェーズです
 * 
 * 実際のFDCは、Command Phaseの後 Execution Phaseがあり、そこでディスクへのアクセスが行われますが、
 * エミュレータはそこの待ち時間を考慮する必要がないので、省略しています。
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg5_write(THIS* d, uint8_t value) {
	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		d->command_params_index = 0;
		d->command_params_rest = 0;
		d->result_datas_rest = 0;
		if ((value & 0x1f) == 0x06) {
			d->command = TC8566AF_CMD_READ_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x3f) == 0x05) {
			d->command = TC8566AF_CMD_WRITE_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x3f) == 0x09) {
			d->command = TC8566AF_CMD_WRITE_DELETED_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x1f) == 0x0c) {
			d->command = TC8566AF_CMD_READ_DELETED_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0xbf) == 0x02) {
			d->command = TC8566AF_CMD_READ_DIAGNOSTIC;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0xbf) == 0x0a) {
			d->command = TC8566AF_CMD_READ_ID;
			d->command_params_rest = 1;	// flag
		} else if ((value & 0xbf) == 0x0d) {
			d->command = TC8566AF_CMD_FORMAT;
			d->command_params_rest = 5;	// flag, n, sc, gpl, d
		} else if ((value & 0x1f) == 0x11) {
			d->command = TC8566AF_CMD_SCAN_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0x1f) == 0x19) {
			d->command = TC8566AF_CMD_SCAN_LOW_OR_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0x1f) == 0x1d) {
			d->command = TC8566AF_CMD_SCAN_HIGH_OR_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0xff) == 0x0f) {
			d->command = TC8566AF_CMD_SEEK;
			d->command_params_rest = 2;	// flag, ncn
		} else if ((value & 0xff) == 0x07) {
			d->command = TC8566AF_CMD_RECALIBRATE;
			d->command_params_rest = 1;	// flag
		} else if ((value & 0xff) == 0x08) {
			d->command = TC8566AF_CMD_SENSE_INTERRUPT_STATUS;
			d->command_params_rest = 0;
			d->phase = TC8566AF_PHASE_RESULT; // Command Phase, Execution Phase がないので直接Result Phaseへ
			return;
		} else if ((value & 0xff) == 0x03) {
			d->command = TC8566AF_CMD_SPECIFY;
			d->command_params_rest = 2;	// srt/hut, hlt/nd
		} else if ((value & 0xff) == 0x04) {
			d->command = TC8566AF_CMD_SENSE_DEVICE_STATUS;
			d->command_params_rest = 1;	// flag
		} else {
			// invalid command
			d->command = TC8566AF_CMD_INVALID;
			d->phase = TC8566AF_PHASE_RESULT; // Command Phase, Execution Phase がないので直接Result Phaseへ
			return;
		}

		// Command Phase へ移行
		d->phase = TC8566AF_PHASE_COMMAND;
		if (d->command_params_rest == 0) {
			// 異常状態
			printf("Command Phase で command_params_rest が 0 です\n");
		}
		break;
	case TC8566AF_PHASE_COMMAND:
		d->command_params[d->command_params_index++] = value;
		d->command_params_rest--;
		if (d->command_params_rest == 0) {
			// Execute
			ms_fdc_execute_TC8566AF(d);
			d->phase = TC8566AF_PHASE_RESULT;
		}
		break;
	case TC8566AF_PHASE_RESULT:
		switch(d->command) {

		}
		break;
	}
}

void ms_fdc_execute_TC8566AF(THIS* d) {
	int data_input_output = 0;
	switch(d->command) {
	case TC8566AF_CMD_READ_DATA:
		// Perform read data operation
		data_input_output = 1; // FDC -> Host
		break;
	case TC8566AF_CMD_WRITE_DATA:
		// Perform write data operation
		data_input_output = 0; // Host -> FDC
		break;
	case TC8566AF_CMD_WRITE_DELETED_DATA:
		// Perform write deleted data operation
		break;
	case TC8566AF_CMD_READ_DELETED_DATA:
		// Perform read deleted data operation
		break;
	case TC8566AF_CMD_READ_DIAGNOSTIC:
		// Perform read diagnostic operation
		break;
	case TC8566AF_CMD_READ_ID:
		// Perform read ID operation
		break;
	case TC8566AF_CMD_FORMAT:
		// Perform format operation
		break;
	case TC8566AF_CMD_SCAN_EQUAL:
		// Perform scan equal operation
		break;
	case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
		// Perform scan low or equal operation
		break;
	case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
		// Perform scan high or equal operation
		break;
	case TC8566AF_CMD_SEEK:
		// Perform seek operation
		break;
	case TC8566AF_CMD_RECALIBRATE:
		// Perform recalibrate operation
		break;
	case TC8566AF_CMD_SENSE_INTERRUPT_STATUS:
		// Perform sense interrupt status operation
		break;
	case TC8566AF_CMD_SPECIFY:
		// Perform specify operation
		break;
	case TC8566AF_CMD_SENSE_DEVICE_STATUS:
		// Perform sense device status operation
		break;
	}
}