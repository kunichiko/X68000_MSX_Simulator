#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include "ms_memmap.h"
#include "ms_memmap_ESE_RAM.h"
#include "../ms.h"

#define THIS ms_memmap_driver_ESE_RAM_t

static char* driver_name = "ESE_RAM";

static void _select_bank(THIS* d, int region, int segment);
static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);
static void _did_pause(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

static void _fflush(ms_memmap_driver_t* driver);

/*
	�m�ۃ��[�`��
 */
THIS* ms_memmap_ESE_RAM_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_ESE_RAM_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.type = ROM_TYPE_ESE_RAM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_ESE_RAM_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_pause = _did_pause;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	instance->base.buffer = (uint8_t*)buffer;
	instance->base.buffer_length = length;
	instance->num_segments = length / 0x2000;

	// �v���C�x�[�g�v���p�e�B
	memcpy(instance->file_path, file_path, 256);

	int region;
	for(region = 0; region < 4; region++) {
		_select_bank(instance, region, 0);	// ASCII 8K���K�����̏ꍇ�A�����l��0
	}
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

/**
 * @brief �h���C�o�̃f�^�b�`���ɌĂяo�����R�[���o�b�N�ŁASRAM���t�@�C���ɏ����o���܂�
 * 
 * @param driver 
 * @return int 
 */
static int _will_detach(ms_memmap_driver_t* driver) {
	MS_LOG(MS_LOG_INFO, "ESE-RAM��SRAM���t�@�C���ɏ����o���܂�\n");
	_fflush(driver);
	return 0;
}

/**
 * @brief �|�[�Y���ɌĂяo�����R�[���o�b�N
 * 
 * @param driver 
 */
static void _did_pause(ms_memmap_driver_t* driver) {
	volatile uint8_t* BITSNS_WORK = (uint8_t*)0x800;
	if (BITSNS_WORK[0xe] & 1) {
		// �|�[�Y���� SHIFT�L�[��������Ă�����ASRAM���t�@�C���ɏ����o��
		_fflush(driver);
		printf("ESE-RAM��SRAM���t�@�C���ɏ����o���܂���\n");
	}
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

/**
 * @brief region (0-3)�Ɍ�����o���N��I�����܂�
 * 
 * @param d 
 * @param region 
 * @param segment 
 */
static void _select_bank(THIS* d, int region, int segment) {
	uint8_t* buf;
	uint8_t segment_masked = segment & 0x7f;
	if ( segment_masked >= d->num_segments) {
		MS_LOG(MS_LOG_FINE,"ESE_RAM: segment out of range: %d (%02x)\n", segment_masked, segment);
		buf = NULL;
	} else {
		buf = d->base.buffer + (segment_masked * 0x2000);
	}
	//MS_LOG(MS_LOG_INFO,"ESE_RAM: select bank [%0d] = %0d\n", region, segment);

	d->selected_segment[region] = segment;
	d->base.page8k_pointers[(region+2)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+2)&0x7);	// �؂�ւ����N���������Ƃ� memmap �ɒʒm
	d->base.page8k_pointers[(region+6)&0x7] = NULL;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+6)&0x7);	// �؂�ւ����N���������Ƃ� memmap �ɒʒm

	return;
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"ESE_RAM: read out of range: %04x\n", addr);
		return 0xff;
	}

	uint8_t ret = p8k[local_addr];
	return ret;
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

/*
	ASCII 8K���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#ASCII_8K

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* �؂�ւ��A�h���X:	6000h (mirrors: 6001h~67FFh)
		* �����Z�O�����g	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* �؂�ւ��A�h���X	6800h (mirrors: 6801h~6FFFh)
		* �����Z�O�����g	0
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* �؂�ւ��A�h���X	7000h (mirrors: 7001h~77FFh)
		* �����Z�O�����g	0
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* �؂�ւ��A�h���X	7800h (mirrors: 7801h~7FFFh)
		* �����Z�O�����g	0
 */
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;

	int area = addr >> 11;
	switch(area) {
		case 0x6*2+0:
			_select_bank(d, 0, data);
			break;
		case 0x6*2+1:
			_select_bank(d, 1, data);
			break;
		case 0x7*2+0:
			_select_bank(d, 2, data);
			break;
		case 0x7*2+1:
			_select_bank(d, 3, data);
			break;
		default:
			// ESE-RAM�� 0x80?0xFF��I������ƁA���������\�ɂȂ�
			MS_LOG(MS_LOG_DEBUG,"ESE_RAM: wr %04x<=%02x", addr, data);
			int page8k = addr >> 13;
			int local_addr = addr & 0x1fff;
			int region = (page8k - 2) & 0x03;
			if( (d->selected_segment[region] & 0x80) != 0) {
				MS_LOG(MS_LOG_DEBUG," OK\n");
				// ���������\
				uint8_t* p8k = driver->page8k_pointers[page8k];
				if( p8k == NULL ) {
					MS_LOG(MS_LOG_FINE,"ESE_RAM: write out of range: %04x\n", addr);
					return;
				}
				p8k[local_addr] = data;
			} else {
				MS_LOG(MS_LOG_DEBUG," NG\n");
			}
			break;
	}

	return;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

/**
 * @brief ESE-RAM��SRAM���t�@�C���ɏ����o���܂�
 * 
 * @param instance 
 */
void _fflush(ms_memmap_driver_t* driver) {
	THIS* instance = (THIS*)driver;
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( instance->file_path, O_RDWR | O_BINARY);
	if (crt_fh == -1) {
		MS_LOG(MS_LOG_ERROR,"SRAM�t�@�C�����J���܂���. %s\n", instance->file_path);
		return;
	}

	write(crt_fh, instance->base.buffer, instance->base.buffer_length);

	close(crt_fh);
}
