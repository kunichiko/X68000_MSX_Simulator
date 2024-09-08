#ifndef MS_KANJIROM12_H
#define MS_KANJIROM12_H

#include <stdint.h>
#include "../ms_iomap.h"

typedef struct ms_kanjirom12 {
	char* rom_path;
	uint8_t* rom_data;
	uint32_t rom_size;

	// 第一水準アドレス
	uint32_t addr1;
	// 第二水準アドレス
	uint32_t addr2;

	// I/O port アクセスを提供
	ms_ioport_t io_port_D8;
	ms_ioport_t io_port_D9;
} ms_kanjirom12_t;

ms_kanjirom12_t* ms_kanjirom12_alloc();
void ms_kanjirom12_init(ms_kanjirom12_t* instance, ms_iomap_t* iomap, char* rom_path);
void ms_kanjirom12_deinit(ms_kanjirom12_t* instance, ms_iomap_t* iomap);

#endif