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

/*	スロットが拡張されているかどうか	*/
typedef struct slot_ex_info {
	char flag[4];
} slot_ex_info_t;


typedef struct ms_memmap {
	// ページが切り替わった際に各ドライバから呼び出されるコールバック
	void (*update_page_pointer)(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k);

	// メインメモリのインスタンスは特別に参照を持つ
	// (attached_driverの中にも含まれる)
	ms_memmap_driver_MAINRAM_t* mainram_driver;

	// 何も存在しないページ用のドライバインスタンスも特別に参照を持つ
	ms_memmap_driver_NOTHING_t* nothing_driver;

	// 現在の基本スロット選択状態 (= 0xa8レジスタの値)
	uint8_t	slot_sel[4];

	// 現在の拡張スロット選択状態 (= 0xffffレジスタの値 x 4セット)
	uint8_t slot_sel_ex[4][4];

	// スロットが拡張されているかどうか
	slot_ex_info_t slot_expanded;

	// 現在CPUから見えているスロット配置
	// この値は、上記 slot_sel, slot_sel_expanded, slot_sel_ex から導出されますが、
	// 毎回計算すると遅いため、この値をキャッシュしています
	// また、この値が更新される際は、CPU側にその変更を通知するようになっていますので、
	// 外部で書き換えないでください。
	ms_memmap_driver_t* current_driver[4];

	// 全てのスロット配置
	// 基本スロット4 x 拡張スロット4 x ページ4
	ms_memmap_driver_t* attached_driver[4][4][4];


	// CPU側と共有しているポインタの配列へのポインタ
	uint8_t** current_ptr;
} ms_memmap_t;


/*
	このアドレスはヘッダ等を含まない部分を指す。よって、ヘッダは
	(アドレス) - 8 のアドレスから存在する。
*/
typedef struct ms_memmap_page {
	uint8_t *first_half;    // 前半8Kバイト
	uint8_t *second_half;   // 後半8Kバイト
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

int allocateAndSetNORMALROM(int fh, int kind, int slot_base, int slot_ex, int page);
void allocateAndSetDISKBIOSROM(const char *romFileName, ms_disk_container_t* disk_container);
void allocateAndSetCartridge(const char* romFileName, int slot_base, int kind);

uint8_t ms_memmap_read8(uint16_t addr);
void ms_memmap_write8(uint16_t addr, uint8_t data);
uint16_t ms_memmap_read16(uint16_t addr);
void ms_memmap_write16(uint16_t addr, uint16_t data);

int filelength(int fh);

// 最後に置く
// #include "ms_memmap_driver.h"
// #include "ms_memmap_NOTHING.h"
// #include "ms_memmap_NORMALROM.h"
// #include "ms_memmap_MAINRAM.h"
// #include "ms_memmap_MEGAROM_GENERIC_8K.h"
// #include "ms_memmap_MEGAROM_ASCII_8K.h"
// #include "ms_memmap_MEGAROM_KONAMI.h"
// #include "ms_memmap_MEGAROM_KONAMI_SCC.h"

#endif // _MS_MEMMAP_H_