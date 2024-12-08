/*
    ESE-SCC, MEGA-SCSI, WAVE-SCSI (MEGA-SCSI+SCC) �̃o���N�؂�ւ��̎d�g��

	���K�����R���g���[���Ƃ��� ASCII 8K�����ł͂Ȃ��AKONAMI SCC�������g���Ă���̂ŁAESE-RAM�Ƃ�
	�o���N�؂�ւ��̎d�g�݂��قȂ�܂��B

	�� region
	MSX�̃X���b�g�� Page1�� Page2�� 32KB�̗̈�� 4�� 8KB�u���b�N�ɕ������Ĉ����܂��B
	���� 8KB�u���b�N�� region �ƌĂт܂��B
	region 0: 0x4000-0x5fff
	region 1: 0x6000-0x7fff
	region 2: 0x8000-0x9fff
	region 3: 0xa000-0xbfff

	�� segment
	�ő�1MB�̃������� 8KB���ɕ����������̂� segment �ƌĂт܂��B
	- 1MB�̏ꍇ�A128�� segment ������܂��B
	- 512KB�̏ꍇ�A64�� segment ������܂��B

	�� �o���N�؂�ւ�
	�e region�̃o���N��؂�ւ��邽�߂ɂ́A�ȉ��̃������Ԓn�ɃZ�O�����g�ԍ����������݂܂��B
	region 0: 0x4000-0x5fff (�ʏ�� 0x4000���g�p)
	region 1: 0x6000-0x7fff (�ʏ�� 0x6000���g�p)
	region 2: 0x8000-0x9fff (�ʏ�� 0x8000���g�p)
	region 3: 0xa000-0xbfff (�ʏ�� 0xa000���g�p)

	�������A���Ƃ��� KONAMI SCC���K�����R���g���[���� �o���N�̎w��� 6bit�����g���Ȃ����߁A
	�؂�ւ�����Z�O�����g�� 0-63�܂ł�64�Ɍ����܂��B���ꂾ�� 512KB�܂ł��������܂���B
	(1MB�܂ň������@�͌�q)

	���̃o���N�؂�ւ����W�X�^���u���ʃo���N�؂�ւ����W�X�^�v�ƌĂт܂��B

	�� SCC���W�X�^�ɃA�N�Z�X������@
	region 2�̃o���N�� 0x3f�ɂ���ƁA0x9800-0x9fff�͈̔͂� SCC���W�X�^���A�N�Z�X�\�ɂȂ�܂��B
	region 2�ȊO�̃o���N�� 0x3f�ɂ����ꍇ�́AROM�̃Z�O�����g0x3f�ɃA�N�Z�X���邱�ƂɂȂ�܂��B

	�� SPC (SCSI Protocol Controller)�̃��W�X�^�ɃA�N�Z�X������@
	��L�o���N�؂�ւ��A�h���X�Ƃ͈قȂ� 0x7ffe, 0x7fff �� bit6�� 1�ɂ���ƁAregion 0��
	SPC�̃��W�X�^��������悤�ɂȂ�܂��B���̐؂�ւ����W�X�^���u��ʃo���N�؂�ւ����W�X�^�v�ƌĂт܂��B

	�Ȃ��A����bit 6��1�ɂ��Ă�region 1-3�ɂ� SPC�̃��W�X�^�͌������A�ʏ�̃o���N�؂�ւ����s���܂��B
	MS.X�ł� MEGA-SCSI�͂܂��T�|�[�g���Ă��Ȃ��̂�SPC�̏ڍׂ͏ȗ����܂��B

	�� SRAM��������������@
	���Ƃ��ƃ��K�����R���g���[����ROM���������̂ŁA���������\��SRAM�������@�\�͂���܂���B
	���ہA�����̗̈�ւ̏������݂��o���N�؂�ւ����W�X�^�ɂȂ��Ă��邹���ŁA���̂܂܂ł�SRAM�̏����������ł��܂���B

	SRAM������������ꍇ�́ASPC�̃��W�X�^�A�N�Z�X�ł��g�p���� 0x7ffe, 0x7fff�́u��ʃo���N�؂�ւ����W�X�^�v��
	bit4���g�p���܂��B���� bit4�� 1�ɂ���ƌ��ݑI������Ă���o���N�����������\�ɂȂ�A���ʃo���N�؂�ւ����W�X�^��
	�g���Ȃ��Ȃ�܂��B
	���̏�Ԃł���ʃo���N�؂�ւ����W�X�^�͎g����̂� bit4�� 0�ɖ߂����ƂŁA�ʏ�̃o���N�؂�ւ����s����悤�ɂȂ�܂��B

	�Ȃ��ASRAM�̏����������L���ɂȂ�̂� region 0��1�݂̂ŁAregion 2, 3�ł͎g���܂���B

	�� 1MB��SRAM���������@
	��ʃo���N�؂�ւ����W�X�^��bit4��1�ɂ���SRAM�̏��������\���[�h�ɐݒ肵�Ă���Ƃ��́A
	bit6�� 1�ɂ��Ă��Aregion 0�� SPC�̃��W�X�^���������A512KB-1MB��SRAM��������悤�ɂȂ�܂��B
	
	������Ƃ�₱�����ł����A���́A��ʃo���N�؂�ւ����W�X�^�� bit6�� bit4�́A�o���N�A�h���X�� bit6, bit7��
	��������ƍl����Ɨ������₷���Ȃ�܂��B�܂�Abit6�� 1�ɂ���ƁA�o���N�A�h���X�� bit6�� 1�ɂȂ�Abit4�� 1�ɂ����
	�o���N�A�h���X�� bit7�� 1�ɂȂ�܂��B
	���̂悤�ɍl����ƁA�e�o���N�A�h���X�ɂ́A�ȉ��̂悤�ȃ��������}�b�s���O����Ă��邱�ƂɂȂ�܂��B

	0x00-0x3f: �Z�O�����g�ԍ� 0-63 �� SRAM (Read Only�Ȃ̂�ROM�Ƃ��ĐU�镑��, �ő� 512KB)
	0x40-0x7f: SPC�̃��W�X�^ (Read/Write, ������ region 0�ł̂ݗL��)
	0x80-0xbf: �Z�O�����g�ԍ� 0-63 �� SRAM (Read/Write, �ő� 512KB)
	0xc0-0xff: �Z�O�����g�ԍ� 64-127 �� SRAM (Read/Write, �ő� 512KB, ������ region 0�ł̂ݗL��)
	���������Aregion 2�Ƀo���N0x3f���w�肵���ꍇ�́ASCC���W�X�^���}�b�s���O����܂�

	���̂悤�ɁA��ʃo���N�؂�ւ����W�X�^�����܂������ƁA1MB�܂ł�SRAM���������Ƃ��ł��܂��B
	�������A���ꂪ�\�Ȃ̂� SPC�𓋍ڂ��� MEGA-SCSI, WAVE-SCSI �̏ꍇ�݂̂̂悤�ł��B

	�� ���ӓ_
	SPC����512KB�̗̈�́Aregion 0�ł����A�N�Z�X�ł��܂���B�܂��ASCC���W�X�^�� region 2�ł����A�N�Z�X�ł��܂���B

	�܂Ƃ߂�ƁA�ȉ��̂悤�ɂȂ�܂��B
                         00-3E 3F   40-7E 7F   80-BF C0-FE FF
	region0 (4000-5fff): ROM   ROM  SPC   SPC  SRAM  SRAM  SRAM
	region1 (6000-7fff): ROM   ROM  00-3E 3F   SRAM  00-3E 3F
	region2 (8000-9fff): ROM   SCC  00-3E SCC  SRAM  00-3E SCC
	region3 (a000-bfff): ROM   ROM  00-3E 3F   SRAM  00-3E 3F

	�� �{���W���[���̎������j
	�{���W���[���́Aselected_segment �� region 0-3�̂��ꂼ��̑I������Ă���Z�O�����g�ԍ���ێ����܂��B
	�Z�O�����g�ԍ��́A��ʃo���N�I�����W�X�^�̒l�������� 6bit (���ʃo���N�I�����W�X�^�̒l) �ɂȂ�܂��B
	��ʃo���N�؂�ւ����W�X�^�̒l�́A�ȉ��̂悤�ɕێ����܂��B

	* bit4: SRAM�̏��������\���[�h
		* write_enable �ɕێ�
	* bit6: 512KB-1MB��SRAM��I�����郂�[�h
		* upper_bank_enable �ɕێ�

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include "ms_memmap.h"
#include "ms_memmap_ESE_SCC.h"
#include "../ms.h"

#define THIS ms_memmap_driver_ESE_SCC_t

static char* driver_name = "ESE_SCC";

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
THIS* ms_memmap_ESE_SCC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_ESE_SCC_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.type = ROM_TYPE_ESE_SCC;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_ESE_SCC_deinit; �I�[�o�[���C�h�s�v
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
		_select_bank(instance, region, region);	// KONAMI SCC���K�����̏ꍇ�A�����l��0,1,2,3
	}

	// SCC�̏�����
	int i;
	uint8_t* scc_segment;

	// �Z�O�����g�ԍ�63��SCC���W�X�^�Ƃ��Ďg�p�ł���̂ŁA���̗̈���m��
	scc_segment = (uint8_t*)new_malloc( 8*1024 );
	if(scc_segment == NULL) {
		printf("���������m�ۂł��܂���B\n");
		return;
	}
	for (i = 0; i < 8*1024; i++) {
		scc_segment[i] = 0xff;
	}
	// init SCC registers
	for (i= 0x9800; i <= 0x98ff; i++) {
		scc_segment[i & 0x1fff] = 0;
	}
	instance->scc_segment = scc_segment;
	
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
	MS_LOG(MS_LOG_INFO, "ESE-SCC��SRAM���t�@�C���ɏ����o���܂�\n");
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
		printf("ESE-SCC��SRAM���t�@�C���ɏ����o���܂���\n");
	}
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

/**
  @brief region (0-3)�Ɍ�����o���N��I�����܂�

	SCC�t����KONAMI_SCC���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#Konami_MegaROMs_with_SCC

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* �؂�ւ��A�h���X:	5000h (mirrors: 5001h~57FFh)
		* �����Z�O�����g	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* �؂�ւ��A�h���X	7000h (mirrors: 7001h~77FFh)
		* �����Z�O�����g	1
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* �؂�ւ��A�h���X	9000h (mirrors: 9001h~97FFh)
		* �����Z�O�����g	Random
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* �؂�ւ��A�h���X	b000h (mirrors: B001h~B7FFh)
		* �����Z�O�����g	Random

 */
static void _select_bank(THIS* d, int region, int segment) {
	uint8_t* buf;

	segment &= 0x3f;
	d->selected_segment[region] = segment;

	if( (region == 2) && (segment == 0x3f) ) {
		// SCC register
		buf = d->scc_segment;
	} else {
		if ( segment >= d->num_segments) {
			MS_LOG(MS_LOG_DEBUG,"ESE_SCC: segment out of range: %d\n", segment);
			buf = NULL;
		} else {
			if ( (region == 0) && d->upper_bank_enable && (d->num_segments == 128) ) {
				// 512KB-1MB��SRAM��I��
				buf = d->base.buffer + (segment * 0x2000) + 0x80000;
			} else {
				buf = d->base.buffer + (segment * 0x2000);
			}
		}
	}

	d->base.page8k_pointers[(region+2)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+2)&0x7);	// �؂�ւ����N���������Ƃ� memmap �ɒʒm
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"ESE_SCC: read out of range: %04x\n", addr);
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
*/
static uint32_t conv_freq(THIS* d, uint32_t freq) {
	switch (d->scc_segment[0x18e0] & 0x3) {
		case 0:
			return freq & 0xfff;
		case 1:
			return (freq & 0xf) << 8;
		case 2:
			return (freq & 0xff) << 4;
		default:
			break;
	}
	// ????
	return (freq & 0xf) << 8;
}

static void _write_scc_v1_reg(THIS* d, uint16_t addr, uint8_t data) {
	MS_LOG(MS_LOG_TRACE, "SCC: write %04x <- %02x\n", addr, data);
	int32_t freq;
	switch(addr) {
		case 0x9880:
		case 0x9881:
		case 0x9890:
		case 0x9891:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1880] | (d->scc_segment[0x1881] << 8);
			w_SCC_freq(0, conv_freq(d, freq));
			break;
		case 0x9882:
		case 0x9883:
		case 0x9892:
		case 0x9893:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1882] | (d->scc_segment[0x1883] << 8);
			w_SCC_freq(1, conv_freq(d, freq));
			break;
		case 0x9884:
		case 0x9885:
		case 0x9894:
		case 0x9895:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1884] | (d->scc_segment[0x1885] << 8);
			w_SCC_freq(2, conv_freq(d, freq));
			break;
		case 0x9886:
		case 0x9887:
		case 0x9896:
		case 0x9897:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1886] | (d->scc_segment[0x1887] << 8);
			w_SCC_freq(3, conv_freq(d, freq));
			break;
		case 0x9888:
		case 0x9889:
		case 0x9898:
		case 0x9899:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1888] | (d->scc_segment[0x1889] << 8);
			//w_SCC_freq(4, conv_freq(d, freq)); �T�|�[�g���Ă��Ȃ�
			break;
		case 0x988a:
		case 0x989a:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(0, data);
			break;
		case 0x988b:
		case 0x989b:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(1, data);
			break;
		case 0x988c:
		case 0x989c:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(2, data);
			break;
		case 0x988d:
		case 0x989d:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(3, data);
			break;
		case 0x988e:
		case 0x989e:
			d->scc_segment[addr & 0x1fef] = data;
			//w_SCC_volume(4, data); �T�|�[�g���Ă��Ȃ�
			break;
		case 0x988f:
		case 0x989f:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_keyon(data);
			break;
		default:
			if (addr >= 0x98e0 && addr <= 0x98ff) {
				d->scc_segment[0x18e0] = data;
				w_SCC_deformation(data);
			} else {
				d->scc_segment[addr & 0x1fff] = data;						
			}
			break;
	}
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	int area = addr >> 11;	// 4K�o�C�g�P�ʂ̃G���A�ԍ�

	if (area < 0x4*2 || area >= 0xc*2) {
		MS_LOG(MS_LOG_FINE,"ESE_SCC: write out of range: %04x\n", addr);
		return;
	}

	// ��ʃo���N�؂�ւ����W�X�^
	if( (addr & 0x7ffe) == 0x7ffe ) {
		d->upper_bank_enable = (data & 0x40) >> 6;
		d->write_enable = (data & 0x10) >> 4;
		_select_bank(d, 0, d->selected_segment[0]); // region 0 �ɉe��������̂ōđI������
		return;
	}

	// 0x4000-0x7ffd ��SRAM�ւ̏�������
	if (d->write_enable && (area < 0x8*2) ) {
		int page8k = addr >> 13;
		int local_addr = addr & 0x1fff;
		uint8_t* p8k = driver->page8k_pointers[page8k];
		if( p8k == NULL ) {
			MS_LOG(MS_LOG_FINE,"ESE_SCC: write out of range: %04x\n", addr);
			return;
		}
		p8k[local_addr] = data;
		return;
	}

	// �o���N�؂�ւ�������SCC���W�X�^�ւ̏�������
	switch(area) {
	case 0x5*2:
		_select_bank(d, 0, data);
		break;
	case 0x7*2:
		_select_bank(d, 1, data);
		break;
	case 0x9*2:
		_select_bank(d, 2, data);
		break;
	case 0x9*2+1:	// 0x9800-0x9fff �ɂ� SCC�̃��W�X�^������
		if ((d->selected_segment[2] == 0x3f) && r_SCC_enable() ) {
			_write_scc_v1_reg(d, addr, data);
		}
		break;
	case 0xb*2:
		_select_bank(d, 3, data);
		break;
	}
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

/**
 * @brief ESE-SCC��SRAM���t�@�C���ɏ����o���܂�
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
