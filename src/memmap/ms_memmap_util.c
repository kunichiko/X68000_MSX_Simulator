#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_driver.h"
#include "ms_memmap_NOTHING.h"
#include "ms_memmap_NORMALROM.h"
#include "ms_memmap_MAINRAM.h"
#include "ms_memmap_MEGAROM_GENERIC_8K.h"
#include "ms_memmap_MEGAROM_ASCII_8K.h"
#include "ms_memmap_MEGAROM_KONAMI.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"
#include "../disk/ms_disk_bios_Panasonic.h"

extern ms_memmap_t* ms_memmap_shared;

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
 * @param kind ROMタイプの指定。-1 を指定すると自動判定します
 */
void allocateAndSetROM_Cartridge(const char *romFileName, int slot_base, int kind) {
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

	// ROMデータをロードして、ROMの種類を判定
	crt_buff =  (uint8_t*)new_malloc(crt_length);
	if(crt_buff == NULL) {
		printf("メモリが確保できません。\n");
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
		case ROM_TYPE_NORMAL_ROM:
			allocateAndSetROM(romFileName, ROM_TYPE_NORMAL_ROM, slot_base, -1, 1);
			break;
		case ROM_TYPE_MEGAROM_GENERIC_8K:
			// GENERIC 8K メガロム
			ms_memmap_driver_MEGAROM_GENERIC_8K_t* g8k = ms_memmap_MEGAROM_GENERIC_8K_alloc();
			driver = (ms_memmap_driver_t*)g8k;
			if( driver == NULL) {
				printf("MEGAROM GENERIC 8Kの初期化に失敗しました\n");
				return;
			}
			ms_memmap_MEGAROM_GENERIC_8K_init(g8k, ms_memmap_shared, crt_buff, crt_length);
			break;
		case ROM_TYPE_MEGAROM_ASCII_8K:
			// ASCII 8K メガロム
			ms_memmap_driver_MEGAROM_ASCII_8K_t* a8k = ms_memmap_MEGAROM_ASCII_8K_alloc();
			driver = (ms_memmap_driver_t*)a8k;
			if( driver == NULL) {
				printf("MEGAROM 8Kの初期化に失敗しました\n");
				return;
			}
			ms_memmap_MEGAROM_ASCII_8K_init(a8k, ms_memmap_shared, crt_buff, crt_length);
			break;
		case ROM_TYPE_MEGAROM_KONAMI:
			// MEGAROM KONAMIとしてロードする
			ms_memmap_driver_MEGAROM_KONAMI_t* kon = ms_memmap_MEGAROM_KONAMI_alloc();
			driver = (ms_memmap_driver_t*)kon;
			if( driver == NULL) {
				printf("MEGAROM KONAMIの初期化に失敗しました\n");
				return;
			}
			ms_memmap_MEGAROM_KONAMI_init(kon, ms_memmap_shared, crt_buff, crt_length);
			break;
		case ROM_TYPE_MEGAROM_KONAMI_SCC:
			// MEGAROM KONAMI SCCとしてロードする
			ms_memmap_driver_MEGAROM_KONAMI_SCC_t* scc = ms_memmap_MEGAROM_KONAMI_SCC_alloc();
			driver = (ms_memmap_driver_t*)scc;
			if( driver == NULL) {
				printf("MEGAROM KONAMI SCCの初期化に失敗しました\n");
				return;
			}
			ms_memmap_MEGAROM_KONAMI_SCC_init(scc, ms_memmap_shared, crt_buff, crt_length);
			break;
		default:
			break;
	}
	if(driver != NULL) {
		if ( ms_memmap_attach_driver(ms_memmap_shared, driver, slot_base, -1) != 0) {
			printf("ドライバのアタッチに失敗しました\n");
			driver->deinit(driver);
			new_free(driver);
			return;
		}
	}
}

/*
	OpenMSXの実装を参考に、以下の方法で読み込んだROMの種類を判定します。
	メガロムはZ80の LD (nn),a 命令を使ってメモリへの書き込みを行なってバンク切り替えを行っています。
	それを利用し、ROMの全領域から上記命令 (0x32, 0xLL, 0xHH) の出現回数をカウントし、
	そのランキングを元にROMの種類を判定します。

	nnの値に応じて、以下のようにROMの種類に分類します。

	0x5000, 0x9000, 0xb000	: コナミSCC付き
	0x4000, 0x8000, 0xa000	: コナミSCCなし
	0x6800, 0x7800			: ASCII8K
	0x6000					: コナミSCCなし or ASCII8K or ASCII16K
	0x7000					: コナミSCC付き or ASCII8K or ASCII16K
	0x77ff					: ASCII16K

	合計を取り、最も多いものを採用します。
*/
int detect_rom_type(uint8_t* buffer, int length) {
	int konami_scc_with = 0;
	int konami_scc_without = 0;
	int ascii8k = 0;
	int ascii16k = 0;
	int i;

	if (length <= 32 * 1024) {
		printf("通常ロムと推定しました。\n");
		return ROM_TYPE_NORMAL_ROM;
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

	printf("コナミ SCC 付き: %d\n", konami_scc_with);
	printf("コナミ SCC 無し: %d\n", konami_scc_without);
	printf("ASCII 8K: %d\n", ascii8k);
	printf("ASCII 16K: %d\n", ascii16k);

	if (ascii8k > 0 ) {
		ascii8k--; // 優先順位調整
	}
	if (konami_scc_with == 0 && konami_scc_without == 0 && ascii8k == 0 && ascii16k == 0) {
		printf("GENERIC 8Kメガロムと推定しました。\n");
		return ROM_TYPE_MEGAROM_GENERIC_8K;
	}
	if (konami_scc_with >= konami_scc_without && konami_scc_with >= ascii8k && konami_scc_with >= ascii16k) {
		printf("コナミ SCC 付きメガロムと推定しました。\n");
		return ROM_TYPE_MEGAROM_KONAMI_SCC;
	} else if (konami_scc_without >= konami_scc_with && konami_scc_without >= ascii8k && konami_scc_without >= ascii16k) {
		printf("コナミ SCC 無しメガロムと推定しました。\n");
		return ROM_TYPE_MEGAROM_KONAMI;
	} else if (ascii8k >= konami_scc_with && ascii8k >= konami_scc_without && ascii8k >= ascii16k) {
		printf("ASCII 8K メガロムと推定しました。\n");
		return ROM_TYPE_MEGAROM_ASCII_8K;
	} else if (ascii16k >= konami_scc_with && ascii16k >= konami_scc_without && ascii16k >= ascii8k) {
		printf("ASCII 16K メガロムと推定しました。\n");
		return ROM_TYPE_MEGAROM_ASCII_16K;
	}
	// ここには来ないはず
	printf("GENERIC 8Kメガロムと推定しました。\n");
	return ROM_TYPE_MEGAROM_GENERIC_8K;
}

void allocateAndSetROM(const char *romFileName, int kind, int slot_base, int slot_ex, int page) {
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
			if( ( crt_buff = (uint8_t*)new_malloc( 16 * 1024) ) == NULL) {
				printf("メモリが確保できません。\n");
				ms_exit();
				return;
			}
			read( crt_fh, crt_buff, 16 * 1024);

			ms_memmap_driver_NORMALROM_t* driver = ms_memmap_NORMALROM_alloc();
			if (driver == NULL) {
				printf("メモリが確保できません。\n");
				ms_exit();
				return;
			}
			ms_memmap_NORMALROM_init(driver, ms_memmap_shared, crt_buff, page + i);
		
			if (ms_memmap_attach_driver(ms_memmap_shared, (ms_memmap_driver_t*)driver, slot_base, slot_ex) != 0) {
				printf("メモリマッピングに失敗しました。\n");
				ms_exit();
				return;
			}
			crt_length -= 16 * 1024;
		}
	} else {
		printf("ファイルが認識できませんでした\n");
		ms_exit();
	}
 	close( crt_fh);
}

void allocateAndSetDISKBIOSROM(const char *romFileName, int diskcount, char* diskimages[]) {
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
	if(crt_length != 16*1024) {
		printf("ファイルサイズが不正です。\n");
		ms_exit();
		return;
	}

	// 16Kバイト読み込んでROMにセット
	if( ( crt_buff = (uint8_t*)new_malloc( 16 * 1024) ) == NULL) {
		printf("メモリが確保できません。\n");
		ms_exit();
		return;
	}
	read( crt_fh, crt_buff, 16 * 1024);

	// ディスクコンテナの初期化
	ms_disk_container_t* disk_container = ms_disk_container_alloc();
	if (disk_container == NULL) {
		printf("メモリが確保できません。\n");
		ms_exit();
		return;
	}
	ms_disk_container_init(disk_container, diskcount, diskimages);

	ms_memmap_driver_DISKBIOS_PANASONIC_t* driver = ms_disk_bios_Panasonic_alloc();
	if (driver == NULL) {
		printf("メモリが確保できません。\n");
		ms_exit();
		return;
	}
	ms_disk_bios_Panasonic_init(driver, ms_memmap_shared, crt_buff, disk_container);
	
	// スロット3-2にアタッチ
	if (ms_memmap_attach_driver(ms_memmap_shared, (ms_memmap_driver_t*)driver, 3, 2) != 0) {
		printf("メモリマッピングに失敗しました。\n");
		ms_exit();
		return;
	}
 	close( crt_fh);
}
