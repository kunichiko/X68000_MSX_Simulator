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

/*	スロットが拡張されているかどうか	*/
struct slot_ex_info_st {
	char slot_0;
	char slot_1;
	char slot_2;
	char slot_3;
};

struct one_slot_st {
	uint8_t *page_0;
	uint8_t *page_1;
	uint8_t *page_2;
	uint8_t *page_3;
};

/*
	あるページの実体を指すポインタ。バンク切り替えのできるページの場合、
	現在選択されているものが入っている。
*/
typedef struct slot_map	{
	struct one_slot_st s00;
	struct one_slot_st s01;
	struct one_slot_st s02;
	struct one_slot_st s03;

	struct one_slot_st s10;
	struct one_slot_st s11;
	struct one_slot_st s12;
	struct one_slot_st s13;

	struct one_slot_st s20;
	struct one_slot_st s21;
	struct one_slot_st s22;
	struct one_slot_st s23;

	struct one_slot_st s30;
	struct one_slot_st s31;
	struct one_slot_st s32;
	struct one_slot_st s33;
} slot_map_t;

extern slot_map_t *ms_memmap_slot_map_ptr;

typedef struct ms_memmap {
	uint8_t *main_mem;
} ms_memmap_t;

typedef struct ms_memmap_driver ms_memmap_driver_t;

typedef struct ms_memmap_driver {
	// manager
	ms_memmap_t* memmap;

	// memory (CPUからSLOT1、SLOT2として見える領域)
	// ヘッダ領域を含むため、実際のメモリ領域はMS_MEMMAP_HEADER_LENGTHバイトずれている
	// 設定後は動かしてはいけないので、ポインタの指す先を変更しないこと
	// バンク切り替え処理が行われたら、愚直に中身を書き換えるしかない
	// TODO: 将来的には8Kバンク切り替えの処理を追加する
	uint8_t* mem_slot1;
	uint8_t* mem_slot2;

	// buffer (各ドライバが使用するバッファ領域)
	uint8_t* buffer;

	void (*deinit)(ms_memmap_driver_t* driver);
	uint8_t (*read8)(ms_memmap_driver_t* memmap, uint16_t addr);
	uint16_t (*read16)(ms_memmap_driver_t* memmap, uint16_t addr);
	void (*write8)(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
	void (*write16)(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);
} ms_memmap_driver_t;

// 構造体を拡張し、プロパティを追加する
typedef struct ms_memmap_driver_MEGAROM_8K {
	ms_memmap_driver_t base;
	// extended properties
	int bank_size;
	int selected_bank[4];	// SLOT1前半、SLOT1後半、SLOT2前半、SLOT2後半の4つのバンクの選択状態
} ms_memmap_driver_MEGAROM_8K_t;

// 構造体を拡張し、プロパティを追加する
typedef struct ms_memmap_driver_MEGAROM_KONAMI {
	ms_memmap_driver_t base;
	// extended properties
	int bank_size;
	int selected_bank[4];	// SLOT1前半、SLOT1後半、SLOT2前半、SLOT2後半の4つのバンクの選択状態
} ms_memmap_driver_MEGAROM_KONAMI_t;

// 構造体を拡張し、プロパティを追加する
typedef struct ms_memmap_driver_MEGAROM_KONAMI_SCC {
	ms_memmap_driver_t base;
	// extended properties
	int bank_size;
	int selected_bank[4];	// SLOT1前半、SLOT1後半、SLOT2前半、SLOT2後半の4つのバンクの選択状態
} ms_memmap_driver_MEGAROM_KONAMI_SCC_t;




ms_memmap_t* ms_memmap_init();
void ms_memmap_init_mac();
void ms_memmap_deinit(ms_memmap_t* memmap);
void ms_memmap_deinit_mac();
void ms_memmap_set_main_mem( void *, int);

ms_memmap_driver_t* ms_memmap_MEGAROM_8K_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length);
ms_memmap_driver_t* ms_memmap_MEGAROM_KONAMI_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length);
ms_memmap_driver_t* ms_memmap_MEGAROM_KONAMI_SCC_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length);

void allocateAndSetROM(const char* romFileName, int kind, int slot, int page);
void allocateAndSetROM_Cartridge(const char* romFileName);

#endif // _MS_MEMMAP_H_