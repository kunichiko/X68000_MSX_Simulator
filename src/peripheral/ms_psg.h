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

    // registers
    // r00-r14は ms_psg.has のアセンブラコード内に定義されている
    uint8_t r15;

    // オプション
    bool use_iocs;
    bool swap_AB;
} ms_psg_t;

// singleton instance
ms_psg_t* ms_psg_shared_instance();
void ms_psg_shared_init(ms_iomap_t* iomap, ms_init_params_t* init_param);
void ms_psg_shared_deinit(ms_iomap_t* iomap);

void ms_psg_set_ch_enable(int32_t enable);
int32_t ms_psg_get_ch_enable();

#endif