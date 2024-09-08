#ifndef MS_IOMAP_H
#define MS_IOMAP_H

#include <stdint.h>
#include "vdp/ms_vdp.h"

/**
 * @brief I/Oポートの読み書きを行うモジュールが実装する構造体
 * 
 */
typedef struct ms_ioport ms_ioport_t;

typedef struct ms_ioport {
	// offset +0
	void* instance;
	// offset +4
	uint8_t (*read)(ms_ioport_t* ioport, uint8_t port);
	// offset +8
	void (*write)(ms_ioport_t* ioport, uint8_t port, uint8_t data);
} ms_ioport_t;

typedef struct ms_iomap {
	ms_ioport_t* ioport_table[256];
} ms_iomap_t;


// singleton instance
ms_iomap_t* ms_iomap_shared_instance();
void ms_iomap_shared_deinit();

// functions
void ms_iomap_attach_ioport(ms_iomap_t* instance, uint8_t port, ms_ioport_t* ioport);
void ms_iomap_detach_ioport(ms_iomap_t* instance, uint8_t port);

void ms_iomap_out(uint8_t port, uint8_t data);
uint8_t ms_iomap_in(uint8_t port);

#endif