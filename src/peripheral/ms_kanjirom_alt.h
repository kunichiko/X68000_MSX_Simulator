#ifndef MS_KANJIROM_ALT_H
#define MS_KANJIROM_ALT_H

#include <stdint.h>
#include "../ms_iomap.h"

typedef struct ms_kanjirom_alt {
	char* rom_path;

	// 第一水準漢字番号
	uint16_t k1_num;
	// 第一水準 行番号(左上、右上、左下、右下の順に0-31)
	uint8_t k1_line;

	// 第二水準漢字番号
	uint16_t k2_num;
	// 第一水準 行番号(左上、右上、左下、右下の順に0-31)
	uint8_t k2_line;

	// I/O port アクセスを提供
	ms_ioport_t io_port_D8;
	ms_ioport_t io_port_D9;
} ms_kanjirom_alt_t;

ms_kanjirom_alt_t* ms_kanjirom_alt_alloc();
void ms_kanjirom_alt_init(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap);
void ms_kanjirom_alt_deinit(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap);

#endif