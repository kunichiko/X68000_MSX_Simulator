#ifndef MS_IOMAP_H
#define MS_IOMAP_H

#include <stdint.h>
#include "vdp/ms_vdp.h"

typedef struct ms_iomap {
} ms_iomap_t;

ms_iomap_t* ms_iomap_init(ms_vdp_t* vdp);
void ms_iomap_deinit(ms_iomap_t* iomap);

#endif