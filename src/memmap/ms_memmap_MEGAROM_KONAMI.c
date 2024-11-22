#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MEGAROM_KONAMI.h"
#include "../ms.h"

#define THIS ms_memmap_driver_MEGAROM_KONAMI_t

static char* driver_name = "MEGAROM_KONAMI";

static void _select_bank(THIS* d, int rom_page, int bank);

static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

/*
	�m�ۃ��[�`��
 */
THIS* ms_memmap_MEGAROM_KONAMI_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_MEGAROM_KONAMI_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_MEGAROM_KONAMI;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MEGAROM_KONAMI_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->base.buffer_length = length;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k = 2; page8k < 6; page8k++) {
		_select_bank(instance, page8k, page8k-2);	// KONAMI���K�����̏ꍇ�A�����l��0,1,2,3
	}
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

static int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static void _select_bank(THIS* d, int page8k, int segment) {
	if ( segment >= d->num_segments) {
		printf("MEGAROM_KONAMI: segment out of range: %d\n", segment);
		return;
	}
	d->base.page8k_pointers[page8k] = d->base.buffer + (segment * 0x2000);
	d->selected_segment[page8k] = segment;

	// �؂�ւ����N���������Ƃ� memmap �ɒʒm
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page8k);
	return;
}


static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"MEGAROM_KONAMI: read out of range: %04x\n", addr);
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
	SCC������KONAMI���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#Konami_MegaROMs_without_SCC

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* �؂�ւ��A�h���X:	None
		* �����Z�O�����g	Always 0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* �؂�ւ��A�h���X	6000h (mirrors: 6001h~7FFFh)
		* �����Z�O�����g	1
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* �؂�ւ��A�h���X	8000h (mirrors: 8001h~9FFFh)
		* �����Z�O�����g	Random
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* �؂�ւ��A�h���X	A000h (mirrors: A001h~BFFFh)
		* �����Z�O�����g	Random
 */
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	// �o���N�؂�ւ�����
	int page8k = -1;
	int area = addr >> 12;
	switch(area) {
		case 0x4:
		case 0x5:
			break;
		case 0x6:
		case 0x7:
			// 0x6000-0x7fff
			page8k = 3;
			break;
		case 0x8:
		case 0x9:
			// 0x8000-0x9fff
			page8k = 4;
			break;
		case 0xa:
		case 0xb:
			// 0xa000-0xbfff
			page8k = 5;
			break;
	}
	if (page8k != -1) {
		_select_bank(d, page8k, data);
	}
	return;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}
