#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <x68k/dos.h>
#include "ms_disk.h"
#include "ms_disk_media_dskformat.h"
#include "../ms.h"

#define THIS ms_disk_media_dskformat_t

static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);

/*
	�m�ۃ��[�`��
 */
THIS* ms_disk_media_dskformat_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

int ms_disk_media_dskformat_init(THIS* instance, char* file_path) {
	if (instance == NULL) {
		return 0;
	}
	// base�N���X�̏�����
	ms_disk_media_sectorbase_init(&instance->base, file_path);
	// ���\�b�h�̓o�^
	instance->base.base.deinit =  (void (*)(ms_disk_media_t*))ms_disk_media_dskformat_deinit; //override
	instance->base.read_sector = _read_sector; //override
	instance->base.write_sector = _write_sector; //override

	// �v���p�e�B�̏�����
	instance->file_path = file_path;
	int attr = _dos_chmod(file_path, -1); // �t�@�C���������擾
	if (attr == -1) {
		printf("�t�@�C�������݂��܂���. %s\n", file_path);
		return 0;
	}
	if ( (attr & 0x20) == 0 ) {
		printf("�t�@�C�����ʏ�̃t�@�C���ł͂���܂���. %s\n", file_path);
		return 0;
	}
	if ( (attr & 0x10) != 0 ) {
		printf("�f�B���N�g���͎w��ł��܂���. %s\n", file_path);
		return 0;
	}
	instance->base.base.is_write_protected = (attr & 0x1) == 1; // READ ONLY����������΃��C�g�v���e�N�g
	int mode = instance->base.base.is_write_protected ? O_RDONLY : O_RDWR;
	instance->file_handle = open(file_path, mode | O_BINARY);
	if (instance->file_handle == -1) {
		printf("�t�@�C�����J���܂���. %s\n", file_path);
		return 0;
	}
	uint32_t file_size = filelength(instance->file_handle);

	instance->file_size	= file_size;
	instance->base.sectors_per_track = 9;
	instance->base.heads = file_size <= 512*9*80 ? 1 : 2;
	instance->base.tracks = (file_size / instance->base.heads) / 9 / 512;
	return 1;
}

void ms_disk_media_dskformat_deinit(THIS* instance) {
	ms_disk_media_sectorbase_deinit(&instance->base);
	close(instance->file_handle);
}

/**
 * @brief �Z�N�^�[��ǂݍ��݂܂��B
 * 
 * @param instance 
 * @param sector_id 1����n�܂邱�Ƃɒ���
 * @param sector 
 */
static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	int index = sector_id - 1;
	THIS* dsk = (THIS*)instance;
	uint32_t offset = index * 512;
	lseek(dsk->file_handle, offset, SEEK_SET);
	read(dsk->file_handle, sector, 512);
}

static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	if(instance->is_write_protected) {
		return;
	}
	int index = sector_id - 1;
	THIS* dsk = (THIS*)instance;
	uint32_t offset = index * 512;
	lseek(dsk->file_handle, offset, SEEK_SET);
	if( write(dsk->file_handle, sector, 512) != 512 ) {
		MS_LOG(MS_LOG_ERROR, "�Z�N�^�̏������݂Ɏ��s���܂���. %s\n", dsk->file_path);
		return;
	}
	MS_LOG(MS_LOG_FINE, "Wr logical sector: %d\n", sector_id);

	// �t�@�C���̃t���b�V��
	// HEiJ �� HFS(Host File System)��œ������Ă���ƁA���� _dos_flush() ���Ȃ��ƃt�@�C�����X�V����Ȃ�
	// ���Ƃ��킩��B
	// X68000�̎��@�̏ꍇ�A���̃v���Z�X�����̃t�@�C����`�����邱�Ƃ���{�I�ɂȂ��̂ŁA���ۂɃt���b�V������Ă���
	// �����𒲂ׂ�̂�����i�������Z�b�g�����Ă݂邵���Ȃ��j�̂ŁA���ۂɎ��@�ł� flush ����Ă��Ȃ��̂��A
	// ����t�� HFS�̓����ŁAflush ���Ă΂��܂�Host���̃t�@�C�����X�V���Ȃ��d�l�Ȃ̂��͂킩��Ȃ����A
	// �G�~�����[�^���Ŏg���l�������̂ŁA�Ƃ肠���� flush ���Ă����B
	_dos_fflush();
}
