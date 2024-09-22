#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "ms_psg.h"

int ms_psg_init_mac(uint8_t* psg2octnote, uint8_t* psg2kf);
void ms_psg_deinit_mac(void);

uint8_t r_port_A0();
uint8_t r_port_A2();
void w_port_A0(uint8_t data);
void w_port_A1(uint8_t data);

static void _write_psg_A0(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A0(ms_ioport_t* ioport, uint8_t port);
static void _write_psg_A1(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A1(ms_ioport_t* ioport, uint8_t port);
static void _write_psg_A2(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A2(ms_ioport_t* ioport, uint8_t port);

// Singleton instance
static ms_psg_t* _shared = NULL;
static initialized = 0;

ms_psg_t* ms_psg_shared_instance() {
	if( _shared != NULL) {
		return _shared;
	}
	if ( (_shared = (ms_psg_t*)new_malloc(sizeof(ms_psg_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"メモリが確保できません\n");
		return NULL;
	}
	return _shared;
}

void ms_psg_shared_init(ms_iomap_t* iomap) {
	if (initialized) {
		return;
	}
	initialized = 1;

	// I/O port アクセスを提供
	_shared->io_port_A0.instance = _shared;
	_shared->io_port_A0.read = _read_psg_A0;
	_shared->io_port_A0.write = _write_psg_A0;
	ms_iomap_attach_ioport(iomap, 0xa0, &_shared->io_port_A0);

	_shared->io_port_A1.instance = _shared;
	_shared->io_port_A1.read = _read_psg_A1;
	_shared->io_port_A1.write = _write_psg_A1;
	ms_iomap_attach_ioport(iomap, 0xa1, &_shared->io_port_A1);

	_shared->io_port_A2.instance = _shared;
	_shared->io_port_A2.read = _read_psg_A2;
	_shared->io_port_A2.write = _write_psg_A2;
	ms_iomap_attach_ioport(iomap, 0xa2, &_shared->io_port_A2);

	// PSGの分周パラメータnからOPMのoct,note,kfに変換するテーブル
	// PSGの分周パラメータをnとすると、PSGの周波数は、Fpsg = 3579545 / (32 * n) [Hz]となる
	// OPMの周波数はオクターブ(oct)、ノート(note)、キーフラクション(kf)で表される。
	// OPMが 3.579545MHzで駆動されている場合、oct=4, note=8, kf=0 で 440HzのA音が出る。
	// 式で表すと Fopm = 440 * 2^(oct-4 + (n-8)/12 + kf/12/64) [Hz]
	// であるば、X68000のOPMは 4MHzで駆動されているため、
	// Fopm = 440 * 2^(oct-4 + (n-8)/12 + kf/12/64) * 4/3.579545 [Hz]
	// となる。
	// 以下、PSGの分周パラメータnからOPMのoct,note,kfに変換するテーブルを作成する。
	// Fopm = Fpsg となるように、oct,note,kfを求める必要があるので、まずは両者の等式を立てると、
	// 440 * 2^(oct-4 + (note-8)/12 + kf/12/64) * 4/3.579545 = 3579545 / (32 * n)
	// となる。これを整理すると、
	// 2^(oct-4 + (note-8)/12 + kf/12/64) = 3579545 / (32 * n) * 3.579545 / 4 / 440
	// 2^(oct-4 + (note-8)/12 + kf/12/64) = 227.50607967018821 / n
	// oct-4 + (note-8)/12 + kf/12/64 = log2(227.50607967018821 / n)
	// となる。
	// O = (oct-4)
	// N = (note-8)
	// K = kf
	// とすると、
	// O*12*64 + N*64 + K = log2(227.50607967018821 / n) * 12 * 64
	// となる。ここから、
	// O→oct→N→note→Kの順に求めていく。
	// まず、N=0, K=0として、右辺を超えない最大の octを求める。(Oは負になるが、octは0以上)
	// Oが決まると、
	// N*64 + K = (log2(227.50607967018821 / n) - O ) * 12 * 64
	// N*64 + K = (log2(227.50607967018821 / n) + 4 - oct) * 12 * 64
	// となるので、次は、K=0として、右辺を超えない最大の noteを求める。(Nは負になるが、noteは0以上)
	// (note-8)*64 = (log2(227.50607967018821 / n) + 4 - oct) * 12 * 64
	// note-8 = (log2(227.50607967018821 / n) + 4 - oct) * 12
	// note = (log2(227.50607967018821 / n) + 4 - oct) * 12 + 8
	// oct, noteが確定すると、Kは
	// K = (log2(227.50607967018821 / n) + 4 - oct - (note-8)/12 ) * 12 * 64
	// K = ((log2(227.50607967018821 / n) + 4 - oct) * 12 - note+8 ) * 64
	// となるので、右辺を四捨五入して整数化してKを求める。
	// 最後に、noteが12以上になるケースがあるので、note >= 12 の場合は Oを1つ増やして、noteを12減らす。
	// これで、PSGの分周パラメータnからOPMのoct,note,kfに変換するテーブルが作成できる。

	uint8_t n2value[] = {0,1,2,4,5,6,8,9,0xa,0xc,0xd,0xe};
	int n;
	_shared->psg2octnote[0] = 7<<4 | n2value[11];
	for(n=1;n<4096;n++) {
		double log2n = log2(227.50607967018821 / n);
		int oct  = (int)(  log2n + 4);
		int note = (int)( (log2n + 4 - oct) * 12 + 8);
		int K    = (int)(((log2n + 4 - oct) * 12 - note + 8) * 64 + 0.5);
		if (K > 63) {
			// 誤差で 64 になる場合があるので、調整
			K = 63;
		}
		if (note >= 12) {
			oct++;
			note -= 12;
		}
		if ( oct < 0 ) {
			oct = 0;
			note = 0;
			K = 0;
		} else if ( oct > 7) {
			oct = 7;
			note = 11;
			K = 63;
		}
		_shared->psg2octnote[n] = (oct<<4) | n2value[note];
		_shared->psg2kf[n] = K;
		MS_LOG(MS_LOG_TRACE, "n=%d, Oct=%d, Note=%d, Kf=%d, octnote=%02x, kf=%02x\n", n, oct, note, K, _shared->psg2octnote[n], _shared->psg2kf[n]);
	}

	// アセンブラルーチンの初期化
	ms_psg_init_mac(_shared->psg2octnote, _shared->psg2kf);
}

void ms_psg_shared_deinit(ms_iomap_t* iomap) {
	if (_shared == NULL) {
		return;
	}
	ms_iomap_detach_ioport(iomap, 0xa0);
	ms_iomap_detach_ioport(iomap, 0xa1);
	ms_iomap_detach_ioport(iomap, 0xa2);

	// アセンブラルーチンの終了処理
	ms_psg_deinit_mac();

	// シングルトンの場合は deinitで freeする
	new_free(_shared);
	_shared = NULL;
}

static void _write_psg_A0(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	w_port_A0(data);
}

static uint8_t _read_psg_A0(ms_ioport_t* ioport, uint8_t port) {
	return r_port_A0();
}

static void _write_psg_A1(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	w_port_A1(data);
}

static uint8_t _read_psg_A1(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static void _write_psg_A2(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
}

static uint8_t _read_psg_A2(ms_ioport_t* ioport, uint8_t port) {
	return r_port_A2();
}
