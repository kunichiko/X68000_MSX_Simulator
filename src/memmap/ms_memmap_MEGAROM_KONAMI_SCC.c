#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"
#include "../peripheral/ms_SCC.h"
#include "../ms.h"

#define THIS ms_memmap_driver_MEGAROM_KONAMI_SCC_t

static char* driver_name = "MEGAROM_KONAMI_SCC";

static void _select_bank(THIS* d, int region, int bank);
static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

static void _write_scc_v1_reg(THIS* d, uint16_t addr, uint8_t data);
static void _write_scc_v2_reg(THIS* d, uint16_t addr, uint8_t data);


/*
	確保ルーチン
 */
THIS* ms_memmap_MEGAROM_KONAMI_SCC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_MEGAROM_KONAMI_SCC_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_MEGAROM_KONAMI_SCC;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MEGAROM_KONAMI_SCC_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->base.buffer_length = length;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	int region;
	for(region = 0; region < 4; region++) {
		_select_bank(instance, region, region);	// KONAMI SCCメガロムの場合、初期値は0,1,2,3
	}

	// SCCレジスタの初期化
	instance->scc_mode = 0;

	int i;
	uint8_t* scc_segment;

	// セグメント番号63はSCCレジスタとして使用できるので、その領域を確保
	scc_segment = (uint8_t*)new_malloc( 8*1024 );
	if(scc_segment == NULL) {
		printf("メモリが確保できません。\n");
		return;
	}
	if (instance->num_segments == 0x40) {
		// 512KBの場合、セグメント番号 0x3f (63) はSCCレジスタと領域がかぶっているいので、
		// SCCレジスタ以外の部分にはROMのデータが見えるように初期化する
		for (i = 0; i < 8*1024; i++) {
			scc_segment[i] = buffer[0x3f*0x2000 + i];
		}
	} else {
		// それ以外の場合は0xffで埋める
		for (i = 0; i < 8*1024; i++) {
			scc_segment[i] = 0xff;
		}
	}
	// init SCC registers
	for (i= 0x9800; i <= 0x98ff; i++) {
		scc_segment[i & 0x1fff] = 0;
	}
	instance->scc_segment = scc_segment;

	return;
}

void _did_attach(ms_memmap_driver_t* driver) {
}

int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"MEGAROM_KONAMI_SCC: read out of range: %04x\n", addr);
		return 0xff;
	}

	uint8_t ret = p8k[local_addr];
	return ret;
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

/*
	SCC付きのKONAMI_SCCメガロムの切り替え処理
	https://www.msx.org/wiki/MegaROM_Mappers#Konami_MegaROMs_with_SCC

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* 切り替えアドレス:	5000h (mirrors: 5001h~57FFh)
		* 初期セグメント	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* 切り替えアドレス	7000h (mirrors: 7001h~77FFh)
		* 初期セグメント	1
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* 切り替えアドレス	9000h (mirrors: 9001h~97FFh)
		* 初期セグメント	Random
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* 切り替えアドレス	b000h (mirrors: B001h~B7FFh)
		* 初期セグメント	Random

	SCC+の場合、0xb800-0xbfffにSCC+のレジスタがある。
	特に、0xbffe, 0xbfffにSCC+のモードレジスタがある。両者は同一のレジスタ。

	SCC+を使用するためにはまず、0xbffeの bit4を1にする必要がある。
	その後、SCCのバンク4(A000h~BFFFh, page8k=5)の選択レジスタのbit7を1にすると、SCC+のレジスタがアクセス可能になる。
 */
static void _select_bank(THIS* d, int region, int segment) {
	uint8_t* buf = NULL;

	if ( d->scc_mode == 0 ) {
		segment &= 0x3f;
		if ( segment == 0x3f) {
			// SCC register
			buf = d->scc_segment;
		} else {
			if ( segment >= d->num_segments) {
				MS_LOG(MS_LOG_DEBUG,"MEGAROM_KONAMI_SCC: segment out of range: %d\n", segment);
				buf = NULL;
			} else {
				buf = d->base.buffer + (segment * 0x2000);
			}
		}
	} else {	
		if ( (segment == 3) && (segment & 0x80) ) {
			// SCC+の場合、region 3の選択レジスタのbit7を1にすると、SCC+のレジスタがアクセス可能になる
			buf =  d->scc_segment;
			segment = 0x80;
		} else {
			segment &= 0x3f;
			if ( segment >= d->num_segments) {
				MS_LOG(MS_LOG_DEBUG,"MEGAROM_KONAMI_SCC: segment out of range: %d\n", segment);
				buf = NULL;
			} else {
				buf = d->base.buffer + (segment * 0x2000);
			}
		}
	}

	d->selected_segment[region] = segment;
	d->base.page8k_pointers[(region+2)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+2)&0x7);	// 切り替えが起こったことを memmap に通知
}


/*
*/
static uint32_t conv_freq(THIS* d, uint32_t freq) {
	switch (d->scc_segment[0x18e0] & 0x3) {
		case 0:
			return freq & 0xfff;
		case 1:
			return (freq & 0xf) << 8;
		case 2:
			return (freq & 0xff) << 4;
		default:
			break;
	}
	// ????
	return (freq & 0xf) << 8;
}

static void _write_scc_mode_reg(THIS* d, uint16_t addr, uint8_t data) {
	if (addr == 0xbffe || addr == 0xbfff) {
		d->scc_mode = (data & 0b00010000) >> 4;
	}
}

static void _write_scc_v1_reg(THIS* d, uint16_t addr, uint8_t data) {
	MS_LOG(MS_LOG_TRACE, "SCC: write %04x <- %02x\n", addr, data);
	int32_t freq;
	switch(addr) {
		case 0x9880:
		case 0x9881:
		case 0x9890:
		case 0x9891:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1880] | (d->scc_segment[0x1881] << 8);
			w_SCC_freq(0, conv_freq(d, freq));
			break;
		case 0x9882:
		case 0x9883:
		case 0x9892:
		case 0x9893:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1882] | (d->scc_segment[0x1883] << 8);
			w_SCC_freq(1, conv_freq(d, freq));
			break;
		case 0x9884:
		case 0x9885:
		case 0x9894:
		case 0x9895:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1884] | (d->scc_segment[0x1885] << 8);
			w_SCC_freq(2, conv_freq(d, freq));
			break;
		case 0x9886:
		case 0x9887:
		case 0x9896:
		case 0x9897:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1886] | (d->scc_segment[0x1887] << 8);
			w_SCC_freq(3, conv_freq(d, freq));
			break;
		case 0x9888:
		case 0x9889:
		case 0x9898:
		case 0x9899:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x1888] | (d->scc_segment[0x1889] << 8);
			//w_SCC_freq(4, conv_freq(d, freq)); サポートしていない
			break;
		case 0x988a:
		case 0x989a:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(0, data);
			break;
		case 0x988b:
		case 0x989b:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(1, data);
			break;
		case 0x988c:
		case 0x989c:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(2, data);
			break;
		case 0x988d:
		case 0x989d:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(3, data);
			break;
		case 0x988e:
		case 0x989e:
			d->scc_segment[addr & 0x1fef] = data;
			//w_SCC_volume(4, data); サポートしていない
			break;
		case 0x988f:
		case 0x989f:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_keyon(data);
			break;
		default:
			if (addr >= 0x98e0 && addr <= 0x98ff) {
				d->scc_segment[0x18e0] = data;
				w_SCC_deformation(data);
			} else {
				d->scc_segment[addr & 0x1fff] = data;						
			}
			break;
	}
}

static void _write_scc_v2_reg(THIS* d, uint16_t addr, uint8_t data) {
	MS_LOG(MS_LOG_TRACE, "SCC+: write %04x <- %02x\n", addr, data);
	int32_t freq;
	switch(addr) {
		case 0xb8a0:
		case 0xb8a1:
		case 0xb8b0:
		case 0xb8b1:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x18a0] | (d->scc_segment[0x18a1] << 8);
			w_SCC_freq(0, conv_freq(d, freq));
			break;
		case 0xb8a2:
		case 0xb8a3:
		case 0xb8b2:
		case 0xb8b3:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x18a2] | (d->scc_segment[0x18a3] << 8);
			w_SCC_freq(1, conv_freq(d, freq));
			break;
		case 0xb8a4:
		case 0xb8a5:
		case 0xb8b4:
		case 0xb8b5:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x18a4] | (d->scc_segment[0x18a5] << 8);
			w_SCC_freq(2, conv_freq(d, freq));
			break;
		case 0xb8a6:
		case 0xb8a7:
		case 0xb8b6:
		case 0xb8b7:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x18a6] | (d->scc_segment[0x18a7] << 8);
			w_SCC_freq(3, conv_freq(d, freq));
			break;
		case 0xb8a8:
		case 0xb8a9:
		case 0xb8b8:
		case 0xb8b9:
			d->scc_segment[addr & 0x1fef] = data;
			freq = d->scc_segment[0x18a8] | (d->scc_segment[0x18a9] << 8);
			//w_SCC_freq(4, conv_freq(d, freq)); サポートしていない
			break;
		case 0xb8aa:
		case 0xb8ba:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(0, data);
			break;
		case 0xb8ab:
		case 0xb8bb:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(1, data);
			break;
		case 0xb8ac:
		case 0xb8bc:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(2, data);
			break;
		case 0xb8ad:
		case 0xb8bd:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_volume(3, data);
			break;
		case 0xb8ae:
		case 0xb8be:
			d->scc_segment[addr & 0x1fef] = data;
			//w_SCC_volume(4, data); サポートしていない
			break;
		case 0xb8af:
		case 0xb8bf:
			d->scc_segment[addr & 0x1fef] = data;
			w_SCC_keyon(data);
			break;
		default:
			if (addr >= 0xb8e0 && addr <= 0xb8ff) {
				d->scc_segment[0x18e0] = data;
				w_SCC_deformation(data);
			} else {
				d->scc_segment[addr & 0x1fff] = data;						
			}
			break;
	}
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	// バンク切り替え処理
	int area = addr >> 11;
	switch(area) {
	case 0x5*2:
		_select_bank(d, 0, data);
		break;
	case 0x7*2:
		_select_bank(d, 1, data);
		break;
	case 0x9*2:
		_select_bank(d, 2, data);
		break;
	case 0x9*2+1:	// 0x9800-0x9fff には SCCのレジスタがある
		if ((d->selected_segment[2] == 0x3f) && (d->scc_mode == 0) && (r_SCC_enable() != 0) ) {
			_write_scc_v1_reg(d, addr, data);
		}
		break;
	case 0xb*2:
		_select_bank(d, 3, data);
		break;
	case 0xb*2+1:	// 0xb800-0xbfff には SCC+のレジスタがある
		if (r_SCC_enable() != 0) {
			if ((addr == 0xbffe) || (addr == 0xbfff)) {
				_write_scc_mode_reg(d, addr, data);
			} else if ( (d->scc_mode != 0) && (d->selected_segment[3] == 0x80) ) {
				_write_scc_v2_reg(d, addr, data);
			}
		}
		break;
	}
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}
