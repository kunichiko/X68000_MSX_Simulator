#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "../ms_R800.h"

ms_memmap_t* ms_memmap_shared = NULL;

void ms_memmap_update_page_pointer(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k);

void select_slot_base_impl(ms_memmap_t* memmap, int page, int slot_base);
void select_slot_ex_impl(ms_memmap_t* memmap, int slot_base, int page, int slot_ex);

/*
	memmapモジュールの確保 & 初期化ルーチン
 */
ms_memmap_t* ms_memmap_init() {
	if (ms_memmap_shared != NULL) {
		return ms_memmap_shared;
	}
	ms_memmap_shared = (ms_memmap_t*)new_malloc(sizeof(ms_memmap_t));
	if ( ms_memmap_shared == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}

	// メンバの初期化

	ms_memmap_shared->update_page_pointer = ms_memmap_update_page_pointer;
	ms_memmap_shared->current_ptr = ms_memmap_current_ptr;

	ms_memmap_shared->mainram_driver = (ms_memmap_driver_MAINRAM_t*)ms_memmap_MAINRAM_init(ms_memmap_shared);
	if( ms_memmap_shared->mainram_driver == NULL) {
		printf("メインメモリが確保できません\n");
		new_free(ms_memmap_shared);
		return NULL;
	}

	ms_memmap_shared->nothing_driver = (ms_memmap_driver_NOTHING_t*)ms_memmap_NOTHING_init(ms_memmap_shared);
	if( ms_memmap_shared->nothing_driver == NULL) {
		printf("NOTHINGドライバが確保できません\n");
		new_free(ms_memmap_shared);
		return NULL;
	}
	int base, ex, page;
	for(base = 0; base < 4; base++) {
		for(ex = 0; ex < 4; ex++) {
			for(page = 0; page < 4; page++) {
				ms_memmap_shared->attached_driver[base][ex][page] = (ms_memmap_driver_t*)ms_memmap_shared->nothing_driver;
			}
		}
	}

	// スロット3を拡張スロットとして使用する
	ms_memmap_shared->slot_expanded.flag[3] = 1;

	// メインメモリをスロット3-0にアタッチ
	if ( ms_memmap_attach_driver(ms_memmap_shared, (ms_memmap_driver_t*)ms_memmap_shared->mainram_driver, 3, 0) != 0) {
		printf("メインメモリのアタッチに失敗しました\n");
		ms_memmap_deinit(ms_memmap_shared);
		return NULL;
	}

	// 現状で初期化
	for(page = 0; page < 4; page++) {
		select_slot_base_impl(ms_memmap_shared, page, ms_memmap_shared->slot_sel[page]);
	}

	return ms_memmap_shared;
}

void ms_memmap_deinit(ms_memmap_t* memmap) {
	if( memmap->mainram_driver != NULL ) {
		ms_memmap_deinit_MAINRAM((ms_memmap_driver_t*)memmap->mainram_driver);
	}
	int base, ex, page;
	for(base = 0; base < 4; base++) {
		for(ex = 0; ex < 4; ex++) {
			for(page = 0; page < 4; page++) {
				ms_memmap_driver_t* driver = memmap->attached_driver[base][ex][page];
				// TODO 一つのドライバが複数のページにまたがっているケースがあるので、
				// TODO ちゃんとデタッチして外してから deinit しないと二重解放になる
				//if( driver != NULL && driver->type != ROM_TYPE_NOTHING && driver->type != ) {
				//	driver->deinit(driver);
				//}
			}
		}
	}
	new_free(memmap->mainram_driver);
	new_free(memmap);
}


/**
 * @brief スロットにドライバをアタッチします
 * 
 * すでにぶつかるドライバがアタッチされていた場合は -1を返します。
 * 拡張されていないスロットに対し、拡張スロット番号が指定されていた場合も -1を返します。
 * 
 * @param memmap memmapインスタンスへのポインタ
 * @param driver 登録したいドライバのインスタンス
 * @param slot_base 基本スロット番号
 * @param slot_ex 拡張スロット番号(基本スロットに配置したい場合は-1)
 * @return ms_memmap_page_slot_t*
 */
int ms_memmap_attach_driver(ms_memmap_t* memmap, ms_memmap_driver_t* driver, const int slot_base, const int slot_ex) {
	if (slot_ex >= 0 && memmap->slot_expanded.flag[slot_base] == 0) {
		printf("拡張されていないスロットの拡張スロットには配置できません。\n");
		return -1;
	}
	int slot_ex_fallback;
	if (slot_ex < 0 ) {
		slot_ex_fallback = 0;
	} else {
		slot_ex_fallback = slot_ex;
	}

	// 衝突検出
	int conflict = 0;
	int page;
	for(page = 0; page < 4; page++) {
		if ( driver->page8k_pointers[page*2] !=  NULL && //
			 memmap->attached_driver[slot_base][slot_ex_fallback][page]->type != ROM_TYPE_NOTHING) {
			conflict = 1;
			break;
		}
	}
	if(conflict) {
		printf("スロットにドライバがすでにアタッチされています。\n");
		return -1;
	}

	// ドライバをアタッチ
	for(page = 0; page < 4; page++) {
		if ( driver->page8k_pointers[page*2] !=  NULL) {
			memmap->attached_driver[slot_base][slot_ex_fallback][page] = driver;
			// このアタッチによって、今見えているページが更新された可能性があるので呼び出す
			select_slot_base_impl(memmap, page, memmap->slot_sel[page]);
		}
	}

	driver->attached_slot_base = slot_base;
	driver->attached_slot_ex = slot_ex;
	driver->did_attach(driver);

	return 0;
}

/**
 * @brief メモリマッパーやメガロムのように、ドライバ内部でページが更新された場合のコールバック
 * 
 * @param memmap 
 * @param driver 
 * @param page8k 
 */
void ms_memmap_update_page_pointer(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k) {
	int slot_base = driver->attached_slot_base;
	int slot_ex = driver->attached_slot_ex;
	if (slot_ex < 0) {
		slot_ex = 0;
	}

	// CPUが見ているページを更新
	// TODO CPU側がまだ16Kにしか対応していないので暫定
	memmap->current_ptr[page8k/2] = memmap->current_driver[page8k/2]->page8k_pointers[page8k & 0xfe];

	// CPU側に通知
	ms_cpu_needs_refresh_PC = 1;
}

/**
 * @brief 指定されたページのスロットを切り替えます
 * 
 * @param memmap 
 * @param page 切り替えたいページ
 * @param slot_base スロット番号
 */
void select_slot_base(ms_memmap_t* memmap, int page, int slot_base) {
	slot_base &= 0x03;
	if( memmap->slot_sel[page] == slot_base) {
		// 変更がない場合は何もしない
		return;
	}
	select_slot_base_impl(memmap, page, slot_base);
}

void select_slot_base_impl(ms_memmap_t* memmap, int page, int slot_base) {
	// 選択したスロットが拡張されているか調べる
	if ( memmap->slot_expanded.flag[slot_base] ) {
		// 拡張されている場合は拡張スロット選択レジスタを見る
		int slot_ex = memmap->slot_sel_ex[slot_base][page];
		memmap->current_driver[page] = memmap->attached_driver[slot_base][slot_ex][page];
	} else {
		memmap->current_driver[page] = memmap->attached_driver[slot_base][0][page];
	}

	// CPUが見てるポインタを更新
	// TODO CPU側がまだ16Kにしか対応していないので暫定
	uint8_t* p = memmap->current_driver[page]->page8k_pointers[page*2+0];
	memmap->current_ptr[page] = p;
	

	memmap->slot_sel[page] = slot_base;
}

/**
 * @brief 指定されたページの拡張スロットを切り替えます。
 * 対象は、現在ページ3に見えている基本スロットの拡張スロットです。
 * 
 * @param memmap 
 * @param page 
 * @param slot_ex 
 */
void select_slot_ex(ms_memmap_t* memmap, int page, int slot_ex) {
	slot_ex &= 0x03;

	// ページ3に見えている基本スロットを取得
	int slot_base = memmap->slot_sel[3];
	// 選択したスロットが拡張されているか調べる
	if ( memmap->slot_expanded.flag[slot_base] == 0 ) {
		// 拡張されていない場合は何もしない
		return;
	}

	if( memmap->slot_sel_ex[slot_base][page] == slot_ex) {
		// 変更がない場合は何もしない
		return;
	}
	select_slot_ex_impl(memmap, slot_base, page, slot_ex);
}

void select_slot_ex_impl(ms_memmap_t* memmap, int slot_base, int page, int slot_ex) {
	if ( memmap->slot_sel[page] == slot_base) {
		// 今回変更した拡張スロットが見えている場合だけ更新
		memmap->current_driver[page] = memmap->attached_driver[slot_base][slot_ex][page];
		// CPUが見てるポインタを更新
		// TODO CPU側がまだ16Kにしか対応していないので暫定
		uint8_t* p = memmap->current_driver[page]->page8k_pointers[page*2+0];
		memmap->current_ptr[page] = p;
	}
	memmap->slot_sel_ex[slot_base][page] = slot_ex;
}



/**
 * @brief スロット選択レジスタ(ポートA8h)への書き込み
 * 
 * bit [1:0]	: ページ0の基本スロット番号
 * bit [3:2]	: ページ1の基本スロット番号
 * bit [5:4]	: ページ2の基本スロット番号
 * bit [7:6]	: ページ3の基本スロット番号
 * 
 */
void write_port_A8(uint8_t data) {
	int page = 0;
	for(page = 0; page < 4; page++) {
		int slot_base = data & 0x03;
		select_slot_base(ms_memmap_shared, page, slot_base);
		data >>= 2;
	}
}

uint8_t read_port_A8() {
	return ms_memmap_shared->slot_sel[0] | (ms_memmap_shared->slot_sel[1] << 2) | (ms_memmap_shared->slot_sel[2] << 4) | (ms_memmap_shared->slot_sel[3] << 6);
}

void write_exslot_reg(uint8_t data) {
	int page = 0;
	for(page = 0; page < 4; page++) {
		int slot_ex = data & 0x03;
		select_slot_ex(ms_memmap_shared, page, slot_ex);
		data >>= 2;
	}
}

uint8_t read_exslot_reg() {
	uint8_t slot_base = ms_memmap_shared->slot_sel[3];
	uint8_t ret = ms_memmap_shared->slot_sel_ex[slot_base][0] | (ms_memmap_shared->slot_sel_ex[slot_base][1] << 2) | (ms_memmap_shared->slot_sel_ex[slot_base][2] << 4) | (ms_memmap_shared->slot_sel_ex[slot_base][3] << 6);
	// 読み出すと反転した値が読める
	return ~ret;
}


/**
 * @brief メモリマッパーのセグメント切り替え処理
 * 
 * @param data 
 */
void write_port_FC(uint8_t data) {
	ms_memmap_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)ms_memmap_shared->mainram_driver, 0, data);
}

void write_port_FD(uint8_t data) {
	ms_memmap_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)ms_memmap_shared->mainram_driver, 1, data);
}

void write_port_FE(uint8_t data) {
	ms_memmap_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)ms_memmap_shared->mainram_driver, 2, data);
}

void write_port_FF(uint8_t data) {
	ms_memmap_shared->mainram_driver->base.did_update_memory_mapper((ms_memmap_driver_t*)ms_memmap_shared->mainram_driver, 3, data);
}

//
//
//
//


uint8_t ms_memmap_read8(uint16_t addr) {
	if (addr == 0xffff) {
		// 拡張スロット選択レジスタのアドレスの場合
		// ページ3が拡張されているかどうかを調べる
		uint8_t slot_base = ms_memmap_shared->slot_sel[3];
		if ( ms_memmap_shared->slot_expanded.flag[slot_base] == 1) {
			// 拡張されている場合は拡張スロット選択レジスタを見る
			return read_exslot_reg();
		}
	}
	// アドレスからページ番号を取得
	int page = (addr >> 14) & 0x03;
	// ページからドライバを特定
	ms_memmap_driver_t* d = ms_memmap_shared->current_driver[page];
	return d->read8(d, addr);
}

void ms_memmap_write8(uint16_t addr, uint8_t data) {
	if (addr == 0xffff) {
		// 拡張スロット選択レジスタのアドレスの場合
		// ページ3が拡張されているかどうかを調べる
		uint8_t slot_base = ms_memmap_shared->slot_sel[3];
		if ( ms_memmap_shared->slot_expanded.flag[slot_base] == 1) {
			// 拡張されている場合は拡張スロット選択レジスタに書き込む
			write_exslot_reg(data);
			return;
		}
	}
	// アドレスからページ番号を取得
	int page = (addr >> 14) & 0x03;
	// ページからドライバを特定
	ms_memmap_driver_t* d = ms_memmap_shared->current_driver[page];
	d->write8(d, addr, data);
}

uint16_t ms_memmap_read16(uint16_t addr) {
	if (addr == 0xfffe || (addr & 0x3fff) == 0x3fff) {
		// 拡張スロット選択レジスタのアドレスのにかかる場合や、
		// ページを跨ぐ場合は、8ビットずつ読む
		uint16_t ret = ms_memmap_read8(addr) | (ms_memmap_read8(addr + 1) << 8);
		return ret;
	}
	// アドレスからページ番号を取得
	int page = (addr >> 14) & 0x03;
	// ページからドライバを特定
	ms_memmap_driver_t* d = ms_memmap_shared->current_driver[page];
	return d->read16(d, addr);
}

void ms_memmap_write16(uint16_t addr, uint16_t data) {
	if (addr == 0xfffe || (addr & 0x3fff) == 0x3fff) {
		// 拡張スロット選択レジスタのアドレスのにかかる場合や、
		// ページを跨ぐ場合は、8ビットずつ読む
		ms_memmap_write8(addr, data & 0xff);
		ms_memmap_write8(addr + 1, (data >> 8) & 0xff);
		return;
	}
	// アドレスからページ番号を取得
	int page = (addr >> 14) & 0x03;
	// ページからドライバを特定
	ms_memmap_driver_t* d = ms_memmap_shared->current_driver[page];
	d->write16(d, addr, data);
}
