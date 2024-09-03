/**
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
#include "../ms.h"
#include "ms_disk_controller_TC8566AF.h"
#include "../memmap/ms_memmap.h"

#define THIS ms_disk_controller_TC8566AF_t

static void _TC8556AF_reg2_write(THIS* d, uint8_t data);
static void _TC8556AF_reg3_write(THIS* d, uint8_t data);
static uint8_t _TC8556AF_reg4_read(THIS* d);
static void _TC8556AF_reg4_write(THIS* d, uint8_t data);
static uint8_t _TC8556AF_reg5_read(THIS* d);
static void _TC8556AF_reg5_write(THIS* d, uint8_t data);

typedef void (*exec_func_t)(THIS* d);
typedef uint8_t (*trans_read_func_t)(THIS* d, uint8_t* finished);
typedef void (*trans_write_func_t)(THIS* d, uint8_t data, uint8_t* finished);
typedef uint8_t (*result_func_t)(THIS* d, uint8_t* finished);

THIS* ms_disk_controller_TC8566AF_alloc() {
	return (THIS*)new_malloc(sizeof(ms_disk_controller_TC8566AF_t));
}

static void read_data_exec(THIS* d);
static uint8_t read_data_transfer(THIS* d, uint8_t* finished); 
static void write_data_exec(THIS* d);
static void write_data_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void read_deleted_data_exec(THIS* d);
static uint8_t read_deleted_data_transfer(THIS* d, uint8_t* finished);
static void write_deleted_data_exec(THIS* d);
static void write_deleted_data_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void read_diagnostic_exec(THIS* d);
static uint8_t read_diagnostic_transfer(THIS* d, uint8_t* finished);
static void read_id_exec(THIS* d);
static uint8_t read_id_transfer(THIS* d, uint8_t* finished);
static void format_exec(THIS* d);  // format = write id
static void format_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_equal_exec(THIS* d);
static void scan_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_low_or_equal_exec(THIS* d);
static void scan_low_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_high_or_equal_exec(THIS* d);
static void scan_high_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void seek_exec(THIS* d);
static void recalibrate_exec(THIS* d);
static void sense_interrupt_status_exec(THIS* d);
static void specify_exec(THIS* d);
static void sense_device_status_exec(THIS* d);
static void invalid_exec(THIS* d);

static uint8_t chrn_result(THIS* d, uint8_t* finished);
static uint8_t sense_interrupt_status_result(THIS* d, uint8_t* finished);
static uint8_t sense_device_status_result(THIS* d, uint8_t* finished);
static uint8_t st0_result(THIS* d, uint8_t* finished);

void end_command(THIS* d);
void execute(THIS* d);
void to_result_phase(THIS* d);

static TC8566AF_command_t Command(uint8_t* name, uint8_t command_byte_count, exec_func_t execution_phase, trans_read_func_t read_func, trans_write_func_t write_func, result_func_t result_phase) {
	TC8566AF_command_t ret;
	ret.name = name;
	ret.command_byte_count = command_byte_count;
	ret.execution_phase = execution_phase;
	ret.trans_read = read_func;
	ret.trans_write = write_func;
	ret.result_phase = result_phase;
	return ret;
}

void ms_disk_controller_TC8566A_init(THIS* instance, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	instance->write_reg2 = _TC8556AF_reg2_write;
	instance->write_reg3 = _TC8556AF_reg3_write;
	instance->read_reg4 = _TC8556AF_reg4_read;
	instance->write_reg4 = _TC8556AF_reg4_write;
	instance->read_reg5 = _TC8556AF_reg5_read;
	instance->write_reg5 = _TC8556AF_reg5_write;

	int i = 0;
	ms_disk_drive_floppy_init(&instance->drive[i], container);
	for(i=1; i<4; i++) {
		ms_disk_drive_floppy_init(&instance->drive[i], NULL);
	}

	instance->phase = TC8566AF_PHASE_IDLE;
	for(i=0; i< 32; i++) {
		instance->commands[i] = 						  Command("INVALID", 			1, invalid_exec,						NULL, NULL,				st0_result);
	}
	instance->commands[TC8566AF_CMD_READ_DATA] 			= Command("READ DATA", 			9, read_data_exec,			read_data_transfer, NULL,			chrn_result);
	instance->commands[TC8566AF_CMD_WRITE_DATA]			= Command("WRITE DATA",			9, write_data_exec, 		NULL, write_data_transfer,			chrn_result);
	instance->commands[TC8566AF_CMD_READ_DELETED_DATA]	= Command("READ DELETED DATA",	9, read_deleted_data_exec,	read_deleted_data_transfer, NULL,	chrn_result);
	instance->commands[TC8566AF_CMD_WRITE_DELETED_DATA]	= Command("WRITE DELETED DATA", 9, write_deleted_data_exec, NULL, write_deleted_data_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_READ_DIAGNOSTIC]	= Command("READ DIAGNOSTIC",	9, read_diagnostic_exec,	read_diagnostic_transfer, NULL,		chrn_result);
	instance->commands[TC8566AF_CMD_READ_ID]			= Command("READ ID",			2, read_id_exec,			read_id_transfer, NULL,				chrn_result);
	instance->commands[TC8566AF_CMD_FORMAT]				= Command("FORMAT",				6, format_exec,				NULL, format_transfer,				chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_EQUAL]			= Command("SCAN EQUAL",			9, scan_equal_exec,			NULL, scan_equal_transfer,			chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_LOW_OR_EQUAL]	= Command("SCAN LOW OR EQUAL",	9, scan_low_or_equal_exec,	NULL, scan_low_or_equal_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_HIGH_OR_EQUAL]	= Command("SCAN HIGH OR EQUAL",	9, scan_high_or_equal_exec,	NULL, scan_high_or_equal_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_SEEK]				= Command("SEEK",				3, seek_exec, 							NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_RECALIBRATE]		= Command("RECALIBRATE",		2, recalibrate_exec, 					NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_SENSE_INTERRUPT_STATUS]	= Command("SENSE INTERRUPT STATUS", 1, sense_interrupt_status_exec, NULL, NULL,				sense_interrupt_status_result);
	instance->commands[TC8566AF_CMD_SPECIFY]			= Command("SPECIFY",			3, specify_exec, 						NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_SENSE_DEVICE_STATUS]	= Command("SENSE DEVICE STATUS", 2, sense_device_status_exec, 		NULL, NULL,				sense_device_status_result);

	instance->status0 = 0;
	instance->status1 = 0;
	instance->status2 = 0;

	instance->request_for_master = 1;
	instance->non_dma_mode = 1; // MSXはDMAを使わない

	instance->_rqm_delay_count = 0;
}

void ms_disk_controller_TC8566A_deinit(THIS* instance) {

}

/**
 * @brief 
 * 
 * 0x3FF8 - レジスタ2 (コントロールレジスタ0) - ドライブ選択 (Write Only)
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
 * LED4		LED3	LED2	LED1	ENID	RST		DriveId	DriveId
 * 
 * * LED4-1 - 1にするとモーターが回転しLEDが点灯
 * * ENID - 1にすると、INT/DRQがアサートされる (使っていない)
 * * RST - 0 にすると、FDCがリセットされる (使っていない)
 * * DriveId - ドライブ番号 (0-3)
 * 
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg2_write(THIS* d, uint8_t data) {
	d->drive[3].set_motor(&d->drive[3], (data >> 7) & 0x01);
	d->drive[2].set_motor(&d->drive[2], (data >> 6) & 0x01);
	d->drive[1].set_motor(&d->drive[1], (data >> 5) & 0x01);
	d->drive[0].set_motor(&d->drive[0], (data >> 4) & 0x01);

	d->driveId = data & 0x03;
	MS_LOG(MS_LOG_DEBUG, "FDC Wr#2: %02x\n", data);
}

/**
 * @brief TC8566AFのレジスタ3に書き込む
 * 
 * 0x3FF9 - レジスタ3 (コントロールレジスタ1)
 * ステータスレジスタ
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bit-1	Bit-0
 * ENB6		C6		ENB4	C4		ENB2	SBM		ENB0	FDCTC
 * 
 * * ENB6: ENABLE C6
 * 		* 1にすると、C6の値は D6 になる
 * 		* 0にすると、C6の値は C6(bit6) になる(?)
 * * ENB4: ENABLE C4
 * 		* 1にすると、C4の値は D4 になる
 * 		* 0にすると、C4の値は C4(bit4) になる(?)
 * * ENB2: ENABLE C2
 * 		* 1にすると、C2の値は D2 になる(?)
 * * SBM: STANDBY MODE
 * 		* 1にすると、FDCはスタンバイモードになる(?)
 * * ENB0: ENABLE C0
 * 		* 1にすると、C0の値は D0 になる(?)
 * * FDCTC: FDC Terminal Count
 * 		* Non-DMAモードで1をセットすると、データ転送が中断する
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg3_write(THIS* d, uint8_t data) {
	MS_LOG( MS_LOG_DEBUG, "FDC: write reg3(control#1): %02x\n", data);
	if (data & 0x01) {
		// FDCTCが1になったら転送を中断する
		if(d->phase == TC8566AF_PHASE_DATA_TRANSFER) {
			to_result_phase(d);
		}
	}
}

/**
 * @brief TC8566AFのレジスタ4 (ステータスレジスタ) を読む
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
static uint8_t _TC8556AF_reg4_read(THIS* d) {
	// bit5の NDMは、DMAを使わない場合に、データ転送フェーズのハンドシェイクのために使われるフラグです
	// 具体的には、データ転送フェーズの最初に1にセットされ、データ転送が終わると0にリセットされます
	uint8_t ndm_req = ( (d->non_dma_mode == 1) && (d->phase == TC8566AF_PHASE_DATA_TRANSFER)) ? 1 : 0;
	uint8_t ret = (d->request_for_master << 7) | //
				(d->data_input_output << 6) | //
				(ndm_req << 5) | //
				(d->fdc_busy << 4) | //
				(d->fdd_busy & 0x3);

	// RQM(FDXのバッファが使えるかどうか)に 0 を返却したとしても、次に読んだら1になるようにする
	if( d->_rqm_delay_count > 0 ) {
		d->_rqm_delay_count--;
	} else {
		d->request_for_master = 1;	// RQM
	}

	MS_LOG(MS_LOG_TRACE, "FDC Rd#4: %02x\n", ret);
	return ret;
}

/**
 * @brief TC8566AFのレジスタ4(ステータスレジスタ)に書き込む
 * 
 * 何も起こらない
 * 
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg4_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AFのレジスタ5(データレジスタ)を読む
 * 
 * データレジスタです
 * 
 * @param d 
 * @return uint8_t 
 */
static uint8_t _TC8556AF_reg5_read(THIS* d) {
	uint8_t ret = 0;
	uint8_t phase = d->phase;
	TC8566AF_command_t* command = &d->commands[d->command];
	switch(phase) {
	case TC8566AF_PHASE_IDLE:
		MS_LOG(MS_LOG_DEBUG, "FDC(Idle): Rd#5:\n");
		break;
	case TC8566AF_PHASE_COMMAND:
		MS_LOG(MS_LOG_DEBUG, "FDC(Comm): CMD:\n %s Rd#5:\n", command->name);
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		MS_LOG(MS_LOG_TRACE, "FDC(Trns): CMD: rest=%d\n %s Rd#5:\n", d->transfer_datas_rest, command->name);
		trans_read_func_t read = command->trans_read;
		if (read == NULL) {
			MS_LOG(MS_LOG_DEBUG, "***Trans read not implemented\n");
			end_command(d);
		} else {
			uint8_t finished = 0;
			ret = read(d, &finished);
			if( finished ) {
				if( command->result_phase != NULL) {
					to_result_phase(d);
				} else {
					end_command(d);
				}
			}
		}
		MS_LOG(MS_LOG_TRACE, "  -> ret: %02x\n", ret);
		break;
	case TC8566AF_PHASE_RESULT:
		MS_LOG(MS_LOG_DEBUG, "FDC(Resl): CMD:\n %s Rd#5:\n", command->name);
		result_func_t result = command->result_phase;
		if (result == NULL) {
			MS_LOG(MS_LOG_DEBUG, "***Result not implemented\n");
			end_command(d);
		} else {
			uint8_t finished = 0;
			ret = result(d, &finished);
			d->result_byte_index++;
			if (finished) {
				end_command(d);
			}
		}
		MS_LOG(MS_LOG_DEBUG, "  -> ret: %02x\n", ret);
		d->request_for_master = 0;	// RQMを一度0にするが、ステータスを読むと1に戻るようになっている
		break;	
	}
	return ret;
}

/**
 * @brief TC8566AFのレジスタ5(データレジスタ)に書き込む
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
 * ## 2. Command Phase
 * コマンドフェーズでは、コマンドに必要なパラメータを書き込みます。コマンドの種類によって何回か繰り返しwrite）
 * 
 * ## 3. Data Transfer Phase
 * データ読み込み or 書き込みを行うフェーズです
 *
 * ## 4. Result Phase
 * 結果をホスト側に返却するフェーズです
 * 
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg5_write(THIS* d, uint8_t value) {
	TC8566AF_command_t* command;

	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		uint8_t cmd = 0;
		if ((value & 0x1f) == 0x06) {
			cmd = TC8566AF_CMD_READ_DATA;
		} else if ((value & 0x3f) == 0x05) {
			cmd = TC8566AF_CMD_WRITE_DATA;
		} else if ((value & 0x3f) == 0x09) {
			cmd = TC8566AF_CMD_WRITE_DELETED_DATA;
		} else if ((value & 0x1f) == 0x0c) {
			cmd = TC8566AF_CMD_READ_DELETED_DATA;
		} else if ((value & 0xbf) == 0x02) {
			cmd = TC8566AF_CMD_READ_DIAGNOSTIC;
		} else if ((value & 0xbf) == 0x0a) {
			cmd = TC8566AF_CMD_READ_ID;
		} else if ((value & 0xbf) == 0x0d) {
			cmd = TC8566AF_CMD_FORMAT;
		} else if ((value & 0x1f) == 0x11) {
			cmd = TC8566AF_CMD_SCAN_EQUAL;
		} else if ((value & 0x1f) == 0x19) {
			cmd = TC8566AF_CMD_SCAN_LOW_OR_EQUAL;
		} else if ((value & 0x1f) == 0x1d) {
			cmd = TC8566AF_CMD_SCAN_HIGH_OR_EQUAL;
		} else if ((value & 0xff) == 0x0f) {
			cmd = TC8566AF_CMD_SEEK;
		} else if ((value & 0xff) == 0x07) {
			cmd = TC8566AF_CMD_RECALIBRATE;
		} else if ((value & 0xff) == 0x08) {
			cmd = TC8566AF_CMD_SENSE_INTERRUPT_STATUS;
		} else if ((value & 0xff) == 0x03) {
			cmd = TC8566AF_CMD_SPECIFY;
		} else if ((value & 0xff) == 0x04) {
			cmd = TC8566AF_CMD_SENSE_DEVICE_STATUS;
		}
		d->fdc_busy = 1;
		d->command = cmd;
		d->command_byte_value[0] = cmd;
		d->command_byte_index = 1;

		command = &d->commands[cmd]; // cmd = 0 なら INVALID が入る
		MS_LOG(MS_LOG_DEBUG, "FDC(Idle): Accept\n %s Wr#5: %02x\n", command->name, value);
		if( command->command_byte_count == 1) {
			d->request_for_master = 0;
			execute(d);
		} else {
			d->phase = TC8566AF_PHASE_COMMAND;
		}
		break;
	case TC8566AF_PHASE_COMMAND:
		command = &d->commands[d->command];
		MS_LOG(MS_LOG_DEBUG, "FDC(Comm): Wr#5:\n %s[%d] = %02x\n", command->name, d->command_byte_index, value);
		d->command_byte_value[d->command_byte_index++] = value;
		if (d->command_byte_index == command->command_byte_count) {
			execute(d);
		}
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		command = &d->commands[d->command];
		MS_LOG(MS_LOG_DEBUG, "FDC(Trns): %s Wr#5: %02x\n", command->name, value);
		if( command->trans_write == NULL) {
			MS_LOG(MS_LOG_DEBUG, "invalid write. igonre\n");
		} else {
			uint8_t finished = 0;
			command->trans_write(d, value, &finished);
			if( finished) {
				if( command->result_phase != NULL) {
					to_result_phase(d);
				} else {
					end_command(d);
				}
			}
		}
		break;
	case TC8566AF_PHASE_RESULT:
		command = &d->commands[d->command];
		MS_LOG(MS_LOG_DEBUG, "FDC(Rslt): %s Wr#5: %02x\n", command->name, value);
		MS_LOG(MS_LOG_DEBUG, "invalid result. ignore\n");  // result phase は read only
		break;		
	}
}

void end_command(THIS* d) {
	d->fdc_busy = 0;			// CB
	d->data_input_output = 0;	// DIO
	d->request_for_master = 1;	// RQM
	d->phase = TC8566AF_PHASE_IDLE;
}

void execute(THIS* d) {
	TC8566AF_command_t* command = &d->commands[d->command];
	exec_func_t exec = command->execution_phase;
	exec(d);	// 実行

	if( command->trans_read != NULL) {
		d->phase = TC8566AF_PHASE_DATA_TRANSFER;
		d->data_input_output = 1;  // FDC -> Host
	} else if (command->trans_write != NULL) {
		d->phase = TC8566AF_PHASE_DATA_TRANSFER;
		d->data_input_output = 0;  // Host -> FDC
	} else if (command->result_phase != NULL) {
		to_result_phase(d);
	} else {
		end_command(d);
	}
}


void decode_cmd_chrn(THIS* d) {
	d->value_HS = (d->command_byte_value[1] & 0x04) >> 2;
	d->value_DS = (d->command_byte_value[1] & 0x03);
	d->value_C = d->command_byte_value[2];
	d->value_H = d->command_byte_value[3];
	d->value_R = d->command_byte_value[4];
	d->value_N = d->command_byte_value[5];
	d->value_EOT = d->command_byte_value[6];
	d->value_GPL = d->command_byte_value[7];
	d->value_DTL_STP = d->command_byte_value[8];

	// status 0を更新
	d->status0 &= 0b00111100; // Interrupt code (b7-6)と Drive select (b1-0) をクリア
	d->status0 |= (d->value_DS);
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	if (drive->container == NULL) {
		// ドライブが存在しない
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	}

	// status 3を更新
	uint8_t is_disk_inserted = drive->is_disk_inserted(drive);
	d->status3 = //
		(0 * 0x80); // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(d->value_HS * 0x04) | // Head select
		(d->value_DS); // Drive select

	// ドライブのヘッド(サイド)を選択
	drive->set_side(drive, d->value_HS);
}

/* *********************************************************

	READ DATA

	READ DATA コマンドは、9バイトからなり、以下の情報が与えられます。

	* 0バイト目: b7:MT, b6:MFM, b5:SK、b4-0:コマンド(0x06)
		* MT: Multi-Track、複数トラックを読むかどうか
		* MFM: MFM、MFMでデータを読むかどうか
		* SK: Skip、DDAM(Deleted Data Address Mark)のあるセクタ(削除されたセクタ)をスキップするかどうか
	* 1バイト目: HS, DS1, DS0
		* HS: ヘッドセレクト、FDの裏表のどちらのサイドかを指定します
		* DS: 2ビットでドライブ番号を指定します
	* 2バイト目: Cylinder
	* 3バイト目: Head
	* 4バイト目: Record (ヘッドないのセクタ番号)
	* 5バイト目: N (セクタサイズの指定、下記表参照)
		* N=0: 128バイト
		* N=1: 256バイト
		* N=2: 512バイト
		* N=3: 1024バイト
	* 6バイト目: EOT (End of Track)、トラック内のどのセクタまで読むか
	* 7バイト目: GPL (Gap Length)
	* 8バイト目: DTL (Data Length)

  MT, MFM SKは無視しています (MT=1)

 READ DATA コマンドの実行は、以下の手順で行います。
 * Step1: ヘッドのロード
	* ヘッドがロードされていない場合はロードします
 * Step2: ID Address Markと ID field の読み込み
 	* ID Address Markというのは、IDフィールドが始まることを示す特殊なデータ(0xa1a1a1fe) です
	* ID fieldというのは CHRN の情報を含むデータのことです
 * Step3: データの読み込み(転送)
 	* コマンドに指定された CHRNと、ID fieldのCHRNが一致するセクタが現れるまで待ち、発見されたらデータを読み込み、転送します
	* もしヘッドのある位置と異なるCHRNを指定して READ DATAコマンドを発行した場合は、データが見つからないので、Not Found エラーを返します(おそらく)
 * Step4: データの転送終了条件成立まで繰り返す
 	* 1つセクターを読んだら、セクター番号(R)をインクリメントして次のセクターを読み込みます
	* これを Multi-sector Read Operation といいます
	* MT=1の場合は裏面まで読みに行きますが、MT=0の場合は同一トラック内のみしか読みません
	* EOTで指定されたセクタ番号に達するか、最大最終セクタに達したら転送を終了します
	* あるいは、reg3 (コントロールレジスタ1)の Bit0 (FDCTC) に1がセットされ他場合もそこで転送を終了します。
	* 最大最終セクタは、MT=0の場合は同一トラック内の最大セクタ、MT=1の場合は裏面の最大セクタで、以下のようになります
		* MT=0でサイド0を読んだ場合の最大最終セクタ	= サイド0, セクタ 9, 最大転送セクタ数=9	
		* MT=0でサイド1を読んだ場合の最大最終セクタ	= サイド1, セクタ 9, 最大転送セクタ数=9
		* MT=1でを読んだ場合の最大最終セクタ		= サイド1, セクタ 9, 最大転送セクタ数=18
* Step5: 転送が終わったらリザルトフェーズで結果を返します

読み込み中にCRCエラーが発生した場合などは、result status0,1,2に以下のビットを立てます
 * status0: 0b01000000 : IC(Interrupt Code) = 01 = コマンド異常終了
 * status1: 0b00100000 : DE(Data Error)
 * status2: 0b00100000 : DD(Data Error in Data Field)
 
********************************************************* */

static uint8_t read_data_setup(THIS* d);

static void read_data_exec(THIS* d) {
	MS_LOG( MS_LOG_DEBUG, "FDC READ DATA\n");
	decode_cmd_chrn(d);
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];

	if ( d->value_R > d->value_EOT) {
		// 指定されたセクタ番号がEOTを超えている場合、0バイト
		d->transfer_datas_rest = 0;
		return;
	}
	// 実際のFDCの動作としては、ヘッドをロードし、回転するディスクの中から
	// CHRNに合致するセクタを見つけ、そのトラックのデータを最後まで読み込みます
	// プロテクトの再現をする場合はそのタイミングなども合わせる必要がありますが、
	// ここでは単にデータを読み込むだけとします

	// ディスクを最大2周させ、該当のセクタを探しバッファする
	d->sector_buffer_ready = 0;
	d->sector_buffer_serach_count = 0;

	if(!read_data_setup(d)) {
		// 読み込み失敗
		d->transfer_datas_rest = 0;
		to_result_phase(d);
	} else {
		d->transfer_datas_rest = (128 << d->value_N);
		MS_LOG(MS_LOG_DEBUG, "FDC: READ DATA Trns rest=%d\n", d->transfer_datas_rest);
	}
}

static uint8_t read_data_setup(THIS* d) {
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	for(;d->sector_buffer_serach_count<9*2; d->sector_buffer_serach_count++) {
		if( !drive->get_next_sector(drive, &d->sector_buffer) ) {
			// 読み込み失敗
			return 0;
		}
		if (d->sector_buffer.track == d->value_C && d->sector_buffer.head == d->value_H && d->sector_buffer.sector == d->value_R) {
			// 発見
			d->sector_buffer_ready = 1;
			d->sector_buffer_byte_offset = 0;
			return 1;
			// // 最後まで達したか？
			// if( d->value_R > d->value_EOT) {
			// 	return 1;
			// }
		} else {
			// 異なる場合はスキップ
			MS_LOG(MS_LOG_DEBUG, "Skip: %d %d %d (expect: %d %d %d)\n", //
			 d->sector_buffer.track, d->sector_buffer.head, d->sector_buffer.sector, //
			 d->value_C, d->value_H, d->value_R);
		}
	}
	return 0;
}

static uint8_t read_data_transfer(THIS* d, uint8_t* finished) {
	if(d->sector_buffer_byte_offset == 0) {
		MS_LOG( MS_LOG_DEBUG, "FDC READ: C:%d H:%d R:%d\n", d->sector_buffer.track, d->sector_buffer.head, d->sector_buffer.sector);
	}

	uint8_t ret = 0;
	d->transfer_datas_rest -= 1;
	if( d->transfer_datas_rest <= 0) {
		MS_LOG(MS_LOG_DEBUG, "FDC: READ DATA Trns rest=%d\n", d->transfer_datas_rest);
		to_result_phase(d);
	}
	// read data from disk
	ret = d->sector_buffer.data[d->sector_buffer_byte_offset++];
	if ( d->sector_buffer_byte_offset == 512 ) {
		// 1セクタ分読み込み終わったので次のセクタを探す
		d->value_R++;
		if( !read_data_setup(d)) {
			// 読み込み失敗 or 最後まで達した
			d->transfer_datas_rest = 0;
			to_result_phase(d);
		}
		d->sector_buffer_byte_offset = 0;
		d->_rqm_delay_count = 10;	// RQMが立つまでの遅延を長めに設定
	} else {
		d->_rqm_delay_count = 1;	// RQMが立つまでの遅延を設定
	}
	d->request_for_master = 0;	// RQMを一度0にするが、ステータスを読むと1に戻るようになっている
	return ret;
}

void to_result_phase(THIS* d) {
	d->data_input_output = 1;  // FDC -> Host
	d->request_for_master = 1; // RQM
	d->result_byte_index = 0;
	d->phase = TC8566AF_PHASE_RESULT;
}

/* *********************************************************

	WRITE DATA

********************************************************* */
static void write_data_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "write_data_exec not implemented\n");
}

static void write_data_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/* *********************************************************

	READ DELETED DATA

********************************************************* */
static void read_deleted_data_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "read_deleted_data_exec not implemented\n");
}

static uint8_t read_deleted_data_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	WRITE DELETED DATA

********************************************************* */
static void write_deleted_data_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "write_deleted_data_exec not implemented\n");
}

static void write_deleted_data_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	READ DIAGNOSTIC

********************************************************* */
static void read_diagnostic_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "read_diagnostic_exec not implemented\n");
}

static uint8_t read_diagnostic_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	READ ID

********************************************************* */
static void read_id_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "read_id_exec not implemented\n");
}

static uint8_t read_id_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	FORMAT

********************************************************* */
static void format_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "format_exec not implemented\n");
}

static void format_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN EQUAL

********************************************************* */
static void scan_equal_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "scan_equal_exec not implemented\n");
}

static void scan_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN LOW OR EQUAL

********************************************************* */
static void scan_low_or_equal_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "scan_low_or_equal_exec not implemented\n");
}

static void scan_low_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN HIGH OR EQUAL

********************************************************* */
static void scan_high_or_equal_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "scan_high_or_equal_exec not implemented\n");
}

static void scan_high_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SEEK
 			 Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
  	CMD[0]	0		0		0		0		1		1		1		1
  	CMD[1]	X		X		X		X		X		X		US1		US0
	CMD[2]	NCN7	NCN6	NCN5	NCN4	NCN3	NCN2	NCN1	NCN0

	* US1, US0: ユニット選択
	* NCN7-0: シーク先のシリンダ番号

********************************************************* */
static void seek_exec(THIS* d) {
	// status 0
	// b7-6	: IC(Interrupt Code) = 00 = 正常終了
	// b5	: Seek End
	// b4	: EC (Equipment Check)
	// b3	: Not Ready
	// b2	: Head Address
	// b1-0	: Drive select
	d->status0 = 0;

	uint8_t HS = (d->command_byte_value[1] & 0x04) >> 2;
	uint8_t DS = d->command_byte_value[1] & 0x03;
	uint8_t ncn = d->command_byte_value[2];

	ms_disk_drive_floppy_t* drive = &d->drive[DS];
	if (drive->container == NULL) {
		// ドライブが存在しない
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	} else {
		// シーク実行
		drive->seek(drive, ncn);
	}

	// result
	d->status0 |= 0x20; 		// Seek End
	d->status0 |= HS << 2;		// Head Address
	d->status0 |= DS;		// 今回シークしたドライブをセット

	uint8_t is_disk_inserted = drive->is_disk_inserted(drive);
	d->status3 = //
		(0 * 0x80) | // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(HS * 0x04) |	 // Head select
		(DS);			 // 今回シークしたドライブをセット
}

/*********************************************************

	RECALIBRATE

 			 Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
  	CMD[0]	0		0		0		0		0		1		1		1
  	CMD[1]	X		X		X		X		X		X		US1		US0

********************************************************* */
static void recalibrate_exec(THIS* d) {
	// status 0
	// b7-6	: IC(Interrupt Code) = 00 = 正常終了
	// b5	: Seek End
	// b4	: EC (Equipment Check)
	// b3	: Not Ready
	// b2	: Head Address
	// b1-0	: Drive select
	d->status0 = 0;

	uint8_t HS = (d->command_byte_value[1] & 0x04) >> 2;
	uint8_t DS = d->command_byte_value[1] & 0x03;

	ms_disk_drive_floppy_t* drive = &d->drive[DS];
	if (drive->container == NULL) {
		// ドライブが存在しない
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	} else {
		// Track0までシーク実行
		drive->seek(drive, 0);
	}

	// result
	d->status0 |= 0x20; 		// Seek End
	d->status0 |= HS << 2;		// Head Address
	d->status0 |= DS;		// 今回シークしたドライブをセット

	uint8_t is_disk_inserted = drive->is_disk_inserted(drive);
	d->status3 = //
		(0 * 0x80) | // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(HS * 0x04) |	 // Head select
		(DS);			 // 今回シークしたドライブをセット
}

/*********************************************************

	SENSE INTERRUPT STATUS

********************************************************* */
static void sense_interrupt_status_exec(THIS* d) {
	// 何もすることはないので空でOK
	MS_LOG( MS_LOG_DEBUG, "FDC: SENSE INTERRUPT STATUS\n");
}

/*********************************************************

	SPECIFY

********************************************************* */
static void specify_exec(THIS* d) {
	// 何もすることはないので空でOK
	uint8_t SRT = (d->command_byte_value[1] & 0xf0) >> 4;
	uint8_t HUT = (d->command_byte_value[1] & 0x0f);
	uint8_t HLT = (d->command_byte_value[2] & 0xfe) >> 1;
	uint8_t ND = (d->command_byte_value[2] & 0x01);
	MS_LOG( MS_LOG_DEBUG, "FDC: SPECIFY\n SRT=%d, HUT=%d, HLT=%d, ND=%d\n", SRT, HUT, HLT, ND);
}

/*********************************************************

	SENSE DEVICE STATUS

********************************************************* */
static void sense_device_status_exec(THIS* d) {
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	if (drive->container == NULL) {
		// ドライブが存在しない
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	}

	uint8_t is_disk_inserted = drive->is_disk_inserted(drive);
	d->status3 = //
		(0 * 0x80) | // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(d->value_HS * 0x04) | // Head select
		(d->value_DS); // Drive select
}

/*********************************************************

	INVALID

********************************************************* */
static void invalid_exec(THIS* d) {
	MS_LOG( MS_LOG_INFO, "invalid command\n");
}


/* *********************************************************

	RESULT

********************************************************* */

/**
 * @brief CHRNの結果を返すresult phaseです
 * 
 * @param d 
 * @param finished 
 * @return uint8_t 
 */
static uint8_t chrn_result(THIS* d, uint8_t* finished) {

	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		TC8566AF_command_t* command = &d->commands[d->command];
		MS_LOG (MS_LOG_DEBUG, "CHRN result: C:%d H:%d R:%d N:%d (%s)\n", d->value_C, d->value_H, d->value_R, d->value_N, command->name);
		ret = d->status0;
		break;
	case 1:
		ret = d->status1;
		break;
	case 2:
		ret = d->status2;
		break;
	case 3:
		ret = d->value_C;
		break;
	case 4:
		ret = d->value_H;
		break;
	case 5:
		ret = d->value_R;
		break;
	case 6:
		ret = d->value_N;
		*finished = 1;
		break;
	default:
		MS_LOG (MS_LOG_DEBUG, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}

static uint8_t sense_interrupt_status_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status0;
		d->status0 = 0;
		break;
	case 1:
		ret = 0; // TODO
		*finished = 1;
		break;
	default:
		MS_LOG( MS_LOG_DEBUG, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}

static uint8_t sense_device_status_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status3;
		*finished = 1;
		break;
	default:
		MS_LOG( MS_LOG_DEBUG, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;		
}

static uint8_t st0_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status0;
		*finished = 1;
		break;
	default:
		MS_LOG( MS_LOG_DEBUG, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}
