#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"

char* driver_name_MEGAROM_GENERIC_8K = "MEGAROM_GENERIC_8K";

void _select_bank_generic_8K(ms_memmap_driver_MEGAROM_GENERIC_8K_t* d, int page, int bank);

/*
	�m�� & ���������[�`��
 */
ms_memmap_driver_t* ms_memmap_MEGAROM_GENERIC_8K_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* instance;
	instance = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)new_malloc(sizeof(ms_memmap_driver_MEGAROM_GENERIC_8K_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.type = ROM_TYPE_MEGAROM_ASCII_8K;
	instance->base.name = driver_name_MEGAROM_GENERIC_8K;
	instance->base.deinit = ms_memmap_deinit_MEGAROM_GENERIC_8K;
	instance->base.did_attach = ms_memmap_did_attach_MEGAROM_GENERIC_8K;
	instance->base.will_detach = ms_memmap_will_detach_MEGAROM_GENERIC_8K;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_MEGAROM_GENERIC_8K;
	instance->base.read8 = ms_memmap_read8_MEGAROM_GENERIC_8K;
	instance->base.read16 = ms_memmap_read16_MEGAROM_GENERIC_8K;
	instance->base.write8 = ms_memmap_write8_MEGAROM_GENERIC_8K;
	instance->base.write16 = ms_memmap_write16_MEGAROM_GENERIC_8K;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k = 2; page8k < 6; page8k++) {
		_select_bank_generic_8K(instance, page8k, page8k-2);	// GENERIC 8K���K�����̏ꍇ�A�����l��0,1,2,3
	}
	return (ms_memmap_driver_t*)instance;
}

void ms_memmap_deinit_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* d = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)driver;
	new_free(d->base.buffer);
	new_free(d);
}

void ms_memmap_did_attach_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

void _select_bank_generic_8K(ms_memmap_driver_MEGAROM_GENERIC_8K_t* d, int page8k, int segment) {
	if ( segment >= d->num_segments) {
		printf("MEGAROM_GENERIC_8K: segment out of range: %d\n", segment);
		return;
	}
	d->base.page8k_pointers[page8k] = d->base.buffer + (segment * 0x2000);
	d->selected_segment[page8k] = segment;

	// �؂�ւ����N���������Ƃ� memmap �ɒʒm
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page8k);
	return;
}


uint8_t ms_memmap_read8_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* d = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)driver;
	int page8k = addr >> 13;
	if( page8k < 2 || page8k > 5) {
		printf("MEGAROM_GENERIC_8K: read out of range: %04x\n", addr);
		return 0xff;
	}
	int segment = d->selected_segment[page8k];
	if (segment >= d->num_segments) {
		printf("MEGAROM_GENERIC_8K: segment out of range: %d\n", segment);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * segment);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MEGAROM_GENERIC_8K: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

uint16_t ms_memmap_read16_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* d = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)driver;
	return ms_memmap_read8_MEGAROM_GENERIC_8K(driver, addr) | (ms_memmap_read8_MEGAROM_GENERIC_8K(driver, addr + 1) << 8);
}

/*
	GENERIC 8K���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#Generic_8K

	* 4000h~5FFFh
		* �؂�ւ��A�h���X:	4000h (mirrors: 4001h~47FFh, 5000h-57ff)
		* �����Z�O�����g	0
	* 6000h~7FFFh
		* �؂�ւ��A�h���X	6000h (mirrors: 6001h~67FFh, 7000h-77ffh)
		* �����Z�O�����g	0
	* 8000h~9FFFh
		* �؂�ւ��A�h���X	8000h (mirrors: 8001h~87FFh, 9000h-97ffh)
		* �����Z�O�����g	0
	* A000h~BFFFh
		* �؂�ւ��A�h���X	a000h (mirrors: a001h~a7FFh, b000h-b7ffh)
		* �����Z�O�����g	0
 */
void ms_memmap_write8_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* d = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)driver;
	// �o���N�؂�ւ�����
	int page8k = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x4*2+0:
		case 0x5*2+0:
			page8k = 2;
			break;
		case 0x6*2+0:
		case 0x7*2+0:
			page8k = 3;
			break;
		case 0x8*2+0:
		case 0x9*2+0:
			page8k = 4;
			break;
		case 0xa*2+0:
		case 0xb*2+0:
			page8k = 5;
			break;
	}
	if (page8k != -1) {
		_select_bank_generic_8K(d, page8k, data);
	}
	return;
}

void ms_memmap_write16_MEGAROM_GENERIC_8K(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MEGAROM_GENERIC_8K_t* d = (ms_memmap_driver_MEGAROM_GENERIC_8K_t*)driver;
	ms_memmap_write8_MEGAROM_GENERIC_8K(driver, addr + 0, data & 0xff);
	ms_memmap_write8_MEGAROM_GENERIC_8K(driver, addr + 1, data >> 8);
	return;
}
