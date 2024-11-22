#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_driver.h"
#include "ms_memmap_NOTHING.h"
#include "ms_memmap_NORMALROM.h"
#include "ms_memmap_MIRROREDROM.h"
#include "ms_memmap_MAINRAM.h"
#include "ms_memmap_MEGAROM_GENERIC_8K.h"
#include "ms_memmap_MEGAROM_ASCII_8K.h"
#include "ms_memmap_MEGAROM_KONAMI.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"
#include "ms_memmap_PAC.h"
#include "ms_memmap_ESE_RAM.h"
#include "ms_memmap_ESE_SCC.h"
#include "../disk/ms_disk_bios_Panasonic.h"

int detect_rom_type(uint8_t* buffer, int length);

int filelength(int fh) {
	struct stat st;
	if( fstat(fh, &st) == -1) {
		return -1;
	}
	return st.st_size;
}

/**
 * @brief 
 * 
 * @param romFileName 
 * @param slot_base 
 * @param kind ROM�^�C�v�̎w��B-1 ���w�肷��Ǝ������肵�܂�
 */
void allocateAndSetCartridge(const char *romFileName, int slot_base, int kind) {
	int crt_fh;
	int crt_length;
	int buf_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( romFileName, O_RDONLY | O_BINARY);
	if (crt_fh == -1) {
		printf("�t�@�C�����J���܂���. %s\n", romFileName);
		ms_exit();
		return;
	}
	crt_length = filelength(crt_fh);
	if(crt_length == -1) {
		printf("�t�@�C���̒������擾�ł��܂���B\n");
		close(crt_fh);
		ms_exit();
		return;
	}
	// length �� 8K�̋��E�ɂȂ��ꍇ��8K�ɐ؂�グ
	if(crt_length % 0x2000 != 0) {
		buf_length = (crt_length + 0x1fff) & 0xffffe000;
	} else {
		buf_length = crt_length;
	}

	// ROM�f�[�^�����[�h���āAROM�̎�ނ𔻒�
	crt_buff =  (uint8_t*)new_malloc(buf_length);
	if(crt_buff == NULL) {
		printf("���������m�ۂł��܂���B\n");
		return;
	}
	read( crt_fh, crt_buff, crt_length);
	close(crt_fh);

	if(0) {
		int x,y;
		for(y=0;y<2;y++) {
			printf("%04x: ", y*16);
			for(x=0;x<16;x++) {
				printf("%02x ", crt_buff[y*16 + x]);
			}
			printf("\n");
		}
	}

	ms_memmap_driver_t* driver = NULL;
	if (kind == -1) {
		kind = detect_rom_type(crt_buff, crt_length);
	}
	switch(kind) {
		case ROM_TYPE_MIRRORED_ROM:
			ms_memmap_driver_MIRROREDROM_t* mir = ms_memmap_MIRROREDROM_alloc();
			driver = (ms_memmap_driver_t*)mir;
			if( driver == NULL) {
				printf("MIRROREDROM�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_MIRROREDROM_init(mir, ms_memmap_shared_instance(), crt_buff, buf_length);
			break;
		case ROM_TYPE_MEGAROM_GENERIC_8K:
			// GENERIC 8K ���K����
			ms_memmap_driver_MEGAROM_GENERIC_8K_t* g8k = ms_memmap_MEGAROM_GENERIC_8K_alloc();
			driver = (ms_memmap_driver_t*)g8k;
			if( driver == NULL) {
				printf("MEGAROM GENERIC 8K�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_MEGAROM_GENERIC_8K_init(g8k, ms_memmap_shared_instance(), crt_buff, buf_length);
			break;
		case ROM_TYPE_MEGAROM_ASCII_8K:
			// ASCII 8K ���K����
			ms_memmap_driver_MEGAROM_ASCII_8K_t* a8k = ms_memmap_MEGAROM_ASCII_8K_alloc();
			driver = (ms_memmap_driver_t*)a8k;
			if( driver == NULL) {
				printf("MEGAROM 8K�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_MEGAROM_ASCII_8K_init(a8k, ms_memmap_shared_instance(), crt_buff, buf_length);
			break;
		case ROM_TYPE_MEGAROM_KONAMI:
			// MEGAROM KONAMI�Ƃ��ă��[�h����
			ms_memmap_driver_MEGAROM_KONAMI_t* kon = ms_memmap_MEGAROM_KONAMI_alloc();
			driver = (ms_memmap_driver_t*)kon;
			if( driver == NULL) {
				printf("MEGAROM KONAMI�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_MEGAROM_KONAMI_init(kon, ms_memmap_shared_instance(), crt_buff, buf_length);
			break;
		case ROM_TYPE_MEGAROM_KONAMI_SCC:
			// MEGAROM KONAMI SCC�Ƃ��ă��[�h����
			ms_memmap_driver_MEGAROM_KONAMI_SCC_t* scc = ms_memmap_MEGAROM_KONAMI_SCC_alloc();
			driver = (ms_memmap_driver_t*)scc;
			if( driver == NULL) {
				printf("MEGAROM KONAMI SCC�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_MEGAROM_KONAMI_SCC_init(scc, ms_memmap_shared_instance(), crt_buff, buf_length);
			break;
		case ROM_TYPE_PAC:
			// PAC�Ƃ��ă��[�h����
			ms_memmap_driver_PAC_t* pac = ms_memmap_PAC_alloc();
			driver = (ms_memmap_driver_t*)pac;
			if( driver == NULL) {
				printf("PAC�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_PAC_init(pac, ms_memmap_shared_instance(), crt_buff, buf_length, crt_length, (uint8_t*)romFileName);
			break;
		case ROM_TYPE_ESE_RAM:
			// ESE_RAM�Ƃ��ă��[�h����
			ms_memmap_driver_ESE_RAM_t* eram = ms_memmap_ESE_RAM_alloc();
			driver = (ms_memmap_driver_t*)eram;
			if( driver == NULL) {
				printf("ESE-RAM�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_ESE_RAM_init(eram, ms_memmap_shared_instance(), crt_buff, buf_length, (uint8_t*)romFileName);
			break;
		case ROM_TYPE_ESE_SCC:
			// ESE_SCC�Ƃ��ă��[�h����
			ms_memmap_driver_ESE_SCC_t* escc = ms_memmap_ESE_SCC_alloc();
			driver = (ms_memmap_driver_t*)escc;
			if( driver == NULL) {
				printf("ESE-SCC�̏������Ɏ��s���܂���\n");
				return;
			}
			ms_memmap_ESE_SCC_init(escc, ms_memmap_shared_instance(), crt_buff, buf_length, (uint8_t*)romFileName);
			break;
		default:
			break;
	}
	if(driver != NULL) {
		if ( ms_memmap_attach_driver(ms_memmap_shared_instance(), driver, slot_base, -1) != 0) {
			printf("�h���C�o�̃A�^�b�`�Ɏ��s���܂���\n");
			driver->deinit(driver);
			new_free(driver);
			return;
		}
		printf(" Loaded %s\n", driver->name);
	}
}

/*
	OpenMSX�̎������Q�l�ɁA�ȉ��̕��@�œǂݍ���ROM�̎�ނ𔻒肵�܂��B
	���K������Z80�� LD (nn),a ���߂��g���ă������ւ̏������݂��s�Ȃ��ăo���N�؂�ւ����s���Ă��܂��B
	����𗘗p���AROM�̑S�̈悩���L���� (0x32, 0xLL, 0xHH) �̏o���񐔂��J�E���g���A
	���̃����L���O������ROM�̎�ނ𔻒肵�܂��B

	nn�̒l�ɉ����āA�ȉ��̂悤��ROM�̎�ނɕ��ނ��܂��B

	0x5000, 0x9000, 0xb000	: �R�i�~SCC�t��
	0x4000, 0x8000, 0xa000	: �R�i�~SCC�Ȃ�
	0x6800, 0x7800			: ASCII8K
	0x6000					: �R�i�~SCC�Ȃ� or ASCII8K or ASCII16K
	0x7000					: �R�i�~SCC�t�� or ASCII8K or ASCII16K
	0x77ff					: ASCII16K

	���v�����A�ł��������̂��̗p���܂��B
*/
int detect_rom_type(uint8_t* buffer, int length) {
	int konami_scc_with = 0;
	int konami_scc_without = 0;
	int ascii8k = 0;
	int ascii16k = 0;
	int i;

	// 8206 = 16 + 8192 - 2
	if ( (length == 8206) && (memcmp(buffer, "PAC2 BACKUP DATA", 16UL) == 0)) {
		printf("PAC�Ɛ��肵�܂����B\n");
		return ROM_TYPE_PAC;
	}

	// ����ȍ~��8K�o�C�g�P�ʂ̂��̂����Ȃ��̂ł����Ńu���b�N
	if( (length & 0x1fff) != 0 ) {
		printf("�s����ROM�ł��B\n");
		return ROM_TYPE_NOTHING;
	}

	if (length <= 32 * 1024) {
		printf("�ʏ�̃~���[�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MIRRORED_ROM;
	}

	for (i = 0; i < length - 3; i++) {
		if (buffer[i] == 0x32) {
			uint16_t value = ((uint16_t)buffer[i + 1]) + (((uint16_t)buffer[i + 2]) << 8);
			switch (value) {
				case 0x5000:
				case 0x9000:
				case 0xb000:
					konami_scc_with++;
					break;
				case 0x4000:
				case 0x8000:
				case 0xa000:
					konami_scc_without++;
					break;
				case 0x6800:
				case 0x7800:
					ascii8k++;
					break;
				case 0x6000:
					konami_scc_without++;
					ascii8k++;
					ascii16k++;
					break;
				case 0x7000:
					konami_scc_with++;
					ascii8k++;
					ascii16k++;
					break;
				case 0x77ff:
					ascii16k++;
					break;
			}
		}
	}

	printf("�R�i�~ SCC �t��: %d\n", konami_scc_with);
	printf("�R�i�~ SCC ����: %d\n", konami_scc_without);
	printf("ASCII 8K: %d\n", ascii8k);
	printf("ASCII 16K: %d\n", ascii16k);

	if (ascii8k > 0 ) {
		ascii8k--; // �D�揇�ʒ���
	}
	if (konami_scc_with == 0 && konami_scc_without == 0 && ascii8k == 0 && ascii16k == 0) {
		printf("GENERIC 8K���K�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MEGAROM_GENERIC_8K;
	}
	if (konami_scc_with >= konami_scc_without && konami_scc_with >= ascii8k && konami_scc_with >= ascii16k) {
		printf("�R�i�~ SCC �t�����K�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MEGAROM_KONAMI_SCC;
	} else if (konami_scc_without >= konami_scc_with && konami_scc_without >= ascii8k && konami_scc_without >= ascii16k) {
		printf("�R�i�~ SCC �������K�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MEGAROM_KONAMI;
	} else if (ascii8k >= konami_scc_with && ascii8k >= konami_scc_without && ascii8k >= ascii16k) {
		printf("ASCII 8K ���K�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MEGAROM_ASCII_8K;
	} else if (ascii16k >= konami_scc_with && ascii16k >= konami_scc_without && ascii16k >= ascii8k) {
		printf("ASCII 16K ���K�����Ɛ��肵�܂����B\n");
		return ROM_TYPE_MEGAROM_ASCII_16K;
	}
	// �����ɂ͗��Ȃ��͂�
	printf("GENERIC 8K���K�����Ɛ��肵�܂����B\n");
	return ROM_TYPE_MEGAROM_GENERIC_8K;
}

int allocateAndSetNORMALROM(int crt_fh, int kind, int slot_base, int slot_ex, int page) {
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_length = filelength(crt_fh);
	if(crt_length == -1) {
		printf("�t�@�C���̒������擾�ł��܂���B\n");
		ms_exit();
		return 0;
	}

	// 16K�o�C�g���ǂݍ����ROM�ɃZ�b�g
	if( crt_length <= 32 * 1024 ) {
		for(i = 0; i < 2; i++) {
			if(crt_length < 16 * 1024) {
				break;
			}
			if( ( crt_buff = (uint8_t*)new_malloc( 16 * 1024) ) == NULL) {
				printf("���������m�ۂł��܂���B\n");
				ms_exit();
				return 0;
			}
			read( crt_fh, crt_buff, 16 * 1024);

			ms_memmap_driver_NORMALROM_t* driver = ms_memmap_NORMALROM_alloc();
			if (driver == NULL) {
				printf("���������m�ۂł��܂���B\n");
				ms_exit();
				return 0;
			}
			ms_memmap_NORMALROM_init(driver, ms_memmap_shared_instance(), crt_buff, page + i);
		
			if (ms_memmap_attach_driver(ms_memmap_shared_instance(), (ms_memmap_driver_t*)driver, slot_base, slot_ex) != 0) {
				printf("�������}�b�s���O�Ɏ��s���܂����B\n");
				ms_exit();
				return 0;
			}
			crt_length -= 16 * 1024;
		}
	} else {
		printf("�t�@�C�����F���ł��܂���ł���\n");
		return 0;
	}
 	close( crt_fh);
	return 1;
}

void allocateAndSetDISKBIOSROM(const char *romFileName, ms_disk_container_t* disk_container) {
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( romFileName, O_RDONLY | O_BINARY);
	if (crt_fh == -1) {
		printf("�t�@�C�����J���܂���. %s\n", romFileName);
		ms_exit();
		return;
	}
	crt_length = filelength(crt_fh);
	if(crt_length == -1) {
		printf("�t�@�C���̒������擾�ł��܂���B\n");
		ms_exit();
		return;
	}
	if(crt_length != 16*1024) {
		printf("�t�@�C���T�C�Y���s���ł��B\n");
		ms_exit();
		return;
	}

	// 16K�o�C�g�ǂݍ����ROM�ɃZ�b�g
	if( ( crt_buff = (uint8_t*)new_malloc( 16 * 1024) ) == NULL) {
		printf("���������m�ۂł��܂���B\n");
		ms_exit();
		return;
	}
	read( crt_fh, crt_buff, 16 * 1024);

	ms_memmap_driver_DISKBIOS_PANASONIC_t* driver = ms_disk_bios_Panasonic_alloc();
	if (driver == NULL) {
		printf("���������m�ۂł��܂���B\n");
		ms_exit();
		return;
	}
	ms_disk_bios_Panasonic_init(driver, ms_memmap_shared_instance(), crt_buff, disk_container);
	
	// �X���b�g3-2�ɃA�^�b�`
	if (ms_memmap_attach_driver(ms_memmap_shared_instance(), (ms_memmap_driver_t*)driver, 3, 2) != 0) {
		printf("�������}�b�s���O�Ɏ��s���܂����B\n");
		ms_exit();
		return;
	}
 	close( crt_fh);
}
