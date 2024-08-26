#ifndef _MS_MEMMAP_H_
#define _MS_MEMMAP_H_

#include <stdint.h>

#define ROM_TYPE_NOTHING			0
#define ROM_TYPE_MAPPER_RAM			1
#define ROM_TYPE_NORMAL_ROM			2
#define ROM_TYPE_MEGAROM_8			3
#define ROM_TYPE_MEGAROM_16			4
#define ROM_TYPE_MEGAROM_KONAMI		5
#define ROM_TYPE_MEGAROM_KONAMI_SCC	6
#define ROM_TYPE_SOUND_CARTRIG		7
#define ROM_TYPE_MEGAROM_PANA		8
#define ROM_TYPE_DOS_ROM			9
#define ROM_TYPE_PAC				10

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

typedef struct ms_memmap_driver ms_memmap_driver_t;

typedef struct ms_memmap_driver {
	// 本ドライバインスタンスを解放する場合に呼び出します
	void (*deinit)(ms_memmap_driver_t* driver);
	// memmapモジュールが本ドライバをアタッチした際に呼び出します
	void (*did_attach)(ms_memmap_driver_t* driver);
	// memmapモジュールが本ドライバをデタッチする際に呼び出します
	int (*will_detach)(ms_memmap_driver_t* driver);
	// メモリマッパーセグメント選択レジスタ(port FCh,FDh,FEh,FFh) の値が変更された際に呼び出します
	void (*did_update_memory_mapper)(ms_memmap_driver_t* driver, int page, uint8_t segment_num);
	// 8ビットの読み出し処理
	uint8_t (*read8)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 16ビットの読み出し処理
	uint16_t (*read16)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 8ビットの書き込み処理
	void (*write8)(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
	// 16ビットの書き込み処理
	void (*write16)(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);

	// タイプ
	int type;
	// 名称
	const char* name;

	// これを管理している memmap への参照
	ms_memmap_t* memmap;
	//
	int attached_slot_base;
	//
	int attached_slot_ex;

	// 64Kバイト空間を8Kバイト単位で区切ったポインタの配列
	// このドライバが対応しているページのポインタのみセットされており、それ以外はNULLが入ります
	// 動作中にポインタの値を書き換えた場合は、memmap->update_page_pointer(attached_slot, page_num)を呼び出してください
	uint8_t* page8k_pointers[8];

	// buffer (各ドライバが使用するバッファ領域)
	uint8_t* buffer;

} ms_memmap_driver_t;

ms_memmap_t* ms_memmap_init();
void ms_memmap_init_mac();
void ms_memmap_deinit(ms_memmap_t* memmap);
void ms_memmap_deinit_mac();
void ms_memmap_set_main_mem( void *, int);

int ms_memmap_attach_driver(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int slot_base, int slot_ex);

void allocateAndSetROM(const char *romFileName, int kind, int slot_base, int slot_ex, int page);
void allocateAndSetROM_Cartridge(const char* romFileName, int slot_base);

int filelength(int fh);

// 最後に置く
#include "ms_memmap_NOTHING.h"
#include "ms_memmap_NORMALROM.h"
#include "ms_memmap_MAINRAM.h"
#include "ms_memmap_MEGAROM_8K.h"
#include "ms_memmap_MEGAROM_KONAMI.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"

#endif // _MS_MEMMAP_H_