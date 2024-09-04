#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_iomap.h"
#include "vdp/ms_vdp.h"

ms_iomap_t* ms_iomap_alloc() {
	return (ms_iomap_t*)new_malloc(sizeof(ms_iomap_t));
}

void ms_iomap_init(ms_iomap_t* instance, ms_vdp_t* vdp) {
}

void ms_iomap_deinit(ms_iomap_t* instance) {
}