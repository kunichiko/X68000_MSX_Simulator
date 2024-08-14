#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_iomap.h"
#include "vdp/ms_vdp.h"

ms_iomap_t* ms_iomap_init(ms_vdp_t* vdp) {
	ms_iomap_t* instance = (ms_iomap_t*)new_malloc(sizeof(ms_iomap_t));
	if (instance >= (ms_iomap_t*)0x81000000) {
		return NULL;
	}
	return instance;
}

void ms_iomap_deinit(ms_iomap_t* iomap) {
	new_free(iomap);
}