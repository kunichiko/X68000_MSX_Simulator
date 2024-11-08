#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_driver.h"
#include "ms_memmap_NOTHING.h"
#include "ms_memmap_MAINRAM.h"

#include "../ms_R800.h"

static ms_memmap_t* _shared = NULL;

void ms_memmap_update_page_pointer(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k);

void select_slot_base_impl(ms_memmap_t* memmap, int page, int slot_base);
void select_slot_ex_impl(ms_memmap_t* memmap, int slot_base, int page, int slot_ex);

/*
	memmap���W���[���̊m�ۃ��[�`��
 */
ms_memmap_t* ms_memmap_shared_instance() {
	if (_shared != NULL) {
		return _shared;
	}
	_shared = (ms_memmap_t*)new_malloc(sizeof(ms_memmap_t));
	if ( _shared == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}

	// �����o�̏�����
	_shared->update_page_pointer = ms_memmap_update_page_pointer;
	_shared->current_ptr = ms_memmap_current_ptr;

	_shared->mainram_driver = ms_memmap_MAINRAM_alloc();
	if( _shared->mainram_driver == NULL) {
		printf("���C�����������m�ۂł��܂���\n");
		new_free(_shared);
		return NULL;
	}
	ms_memmap_MAINRAM_init(_shared->mainram_driver, _shared);

	_shared->nothing_driver = ms_memmap_NOTHING_alloc();
	if( _shared->nothing_driver == NULL) {
		printf("NOTHING�h���C�o���m�ۂł��܂���\n");
		new_free(_shared);
		return NULL;
	}
	ms_memmap_NOTHING_init(_shared->nothing_driver, _shared);

	int base, ex, page;
	for(base = 0; base < 4; base++) {
		for(ex = 0; ex < 4; ex++) {
			for(page = 0; page < 4; page++) {
				_shared->driver_page_map[base][ex][page] = (ms_memmap_driver_t*)_shared->nothing_driver;
			}
		}
	}

	// �X���b�g3���g���X���b�g�Ƃ��Ďg�p����
	_shared->slot_expanded.flag[3] = 1;

	// ���C�����������X���b�g3-0�ɃA�^�b�`
	if ( ms_memmap_attach_driver(_shared, (ms_memmap_driver_t*)_shared->mainram_driver, 3, 0) != 0) {
		printf("���C���������̃A�^�b�`�Ɏ��s���܂���\n");
		ms_memmap_shared_deinit();
		new_free(_shared);
		return NULL;
	}

	// ����ŏ�����
	for(page = 0; page < 4; page++) {
		select_slot_base_impl(_shared, page, _shared->slot_sel[page]);
	}

	return _shared;
}

void _clear_driver(ms_memmap_driver_t* driver);

void ms_memmap_shared_deinit() {
	if( _shared == NULL) {
		return;
	}
	if( _shared->mainram_driver != NULL ) {
		_shared->mainram_driver->base.deinit( (ms_memmap_driver_t*)_shared->mainram_driver);
		new_free(_shared->mainram_driver);
		_shared->mainram_driver = NULL;
	}
	int i;
	for(i = 0; i < 64; i++) {
		ms_memmap_driver_t* driver = _shared->attached_drivers[i];
		if( driver != NULL) {
			driver->will_detach(driver);
			driver->deinit(driver);
		}
	}

	// �V���O���g���̏ꍇ������O�I�� deinit�� free����
	new_free(_shared);
	_shared = NULL;
}

/**
 * @brief �X���b�g�Ƀh���C�o���A�^�b�`���܂�
 * 
 * ���łɂԂ���h���C�o���A�^�b�`����Ă����ꍇ�� -1��Ԃ��܂��B
 * �g������Ă��Ȃ��X���b�g�ɑ΂��A�g���X���b�g�ԍ����w�肳��Ă����ꍇ�� -1��Ԃ��܂��B
 * 
 * @param memmap memmap�C���X�^���X�ւ̃|�C���^
 * @param driver �o�^�������h���C�o�̃C���X�^���X
 * @param slot_base ��{�X���b�g�ԍ�
 * @param slot_ex �g���X���b�g�ԍ�(��{�X���b�g�ɔz�u�������ꍇ��-1)
 * @return ms_memmap_page_slot_t*
 */
int ms_memmap_attach_driver(ms_memmap_t* memmap, ms_memmap_driver_t* driver, const int slot_base, const int slot_ex) {
	if (slot_ex >= 0 && memmap->slot_expanded.flag[slot_base] == 0) {
		printf("�g������Ă��Ȃ��X���b�g�̊g���X���b�g�ɂ͔z�u�ł��܂���B\n");
		return -1;
	}
	int slot_ex_fallback;
	if (slot_ex < 0 ) {
		slot_ex_fallback = 0;
	} else {
		slot_ex_fallback = slot_ex;
	}

	// �h���C�o���X�g�ɒǉ�
	int i;
	for (i = 0; i < 64; i++) {
		if (memmap->attached_drivers[i] == NULL) {
			memmap->attached_drivers[i] = driver;
			break;
		}
	}
	if (i == 64) {
		printf("�A�^�b�`�\�ȃh���C�o�̐��𒴂��܂���\n");
		return -1;
	}

	// �Փˌ��o
	int conflict = 0;
	int page;
	for(page = 0; page < 4; page++) {
		if ( ((driver->page8k_pointers[page*2+0] !=  NULL) || (driver->page8k_pointers[page*2+1] !=  NULL)) && //
			memmap->driver_page_map[slot_base][slot_ex_fallback][page]->type != ROM_TYPE_NOTHING) {
			conflict = 1;
			break;
		}
	}
	if(conflict) {
		printf("�X���b�g�Ƀh���C�o�����łɃA�^�b�`����Ă��܂��B%s ���z�u�ł��܂���ł����B\n", driver->name);
		return -1;
	}

	// �h���C�o���A�^�b�`
	for(page = 0; page < 4; page++) {
		if ( (driver->page8k_pointers[page*2+0] !=  NULL) || (driver->page8k_pointers[page*2+1] !=  NULL)) {
			memmap->driver_page_map[slot_base][slot_ex_fallback][page] = driver;
			// ���̃A�^�b�`�ɂ���āA�������Ă���y�[�W���X�V���ꂽ�\��������̂ŌĂяo��
			select_slot_base_impl(memmap, page, memmap->slot_sel[page]);
		}
	}

	driver->attached_slot_base = slot_base;
	driver->attached_slot_ex = slot_ex;
	driver->did_attach(driver);

	return 0;
}

/**
 * @brief 
 * 
 * @param memmap 
 * @return int 
 */
int ms_memmap_did_pause(ms_memmap_t* memmap) {
	int i;
	for(i = 0; i < 64; i++) {
		ms_memmap_driver_t* driver = memmap->attached_drivers[i];
		if( (driver != NULL) && (driver->did_pause != NULL) ) {
			driver->did_pause(driver);
		}
	}
	return 0;
}


/**
 * @brief �������}�b�p�[�⃁�K�����̂悤�ɁA�h���C�o�����Ńy�[�W���X�V���ꂽ�ꍇ�̃R�[���o�b�N
 * 
 * @param memmap 
 * @param driver 
 * @param page8k 
 */
void ms_memmap_update_page_pointer(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k) {
	int slot_base = driver->attached_slot_base;
	int slot_ex = driver->attached_slot_ex;
	if (slot_ex < 0) {
		slot_ex = 0;
	}

	// CPU�����Ă���y�[�W���X�V
	memmap->current_ptr[page8k] = memmap->current_driver[page8k/2]->page8k_pointers[page8k];

	// CPU���ɒʒm
	ms_cpu_needs_refresh_PC = 1;
}

/**
 * @brief �w�肳�ꂽ�y�[�W�̃X���b�g��؂�ւ��܂�
 * 
 * @param memmap 
 * @param page �؂�ւ������y�[�W
 * @param slot_base �X���b�g�ԍ�
 */
void select_slot_base(ms_memmap_t* memmap, int page, int slot_base) {
	slot_base &= 0x03;
	if( memmap->slot_sel[page] == slot_base) {
		// �ύX���Ȃ��ꍇ�͉������Ȃ�
		return;
	}
	select_slot_base_impl(memmap, page, slot_base);
}

void select_slot_base_impl(ms_memmap_t* memmap, int page, int slot_base) {
	// �I�������X���b�g���g������Ă��邩���ׂ�
	if ( memmap->slot_expanded.flag[slot_base] ) {
		// �g������Ă���ꍇ�͊g���X���b�g�I�����W�X�^������
		int slot_ex = memmap->slot_sel_ex[slot_base][page];
		memmap->current_driver[page] = memmap->driver_page_map[slot_base][slot_ex][page];
	} else {
		memmap->current_driver[page] = memmap->driver_page_map[slot_base][0][page];
	}

	// CPU�����Ă�|�C���^���X�V
	memmap->current_ptr[page*2+0] = memmap->current_driver[page]->page8k_pointers[page*2+0];
	memmap->current_ptr[page*2+1] = memmap->current_driver[page]->page8k_pointers[page*2+1];

	// CPU���ɒʒm
	ms_cpu_needs_refresh_PC = 1;

	memmap->slot_sel[page] = slot_base;
}

/**
 * @brief �w�肳�ꂽ�y�[�W�̊g���X���b�g��؂�ւ��܂��B
 * �Ώۂ́A���݃y�[�W3�Ɍ����Ă����{�X���b�g�̊g���X���b�g�ł��B
 * 
 * @param memmap 
 * @param page 
 * @param slot_ex 
 */
void select_slot_ex(ms_memmap_t* memmap, int page, int slot_ex) {
	slot_ex &= 0x03;

	// �y�[�W3�Ɍ����Ă����{�X���b�g���擾
	int slot_base = memmap->slot_sel[3];
	// �I�������X���b�g���g������Ă��邩���ׂ�
	if ( memmap->slot_expanded.flag[slot_base] == 0 ) {
		// �g������Ă��Ȃ��ꍇ�͉������Ȃ�
		return;
	}

	if( memmap->slot_sel_ex[slot_base][page] == slot_ex) {
		// �ύX���Ȃ��ꍇ�͉������Ȃ�
		return;
	}
	select_slot_ex_impl(memmap, slot_base, page, slot_ex);
}

void select_slot_ex_impl(ms_memmap_t* memmap, int slot_base, int page, int slot_ex) {
	if ( memmap->slot_sel[page] == slot_base) {
		// ����ύX�����g���X���b�g�������Ă���ꍇ�����X�V
		memmap->current_driver[page] = memmap->driver_page_map[slot_base][slot_ex][page];
		// CPU�����Ă�|�C���^���X�V
		memmap->current_ptr[page*2+0] = memmap->current_driver[page]->page8k_pointers[page*2+0];
		memmap->current_ptr[page*2+1] = memmap->current_driver[page]->page8k_pointers[page*2+1];

		// CPU���ɒʒm
		ms_cpu_needs_refresh_PC = 1;
	}
	memmap->slot_sel_ex[slot_base][page] = slot_ex;
}



/**
 * @brief �X���b�g�I�����W�X�^(�|�[�gA8h)�ւ̏�������
 * 
 * bit [1:0]	: �y�[�W0�̊�{�X���b�g�ԍ�
 * bit [3:2]	: �y�[�W1�̊�{�X���b�g�ԍ�
 * bit [5:4]	: �y�[�W2�̊�{�X���b�g�ԍ�
 * bit [7:6]	: �y�[�W3�̊�{�X���b�g�ԍ�
 * 
 */
void write_port_A8(uint8_t data) {
	int page = 0;
	for(page = 0; page < 4; page++) {
		int slot_base = data & 0x03;
		select_slot_base(_shared, page, slot_base);
		data >>= 2;
	}
}

uint8_t read_port_A8() {
	return _shared->slot_sel[0] | (_shared->slot_sel[1] << 2) | (_shared->slot_sel[2] << 4) | (_shared->slot_sel[3] << 6);
}

void write_exslot_reg(uint8_t data) {
	int page = 0;
	for(page = 0; page < 4; page++) {
		int slot_ex = data & 0x03;
		select_slot_ex(_shared, page, slot_ex);
		data >>= 2;
	}
}

uint8_t read_exslot_reg() {
	uint8_t slot_base = _shared->slot_sel[3];
	uint8_t ret = _shared->slot_sel_ex[slot_base][0] | (_shared->slot_sel_ex[slot_base][1] << 2) | (_shared->slot_sel_ex[slot_base][2] << 4) | (_shared->slot_sel_ex[slot_base][3] << 6);
	// �ǂݏo���Ɣ��]�����l���ǂ߂�
	return ~ret;
}


/**
 * @brief �������}�b�p�[�̃Z�O�����g�؂�ւ�����
 * 
 * @param data 
 */
void write_port_FC(uint8_t data) {
	_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)_shared->mainram_driver, 0, data);
}

void write_port_FD(uint8_t data) {
	_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)_shared->mainram_driver, 1, data);
}

void write_port_FE(uint8_t data) {
	_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)_shared->mainram_driver, 2, data);
}

void write_port_FF(uint8_t data) {
	_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)_shared->mainram_driver, 3, data);
}

//
//
//
//


uint8_t ms_memmap_read8(uint16_t addr) {
	if (addr == 0xffff) {
		// �g���X���b�g�I�����W�X�^�̃A�h���X�̏ꍇ
		// �y�[�W3���g������Ă��邩�ǂ����𒲂ׂ�
		uint8_t slot_base = _shared->slot_sel[3];
		if ( _shared->slot_expanded.flag[slot_base] == 1) {
			// �g������Ă���ꍇ�͊g���X���b�g�I�����W�X�^������
			return read_exslot_reg();
		}
	}
	// �A�h���X����y�[�W�ԍ����擾
	int page = (addr >> 14) & 0x03;
	// �y�[�W����h���C�o�����
	ms_memmap_driver_t* d = _shared->current_driver[page];
	return d->read8(d, addr);
}

void ms_memmap_write8(uint16_t addr, uint8_t data) {
	if (addr == 0xffff) {
		// �g���X���b�g�I�����W�X�^�̃A�h���X�̏ꍇ
		// �y�[�W3���g������Ă��邩�ǂ����𒲂ׂ�
		uint8_t slot_base = _shared->slot_sel[3];
		if ( _shared->slot_expanded.flag[slot_base] == 1) {
			// �g������Ă���ꍇ�͊g���X���b�g�I�����W�X�^�ɏ�������
			write_exslot_reg(data);
			return;
		}
	}
	// �A�h���X����y�[�W�ԍ����擾
	int page = (addr >> 14) & 0x03;
	// �y�[�W����h���C�o�����
	ms_memmap_driver_t* d = _shared->current_driver[page];
	d->write8(d, addr, data);
}

uint16_t ms_memmap_read16(uint16_t addr) {
	if (addr == 0xfffe || (addr & 0x3fff) == 0x3fff) {
		// �g���X���b�g�I�����W�X�^�̃A�h���X�ɂ�����ꍇ��A
		// �y�[�W���ׂ��ꍇ�́A8�r�b�g���ǂ�
		uint16_t ret = ms_memmap_read8(addr) | (ms_memmap_read8(addr + 1) << 8);
		return ret;
	}
	// �A�h���X����y�[�W�ԍ����擾
	int page = (addr >> 14) & 0x03;
	// �y�[�W����h���C�o�����
	ms_memmap_driver_t* d = _shared->current_driver[page];
	return d->read16(d, addr);
}

void ms_memmap_write16(uint16_t addr, uint16_t data) {
	if (addr == 0xfffe || (addr & 0x3fff) == 0x3fff) {
		// �g���X���b�g�I�����W�X�^�̃A�h���X�̂ɂ�����ꍇ��A
		// �y�[�W���ׂ��ꍇ�́A8�r�b�g���ǂ�
		ms_memmap_write8(addr, data & 0xff);
		ms_memmap_write8(addr + 1, (data >> 8) & 0xff);
		return;
	}
	// �A�h���X����y�[�W�ԍ����擾
	int page = (addr >> 14) & 0x03;
	// �y�[�W����h���C�o�����
	ms_memmap_driver_t* d = _shared->current_driver[page];
	d->write16(d, addr, data);
}
