/*
	PAC (Pana Amusement Pack�APanasoft SW-M001) �̃h���C�o

	PAC�� SRAM�A�N�Z�X���@ (�\�t�g�����猩������)

	* 1: 5FFEh�A5FFFh�� FFh �ł��邱�Ƃ��m�F����
	* 2: 5FFEh�� 4Dh ����������ŁA�ǂݏo���āA4Dh �ł��邱�Ƃ��m�F����
		* �� �قȂ�l���ǂ߂���A1�œǂݍ��� 5FFEh�̒l�������߂��ďI��
	* 3: 5FFFh�� 69h ����������ŁA�ǂݏo���āA69h �ł��邱�Ƃ��m�F����
		* �� �قȂ�l���ǂ߂���A1�œǂݍ��� 5FFEh�̒l�������߂��ďI��
	* 4: 4000h-5FFDh �͈̔͂�SRAM�̈�Ƃ��ė��p�\�ɂȂ�
	* 5: 5FFEh, 5FFFh �� 00h ����������ŏI�� (4Dh, 69h �ȊO���������܂���OK)
	* 6: �I������ƁA5FFEh, 5FFFh ���� FFh ���ǂ߂�悤�ɂȂ�

	�� FM-PAC��SRAM�o�b�N�A�b�v�t�@�C���̎d�l
	FM-PAC�ɂ̓����e�i���X�c�[����ROM�ɓ�������Ă���ASRAM�f�[�^���t�@�C���Ƀo�b�N�A�b�v���邱�Ƃ��ł���B
	* �g���q�� .PAC
	* �t�@�C���T�C�Y�� 8206 �o�C�g
		* 16�o�C�g�w�b�_: "PAC2 BACKUP DATA"
		* 1024�o�C�g��SRAM�f�[�^ x 7
		* 1022�o�C�g��SRAM�f�[�^ x 1
	

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include "../ms.h"
#include "ms_memmap.h"
#include "ms_memmap_PAC.h"

#define THIS ms_memmap_driver_PAC_t

static char* driver_name = "PAC";

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
THIS* ms_memmap_PAC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_PAC_init(ms_memmap_driver_PAC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t buf_length, uint32_t file_length, uint8_t* file_path) {
	if (instance == NULL) {
		return;
	}

	if (file_length != 8206) {
		MS_LOG(MS_LOG_ERROR,"PAC�̃t�@�C���T�C�Y�� 8206�o�C�g�ł���K�v������܂�\n");
		return;
	}
	// buffer�̐擪16�o�C�g�� "PAC2 BACKUP DATA" �ł��邱�Ƃ��m�F
	if ( memcmp(buffer, "PAC2 BACKUP DATA", 16UL) != 0 ) {
		MS_LOG(MS_LOG_ERROR,"PAC�̃t�@�C���w�b�_���s���ł�\n");
		return;
	}

	// �f�[�^����16�o�C�g�̃w�b�_+8192-2�o�C�g=8206�o�C�g�����A�������m�ۂ�8K�P�ʂōs���Ă���̂�
	// buffer��16�o�C�g+8192�o�C�g�ł���Ƃ��Ĉ����Ă����Ȃ��͂�
	if (buf_length < 16+8192) {
		MS_LOG(MS_LOG_ERROR,"PAC�̃o�b�t�@�T�C�Y���s���ł�\n");
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.type = ROM_TYPE_MIRRORED_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_PAC_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_pause = _did_pause;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	// �v���C�x�[�g�v���p�e�B
	memcpy(instance->file_path, file_path, 256);
	instance->sram_enabled = 0;
	instance->_5ffe = 0xff;
	instance->_5fff = 0xff;
	int i;
	for(i=0;i<8192;i++) {
		instance->disabled_buffer[i] = 0xff;
	}

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->disabled_buffer;
	}
	
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

/**
 * @brief �h���C�o�̃f�^�b�`���ɌĂяo�����R�[���o�b�N�ŁAPAC��SRAM���t�@�C���ɏ����o���܂�
 * 
 * @param driver 
 * @return int 
 */
static int _will_detach(ms_memmap_driver_t* driver) {
	MS_LOG(MS_LOG_INFO, "PAC��SRAM���t�@�C���ɏ����o���܂�\n");
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
		printf("PAC��SRAM���t�@�C���ɏ����o���܂���\n");
	}
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	if(local_addr == 0x5ffe) {
		return d->_5ffe;
	} else if(local_addr == 0x5fff) {
		return d->_5fff;
	}
	uint8_t ret = driver->page8k_pointers[page8k][local_addr];
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	if(addr == 0x5ffe) {
		d->_5ffe = data;
	} else if(addr == 0x5fff) {
		d->_5fff = data;
	} else if (page8k == 2 && d->sram_enabled) {
		// SRAM���L���Ȏ��� page8k == 2 �ɂ� SRAM���}�b�s���O����Ă���
		driver->page8k_pointers[2][local_addr] = data;
	}
	if(d->_5ffe == 0x4d && d->_5fff == 0x69) {
		if (d->sram_enabled == 0) {
			MS_LOG(MS_LOG_INFO,"PAC��SRAM���J����܂���\n");
			d->sram_enabled = 1;
			driver->page8k_pointers[2] = driver->buffer+16;
			// �؂�ւ����N���������Ƃ� memmap �ɒʒm
			d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, 2);
		}
	} else {
		if (d->sram_enabled != 0) {
			MS_LOG(MS_LOG_INFO,"PAC��SRAM������ꂽ�̂ŁA�t�@�C���ɏ����o���܂�\n");
			d->sram_enabled = 0;
			driver->page8k_pointers[2] = d->disabled_buffer;
			_fflush(driver);
			// �؂�ւ����N���������Ƃ� memmap �ɒʒm
			d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, 2);
		}
	}
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

/**
 * @brief PAC��SRAM���t�@�C���ɏ����o���܂�
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
		MS_LOG(MS_LOG_ERROR,"PAC�t�@�C�����J���܂���. %s\n", instance->file_path);
		return;
	}

	write(crt_fh, instance->base.buffer, 16+8192-2);

	close(crt_fh);
}
