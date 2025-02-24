#ifndef MS_PSG_JOY_H
#define MS_PSG_JOY_H

#include <stdint.h>
#include "ms_psg.h"

uint8_t ms_psg_read_R14(ms_psg_t* shared);
uint8_t ms_psg_read_R15(ms_psg_t* shared);

void ms_psg_write_R15(ms_psg_t* shared, uint8_t data);

#endif