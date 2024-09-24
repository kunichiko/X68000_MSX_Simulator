#ifndef MS_SCC_H
#define MS_SCC_H

#include <stdint.h>

void w_SCC_freq(int32_t channel, int32_t freq);
void w_SCC_volume(int32_t channel, int32_t volume);
void w_SCC_keyon(int32_t enable);
void w_SCC_enable(int32_t enable);
int32_t r_SCC_enable();
void w_SCC_deformation(int32_t deformation);

#endif