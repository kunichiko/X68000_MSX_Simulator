#ifndef MS_MEMMAP_DRIVER_H
#define MS_MEMMAP_DRIVER_H

#include <stdint.h>
#include "ms_memmap.h"

typedef struct ms_memmap_driver ms_memmap_driver_t;

/**
 * @brief 
 * 
 * abstract class�Ȃ̂ŁA�e���\�b�h�̓T�u�N���X�Ŏ������܂��B
 */
typedef struct ms_memmap_driver {
	// �{�h���C�o�C���X�^���X���������ꍇ�ɌĂяo���܂�
	void (*deinit)(ms_memmap_driver_t* driver);
	// memmap���W���[�����{�h���C�o���A�^�b�`�����ۂɌĂяo���܂�
	void (*did_attach)(ms_memmap_driver_t* driver);
	// memmap���W���[�����{�h���C�o���f�^�b�`����ۂɌĂяo���܂�
	int (*will_detach)(ms_memmap_driver_t* driver);
	// �V�~�����[�^���|�[�Y�����Ƃ��ɌĂяo���܂�
	void (*did_pause)(ms_memmap_driver_t* driver);
	// �������}�b�p�[�Z�O�����g�I�����W�X�^(port FCh,FDh,FEh,FFh) �̒l���ύX���ꂽ�ۂɌĂяo���܂�
	void (*did_update_memory_mapper)(ms_memmap_driver_t* driver, int page, uint8_t segment_num);
	// 8�r�b�g�̓ǂݏo������
	uint8_t (*read8)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 16�r�b�g�̓ǂݏo������
	uint16_t (*read16)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 8�r�b�g�̏������ݏ���
	void (*write8)(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
	// 16�r�b�g�̏������ݏ���
	void (*write16)(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);

	// �^�C�v
	int type;
	// ����
	const char* name;

	// ������Ǘ����Ă��� memmap �ւ̎Q��
	ms_memmap_t* memmap;
	//
	int attached_slot_base;
	//
	int attached_slot_ex;

	// 64K�o�C�g��Ԃ�8K�o�C�g�P�ʂŋ�؂����|�C���^�̔z��
	// ���̃h���C�o���Ή����Ă���y�[�W�̃|�C���^�̂݃Z�b�g����Ă���A����ȊO��NULL������܂�
	// ���쒆�Ƀ|�C���^�̒l�������������ꍇ�́Amemmap->update_page_pointer(attached_slot, page_num)���Ăяo���Ă�������
	uint8_t* page8k_pointers[8];

	// buffer (�e�h���C�o���g�p����o�b�t�@�̈�)
	uint8_t* buffer;

} ms_memmap_driver_t;

//ms_memmap_driver_t* ms_memmap_driver_alloc(); // abstract class�Ȃ�d��alloc�͕s�v
void ms_memmap_driver_init(ms_memmap_driver_t* instance, ms_memmap_t* memmap, uint8_t* bufffer);
void ms_memmap_driver_deinit(ms_memmap_driver_t* instance);

#endif