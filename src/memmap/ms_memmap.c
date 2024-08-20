#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"

ms_memmap_t* ms_memmap_shared = NULL;
ms_memmap_driver_MEGAROM_8K_t* megarom_8k = NULL;
ms_memmap_driver_MEGAROM_KONAMI_t* megarom_konami = NULL;

ms_memmap_t* ms_memmap_init() {
	if (ms_memmap_shared != NULL) {
		return ms_memmap_shared;
	}
	ms_memmap_shared = (ms_memmap_t*)new_malloc(sizeof(ms_memmap_t));
	if ( ms_memmap_shared == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}

	ms_memmap_init_mac();

	ms_memmap_shared->main_mem = (uint8_t*)new_malloc(64 * 1024 + 8 * MS_MEMMAP_NUM_SEGMENTS); /* ６４Ｋ + ８バイト＊総セグメント数	*/
	if (ms_memmap_shared->main_mem == NULL) { 
		printf("メモリが確保できません\n");
		new_free(ms_memmap_shared);
		return NULL;
	}
	ms_memmap_set_main_mem(ms_memmap_shared->main_mem, (int)MS_MEMMAP_NUM_SEGMENTS); /* アセンブラのルーチンへ引き渡し		*/

	return ms_memmap_shared;
}

void ms_memmap_deinit(ms_memmap_t* memmap) {
	if (memmap->main_mem == NULL) {
		return;
	}
	new_free(memmap->main_mem);
	new_free(memmap);

	ms_memmap_deinit_mac();
}


void allocateAndSetROM_Cartridge(const char *romFileName) {
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( romFileName, O_RDONLY | O_BINARY);
	if (crt_fh == -1) {
		printf("ファイルが開けません. %s\n", romFileName);
		ms_exit();
		return;
	}
	crt_length = filelength(crt_fh);
	if(crt_length == -1) {
		printf("ファイルの長さが取得できません。\n");
		close(crt_fh);
		ms_exit();
		return;
	}
	close(crt_fh);
	if(crt_length <= 32*1024) {
		allocateAndSetROM(romFileName, ROM_TYPE_NORMAL_ROM, 1<<2, 1);
	} else {
		// MEGAROM KONAMIとしてロードする
		if (megarom_konami != NULL) {
			printf("MEGAROM 8Kは既にロードされています\n");
			return;
		}
		megarom_konami = (ms_memmap_driver_MEGAROM_KONAMI_t*)ms_memmap_MEGAROM_KONAMI_init(ms_memmap_shared, romFileName);
		if( megarom_konami == NULL) {
			printf("MEGAROM 8Kの初期化に失敗しました\n");
			return;
		}
		ms_memmap_register_rom(megarom_konami->base.mem_slot1, ROM_TYPE_MEGAROM_KONAMI, 1<<2, 1);
		ms_memmap_register_rom(megarom_konami->base.mem_slot2, ROM_TYPE_MEGAROM_KONAMI, 1<<2, 2);
	}
}

void allocateAndSetROM(const char *romFileName, int kind, int slot, int page) {
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( romFileName, O_RDONLY | O_BINARY);
	if (crt_fh == -1) {
		printf("ファイルが開けません. %s\n", romFileName);
		ms_exit();
		return;
	}
	crt_length = filelength(crt_fh);
	if(crt_length == -1) {
		printf("ファイルの長さが取得できません。\n");
		ms_exit();
		return;
	}

	// 16Kバイトずつ読み込んでROMにセット
	if( crt_length <= 32 * 1024 ) {
		for(i = 0; i < 2; i++) {
			if(crt_length < 16 * 1024) {
				break;
			}
			if( ( crt_buff = (uint8_t*)new_malloc( 16 * 1024 + MS_MEMMAP_HEADER_LENGTH ) ) == NULL) {
				printf("メモリが確保できません。\n");
				ms_exit();
				return;
			}
			read( crt_fh, crt_buff + MS_MEMMAP_HEADER_LENGTH, 16 * 1024);
			// int j;
			// for(j = 0; j < 16; j++) {
			// 	printf("%02x ", crt_buff[MS_MEMMAP_HEADER_LENGTH + i]);
			// }
			// printf("\n");
			ms_memmap_register_rom(crt_buff, kind, slot, page + i);
			crt_length -= 16 * 1024;
		}
	} else {
		printf("ファイルが認識できませんでした\n");
		ms_exit();
	}
 	close( crt_fh);
}

/*
	ASCII MEGAROM 8K
*/
uint8_t ms_memmap_rd8_MEGAROM_8(uint8_t* mem, int page, uint16_t addr) {
	if(megarom_8k == NULL) {
		printf("MEGAROM 8Kがロードされていません\n");
		return 0xff;
	}
	return megarom_8k->base.read8((ms_memmap_driver_t*)megarom_8k, 0x4000*page + addr);
}

void ms_memmap_wr8_MEGAROM_8(uint8_t* mem, int page, uint16_t addr, uint8_t data) {
	if(megarom_8k == NULL) {
		printf("MEGAROM 8Kがロードされていません\n");
		return;
	}
	megarom_8k->base.write8((ms_memmap_driver_t*)megarom_8k, 0x4000*page + addr, data);
}

uint16_t ms_memmap_rd16_MEGAROM_8(uint8_t* mem, int page, uint16_t addr) {
	if(megarom_8k == NULL) {
		printf("MEGAROM 8Kがロードされていません\n");
		return 0xffff;
	}
	return megarom_8k->base.read16((ms_memmap_driver_t*)megarom_8k, 0x4000*page + addr);
}

void ms_memmap_wr16_MEGAROM_8(uint8_t* mem, int page, uint16_t addr, uint16_t data) {
	if(megarom_8k == NULL) {
		printf("MEGAROM 8Kがロードされていません\n");
		return;
	}
	megarom_8k->base.write16((ms_memmap_driver_t*)megarom_8k, 0x4000*page + addr, data);
}

/*
	KONAAMI MEGAROM
*/
uint8_t ms_memmap_rd8_MEGAROM_KONAMI(uint8_t* mem, int page, uint16_t addr) {
	if(megarom_konami == NULL) {
		printf("MEGAROM KONAMIがロードされていません\n");
		return 0xff;
	}
	return megarom_konami->base.read8((ms_memmap_driver_t*)megarom_konami, 0x4000*page + addr);
}

void ms_memmap_wr8_MEGAROM_KONAMI(uint8_t* mem, int page, uint16_t addr, uint8_t data) {
	if(megarom_konami == NULL) {
		printf("MEGAROM KONAMIがロードされていません\n");
		return;
	}
	megarom_konami->base.write8((ms_memmap_driver_t*)megarom_konami, 0x4000*page + addr, data);
}

uint16_t ms_memmap_rd16_MEGAROM_KONAMI(uint8_t* mem, int page, uint16_t addr) {
	if(megarom_konami == NULL) {
		printf("MEGAROM KONAMIがロードされていません\n");
		return 0xffff;
	}
	return megarom_konami->base.read16((ms_memmap_driver_t*)megarom_konami, 0x4000*page + addr);
}

void ms_memmap_wr16_MEGAROM_KONAMI(uint8_t* mem, int page, uint16_t addr, uint16_t data) {
	if(megarom_konami == NULL) {
		printf("MEGAROM KONAMIがロードされていません\n");
		return;
	}
	megarom_konami->base.write16((ms_memmap_driver_t*)megarom_konami, 0x4000*page + addr, data);
}