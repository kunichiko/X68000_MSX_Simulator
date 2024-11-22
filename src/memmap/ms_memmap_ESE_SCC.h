#ifndef MS_MEMMAP_ESE_SCC_H
#define MS_MEMMAP_ESE_SCC_H

#include "ms_memmap.h"
#include "ms_memmap_driver.h"

/*
*/
typedef struct ms_memmap_driver_ESE_SCC {
	ms_memmap_driver_t base;
	// extended properties

	// SCCレジスタセグメント(セグメント番号0x3fをセットするとアクセス可能になる)
	uint8_t* scc_segment;

	int num_segments;

	// region 0-3のそれぞれのセグメント選択状態
	// region は以下のように割り振っていて、オフセットがついているので注意
	// 0 : 0x4000-0x5FFF (0xc000-0xdFFFにもミラー)
	// 1 : 0x6000-0x7FFF (0xE000-0xFFFFにもミラー)
	// 2 : 0x8000-0x9FFF (0x0000-0x1FFFにもミラー)
	// 3 : 0xA000-0xBFFF (0x2000-0x3FFFにもミラー)
	int selected_segment[4];

	// SRAMの書き込み有効フラグ
	// 上位バンク選択レジスタ(0x7ffe, 0x7fff)のbit4が1のときに有効
	int write_enable;

	// セグメント 0x40-0x7fの有効フラグ
	// 上位バンク選択レジスタ(0x7ffe, 0x7fff)のbit6が1のときに有効
	// region 0でのみ有効なので注意
	int upper_bank_enable;

	// SRAMファイルのパス
	uint8_t file_path[256];
} ms_memmap_driver_ESE_SCC_t;

ms_memmap_driver_ESE_SCC_t* ms_memmap_ESE_SCC_alloc();
void ms_memmap_ESE_SCC_init(ms_memmap_driver_ESE_SCC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path);
void ms_memmap_ESE_SCC_deinit(ms_memmap_driver_ESE_SCC_t* instance);

#endif