#ifndef MS_PSG_H
#define MS_PSG_H

#include <stdint.h>

#include "../ms_iomap.h"

typedef struct ms_psg {
	// PSGの分周パラメータnからOPMのoct,note,kfに変換するテーブル
	uint8_t psg2octnote[4096];
	uint8_t psg2kf[4096];

	// I/O port アクセスを提供
	ms_ioport_t io_port_A0;
	ms_ioport_t io_port_A1;
	ms_ioport_t io_port_A2;
} ms_psg_t;

// singleton instance
ms_psg_t* ms_psg_shared_instance();
void ms_psg_shared_deinit(ms_iomap_t* iomap);

#endif