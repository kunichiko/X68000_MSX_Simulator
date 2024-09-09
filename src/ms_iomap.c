#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_iomap.h"
#include "vdp/ms_vdp.h"

#define THIS ms_iomap_t

static THIS* _shared = NULL;

THIS* ms_iomap_shared_instance() {
	_shared = (THIS*)new_malloc(sizeof(ms_iomap_t));
	if (_shared == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}
	int i;
	for(i=0;i<256;i++) {
		_shared->ioport_table[i] = NULL;
	}
	return _shared;
}

void ms_iomap_shared_deinit() {
	if (_shared == NULL) {
		return;
	}
	// シングルトンの場合は deinitで freeする
	new_free(_shared);
	_shared = NULL;
}

void ms_iomap_attach_ioport(ms_iomap_t* instance, uint8_t port, ms_ioport_t* ioport) {
	instance->ioport_table[port] = ioport;
}

void ms_iomap_detach_ioport(ms_iomap_t* instance, uint8_t port) {
	instance->ioport_table[port] = NULL;
}

void ms_iomap_out(uint8_t port, uint8_t data) {
	ms_ioport_t* ioport = _shared->ioport_table[port];
	if (ioport == NULL) {
		int loglevel = MS_LOG_FINE;
		switch(port) {
			case 0xac: // MSX-ENGINE
			case 0xad:
			case 0xae:
			case 0xaf:
			case 0xb8: // ライトペン?
			case 0xb9:
			case 0xba:
			case 0xbb:
			case 0xf4: // System flags
			case 0xf5: // System control
				loglevel = MS_LOG_TRACE;
				break;
		}
		MS_LOG(loglevel, "未対応ポート Wr 0x%02x\n", port);
		return;
	}
	ioport->write(ioport, port, data);
}

uint8_t ms_iomap_in(uint8_t port) {
	ms_ioport_t* ioport = _shared->ioport_table[port];
	if (ioport == NULL) {
		int loglevel = MS_LOG_FINE;
		switch(port) {
			case 0xac: // MSX-ENGINE
			case 0xad:
			case 0xae:
			case 0xaf:
			case 0xb8: // ライトペン?
			case 0xb9:
			case 0xba:
			case 0xbb:
			case 0xf4: // System flags
			case 0xf5: // System control
				loglevel = MS_LOG_TRACE;
				break;
		}
		MS_LOG(loglevel, "未対応ポート Rd 0x%02x\n", port);
		return 0xff;
	}
	return ioport->read(ioport, port);
}