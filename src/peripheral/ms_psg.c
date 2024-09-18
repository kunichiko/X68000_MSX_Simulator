#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_psg.h"

int ms_psg_init_mac(void);
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

	// アセンブラルーチンの初期化
	ms_psg_init_mac();
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