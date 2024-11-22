/*
    ESE-SCC, MEGA-SCSI, WAVE-SCSI (MEGA-SCSI+SCC) のバンク切り替えの仕組み

	メガロムコントローラとして ASCII 8K方式ではなく、KONAMI SCC方式を使っているので、ESE-RAMとは
	バンク切り替えの仕組みが異なります。

	● region
	MSXのスロットの Page1と Page2の 32KBの領域を 4つの 8KBブロックに分割して扱います。
	この 8KBブロックを region と呼びます。
	region 0: 0x4000-0x5fff
	region 1: 0x6000-0x7fff
	region 2: 0x8000-0x9fff
	region 3: 0xa000-0xbfff

	● segment
	最大1MBのメモリを 8KBずつに分解したものを segment と呼びます。
	- 1MBの場合、128個の segment があります。
	- 512KBの場合、64個の segment があります。

	● バンク切り替え
	各 regionのバンクを切り替えるためには、以下のメモリ番地にセグメント番号を書き込みます。
	region 0: 0x4000-0x5fff (通常は 0x4000を使用)
	region 1: 0x6000-0x7fff (通常は 0x6000を使用)
	region 2: 0x8000-0x9fff (通常は 0x8000を使用)
	region 3: 0xa000-0xbfff (通常は 0xa000を使用)

	ただし、もともと KONAMI SCCメガロムコントローラは バンクの指定に 6bitしか使えないため、
	切り替えられるセグメントは 0-63までの64個に限られます。これだと 512KBまでしか扱えません。
	(1MBまで扱う方法は後述)

	このバンク切り替えレジスタを「下位バンク切り替えレジスタ」と呼びます。

	● SCCレジスタにアクセスする方法
	region 2のバンクを 0x3fにすると、0x9800-0x9fffの範囲に SCCレジスタがアクセス可能になります。
	region 2以外のバンクを 0x3fにした場合は、ROMのセグメント0x3fにアクセスすることになります。

	● SPC (SCSI Protocol Controller)のレジスタにアクセスする方法
	上記バンク切り替えアドレスとは異なる 0x7ffe, 0x7fff の bit6を 1にすると、region 0に
	SPCのレジスタが見えるようになります。この切り替えレジスタを「上位バンク切り替えレジスタ」と呼びます。

	なお、このbit 6を1にしてもregion 1-3には SPCのレジスタは見えず、通常のバンク切り替えが行われます。
	MS.Xでは MEGA-SCSIはまだサポートしていないのでSPCの詳細は省略します。

	● SRAMを書き換える方法
	もともとメガロムコントローラはROMを扱うもので、書き換え可能なSRAMを扱う機能はありません。
	実際、多くの領域への書き込みがバンク切り替えレジスタになっているせいで、そのままではSRAMの書き換えができません。

	SRAMを書き換える場合は、SPCのレジスタアクセスでも使用した 0x7ffe, 0x7fffの「上位バンク切り替えレジスタ」の
	bit4を使用します。この bit4を 1にすると現在選択されているバンクが書き換え可能になり、下位バンク切り替えレジスタも
	使えなくなります。
	この状態でも上位バンク切り替えレジスタは使えるので bit4を 0に戻すことで、通常のバンク切り替えが行えるようになります。

	なお、SRAMの書き換えが有効になるのは region 0と1のみで、region 2, 3では使えません。

	● 1MBのSRAMを扱う方法
	上位バンク切り替えレジスタのbit4を1にしてSRAMの書き換え可能モードに設定しているときは、
	bit6を 1にしても、region 0に SPCのレジスタが見えず、512KB-1MBのSRAMが見えるようになります。
	
	ちょっとややこしいですが、実は、上位バンク切り替えレジスタの bit6と bit4は、バンクアドレスの bit6, bit7に
	相当すると考えると理解しやすくなります。つまり、bit6を 1にすると、バンクアドレスの bit6が 1になり、bit4を 1にすると
	バンクアドレスの bit7が 1になります。
	このように考えると、各バンクアドレスには、以下のようなメモリがマッピングされていることになります。

	0x00-0x3f: セグメント番号 0-63 の SRAM (Read OnlyなのでROMとして振る舞う, 最大 512KB)
	0x40-0x7f: SPCのレジスタ (Read/Write, ただし region 0でのみ有効)
	0x80-0xbf: セグメント番号 0-63 の SRAM (Read/Write, 最大 512KB)
	0xc0-0xff: セグメント番号 64-127 の SRAM (Read/Write, 最大 512KB, ただし region 0でのみ有効)
	※ただし、region 2にバンク0x3fを指定した場合は、SCCレジスタがマッピングされます

	このように、上位バンク切り替えレジスタをうまくつかうと、1MBまでのSRAMを扱うことができます。
	ただし、これが可能なのは SPCを搭載した MEGA-SCSI, WAVE-SCSI の場合のみのようです。

	● 注意点
	SPCや上位512KBの領域は、region 0でしかアクセスできません。また、SCCレジスタは region 2でしかアクセスできません。

	まとめると、以下のようになります。
                         00-3E 3F   40-7E 7F   80-BF C0-FE FF
	region0 (4000-5fff): ROM   ROM  SPC   SPC  SRAM  SRAM  SRAM
	region1 (6000-7fff): ROM   ROM  00-3E 3F   SRAM  00-3E 3F
	region2 (8000-9fff): ROM   SCC  00-3E SCC  SRAM  00-3E SCC
	region3 (a000-bfff): ROM   ROM  00-3E 3F   SRAM  00-3E 3F

	● 本モジュールの実装方針
	本モジュールは、selected_segment に region 0-3のそれぞれの選択されているセグメント番号を保持します。
	セグメント番号は、上位バンク選択レジスタの値を除いた 6bit (下位バンク選択レジスタの値) になります。
	上位バンク切り替えレジスタの値は、以下のように保持します。

	* bit4: SRAMの書き換え可能モード
		* write_enable に保持
	* bit6: 512KB-1MBのSRAMを選択するモード
		* upper_bank_enable に保持

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include "ms_memmap.h"
#include "ms_memmap_ESE_SCC.h"
#include "../ms.h"

#define THIS ms_memmap_driver_ESE_SCC_t

static char* driver_name = "ESE_SCC";

static void _select_bank(THIS* d, int region, int segment);
static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);
static void _did_pause(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

static void _fflush(ms_memmap_driver_t* driver);

/*
	確保ルーチン
 */
THIS* ms_memmap_ESE_SCC_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_memmap_ESE_SCC_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, uint8_t* file_path) {
	if (instance == NULL) {
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// プロパティやメソッドの登録
	instance->base.type = ROM_TYPE_ESE_SCC;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_ESE_SCC_deinit; オーバーライド不要
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_pause = _did_pause;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	instance->base.buffer = (uint8_t*)buffer;
	instance->base.buffer_length = length;
	instance->num_segments = length / 0x2000;

	// プライベートプロパティ
	memcpy(instance->file_path, file_path, 256);

	int region;
	for(region = 0; region < 4; region++) {
		_select_bank(instance, region, region);	// KONAMI SCCメガロムの場合、初期値は0,1,2,3
	}

	// SCCの初期化
	int i;
	uint8_t* scc_segment;

	// セグメント番号63はSCCレジスタとして使用できるので、その領域を確保
	scc_segment = (uint8_t*)new_malloc( 8*1024 );
	if(scc_segment == NULL) {
		printf("メモリが確保できません。\n");
		return;
	}
	for (i = 0; i < 8*1024; i++) {
		scc_segment[i] = 0xff;
	}
	// init SCC registers
	for (i= 0x9800; i <= 0x98ff; i++) {
		scc_segment[i & 0x1fff] = 0;
	}
	instance->scc_segment = scc_segment;
	
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

/**
 * @brief ドライバのデタッチ時に呼び出されるコールバックで、SRAMをファイルに書き出します
 * 
 * @param driver 
 * @return int 
 */
static int _will_detach(ms_memmap_driver_t* driver) {
	MS_LOG(MS_LOG_INFO, "ESE-SCCのSRAMをファイルに書き出します\n");
	_fflush(driver);
	return 0;
}

/**
 * @brief ポーズ時に呼び出されるコールバック
 * 
 * @param driver 
 */
static void _did_pause(ms_memmap_driver_t* driver) {
	volatile uint8_t* BITSNS_WORK = (uint8_t*)0x800;
	if (BITSNS_WORK[0xe] & 1) {
		// ポーズ時に SHIFTキーが押されていたら、SRAMをファイルに書き出す
		_fflush(driver);
		printf("ESE-SCCのSRAMをファイルに書き出しました\n");
	}
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

/**
  @brief region (0-3)に見せるバンクを選択します

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

 */
static void _select_bank(THIS* d, int region, int segment) {
	uint8_t* buf;

	segment &= 0x3f;
	d->selected_segment[region] = segment;

	if( (region == 2) && (segment == 0x3f) ) {
		// SCC register
		buf = d->scc_segment;
	} else {
		if ( segment >= d->num_segments) {
			MS_LOG(MS_LOG_DEBUG,"ESE_SCC: segment out of range: %d\n", segment);
			buf = NULL;
		} else {
			if ( (region == 0) && d->upper_bank_enable && (d->num_segments == 128) ) {
				// 512KB-1MBのSRAMを選択
				buf = d->base.buffer + (segment * 0x2000) + 0x80000;
			} else {
				buf = d->base.buffer + (segment * 0x2000);
			}
		}
	}

	d->base.page8k_pointers[(region+2)&0x7] = buf;
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, (region+2)&0x7);	// 切り替えが起こったことを memmap に通知
}

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	int local_addr = addr & 0x1fff;

	uint8_t* p8k = driver->page8k_pointers[page8k];
	if( p8k == NULL ) {
		MS_LOG(MS_LOG_FINE,"ESE_SCC: read out of range: %04x\n", addr);
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

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	int area = addr >> 11;	// 4Kバイト単位のエリア番号

	if (area < 0x4*2 || area >= 0xc*2) {
		MS_LOG(MS_LOG_FINE,"ESE_SCC: write out of range: %04x\n", addr);
		return;
	}

	// 上位バンク切り替えレジスタ
	if( (addr & 0x7ffe) == 0x7ffe ) {
		d->upper_bank_enable = (data & 0x40) >> 6;
		d->write_enable = (data & 0x10) >> 4;
		_select_bank(d, 0, d->selected_segment[0]); // region 0 に影響があるので再選択する
		return;
	}

	// 0x4000-0x7ffd のSRAMへの書き込み
	if (d->write_enable && (area < 0x8*2) ) {
		int page8k = addr >> 13;
		int local_addr = addr & 0x1fff;
		uint8_t* p8k = driver->page8k_pointers[page8k];
		if( p8k == NULL ) {
			MS_LOG(MS_LOG_FINE,"ESE_SCC: write out of range: %04x\n", addr);
			return;
		}
		p8k[local_addr] = data;
		return;
	}

	// バンク切り替え処理とSCCレジスタへの書き込み
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
		if ((d->selected_segment[2] == 0x3f) && r_SCC_enable() ) {
			_write_scc_v1_reg(d, addr, data);
		}
		break;
	case 0xb*2:
		_select_bank(d, 3, data);
		break;
	}
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

/**
 * @brief ESE-SCCのSRAMをファイルに書き出します
 * 
 * @param instance 
 */
void _fflush(ms_memmap_driver_t* driver) {
	THIS* instance = (THIS*)driver;
	int crt_fh;
	int crt_length;
	uint8_t *crt_buff;
	int i;

	crt_fh = open( instance->file_path, O_RDWR | O_BINARY);
	if (crt_fh == -1) {
		MS_LOG(MS_LOG_ERROR,"SRAMファイルが開けません. %s\n", instance->file_path);
		return;
	}

	write(crt_fh, instance->base.buffer, instance->base.buffer_length);

	close(crt_fh);
}
