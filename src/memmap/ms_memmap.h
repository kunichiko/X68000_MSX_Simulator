#ifndef _MS_MEMMAP_H_
#define _MS_MEMMAP_H_

#include <stdint.h>
#include "../disk/ms_disk_container.h"

#define ROM_TYPE_NOTHING			0
#define ROM_TYPE_MAPPER_RAM			1
#define ROM_TYPE_NORMAL_ROM			2
#define ROM_TYPE_MIRRORED_ROM		3
#define ROM_TYPE_MEGAROM_GENERIC_8K	4
#define ROM_TYPE_MEGAROM_ASCII_8K	5
#define ROM_TYPE_MEGAROM_ASCII_16K	6
#define ROM_TYPE_MEGAROM_KONAMI		7
#define ROM_TYPE_MEGAROM_KONAMI_SCC	8
#define ROM_TYPE_SOUND_CARTRIG		9
#define ROM_TYPE_MEGAROM_PANA		10
#define ROM_TYPE_DOS_ROM			11
#define ROM_TYPE_PAC				12

#define MS_MEMMAP_HEADER_LENGTH 8
#define MS_MEMMAP_NUM_SEGMENTS 4

extern uint8_t** ms_memmap_current_ptr;

typedef struct ms_memmap_driver ms_memmap_driver_t;
typedef struct ms_memmap ms_memmap_t;
typedef struct ms_memmap_driver_MAINRAM ms_memmap_driver_MAINRAM_t;
typedef struct ms_memmap_driver_NOTHING ms_memmap_driver_NOTHING_t;

/*	�X���b�g���g������Ă��邩�ǂ���	*/
typedef struct slot_ex_info {
	char flag[4];
} slot_ex_info_t;


typedef struct ms_memmap {
	// �y�[�W���؂�ւ�����ۂɊe�h���C�o����Ăяo�����R�[���o�b�N
	void (*update_page_pointer)(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k);

	// ���C���������̃C���X�^���X�͓��ʂɎQ�Ƃ�����
	// (attached_driver�̒��ɂ��܂܂��)
	ms_memmap_driver_MAINRAM_t* mainram_driver;

	// �������݂��Ȃ��y�[�W�p�̃h���C�o�C���X�^���X�����ʂɎQ�Ƃ�����
	ms_memmap_driver_NOTHING_t* nothing_driver;

	// ���݂̊�{�X���b�g�I����� (= 0xa8���W�X�^�̒l)
	uint8_t	slot_sel[4];

	// ���݂̊g���X���b�g�I����� (= 0xffff���W�X�^�̒l x 4�Z�b�g)
	uint8_t slot_sel_ex[4][4];

	// �X���b�g���g������Ă��邩�ǂ���
	slot_ex_info_t slot_expanded;

	// ����CPU���猩���Ă���X���b�g�z�u
	// ���̒l�́A��L slot_sel, slot_sel_expanded, slot_sel_ex ���瓱�o����܂����A
	// ����v�Z����ƒx�����߁A���̒l���L���b�V�����Ă��܂�
	// �܂��A���̒l���X�V�����ۂ́ACPU���ɂ��̕ύX��ʒm����悤�ɂȂ��Ă��܂��̂ŁA
	// �O���ŏ��������Ȃ��ł��������B
	ms_memmap_driver_t* current_driver[4];

	// �S�ẴX���b�g�z�u
	// ��{�X���b�g4 x �g���X���b�g4 x �y�[�W4
	ms_memmap_driver_t* driver_page_map[4][4][4];

	// �A�^�b�`����Ă��邷�ׂẴh���C�o�̃��X�g
	ms_memmap_driver_t* attached_drivers[64];

	// CPU���Ƌ��L���Ă���|�C���^�̔z��ւ̃|�C���^
	uint8_t** current_ptr;
} ms_memmap_t;


/*
	���̃A�h���X�̓w�b�_�����܂܂Ȃ��������w���B����āA�w�b�_��
	(�A�h���X) - 8 �̃A�h���X���瑶�݂���B
*/
typedef struct ms_memmap_page {
	uint8_t *first_half;    // �O��8K�o�C�g
	uint8_t *second_half;   // �㔼8K�o�C�g
} ms_memmap_page_t;

typedef struct ms_memmap_slot {
	ms_memmap_page_t page_0;
	ms_memmap_page_t page_1;
	ms_memmap_page_t page_2;
	ms_memmap_page_t page_3;
} ms_memmap_page_slot_t;

ms_memmap_t* ms_memmap_shared_instance();
void ms_memmap_shared_deinit();

void ms_memmap_init_mac();
void ms_memmap_deinit_mac();
void ms_memmap_set_main_mem( void *, int);

int ms_memmap_attach_driver(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int slot_base, int slot_ex);
int ms_memmap_did_pause(ms_memmap_t* memmap);

int allocateAndSetNORMALROM(int fh, int kind, int slot_base, int slot_ex, int page);
void allocateAndSetDISKBIOSROM(const char *romFileName, ms_disk_container_t* disk_container);
void allocateAndSetCartridge(const char* romFileName, int slot_base, int kind);

uint8_t ms_memmap_read8(uint16_t addr);
void ms_memmap_write8(uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16(uint16_t addr);
void ms_memmap_write16(uint16_t addr, uint16_t data);

int filelength(int fh);

// �Ō�ɒu��
// #include "ms_memmap_driver.h"
// #include "ms_memmap_NOTHING.h"
// #include "ms_memmap_NORMALROM.h"
// #include "ms_memmap_MAINRAM.h"
// #include "ms_memmap_MEGAROM_GENERIC_8K.h"
// #include "ms_memmap_MEGAROM_ASCII_8K.h"
// #include "ms_memmap_MEGAROM_KONAMI.h"
// #include "ms_memmap_MEGAROM_KONAMI_SCC.h"

#endif // _MS_MEMMAP_H_