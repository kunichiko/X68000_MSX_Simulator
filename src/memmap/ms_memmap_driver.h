#ifndef MS_MEMMAP_DRIVER_H
#define MS_MEMMAP_DRIVER_H

#include <stdint.h>
#include "ms_memmap.h"

typedef struct ms_memmap_driver ms_memmap_driver_t;

/**
 * @brief 
 * 
 * abstract classなので、各メソッドはサブクラスで実装します。
 */
typedef struct ms_memmap_driver {
	// 本ドライバインスタンスを解放する場合に呼び出します
	void (*deinit)(ms_memmap_driver_t* driver);
	// memmapモジュールが本ドライバをアタッチした際に呼び出します
	void (*did_attach)(ms_memmap_driver_t* driver);
	// memmapモジュールが本ドライバをデタッチする際に呼び出します
	int (*will_detach)(ms_memmap_driver_t* driver);
	// シミュレータがポーズしたときに呼び出します
	void (*did_pause)(ms_memmap_driver_t* driver);
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

//ms_memmap_driver_t* ms_memmap_driver_alloc(); // abstract classなのdでallocは不要
void ms_memmap_driver_init(ms_memmap_driver_t* instance, ms_memmap_t* memmap, uint8_t* bufffer);
void ms_memmap_driver_deinit(ms_memmap_driver_t* instance);

#endif