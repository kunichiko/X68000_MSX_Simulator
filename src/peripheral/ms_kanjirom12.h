#ifndef MS_KANJIROM12_H
#define MS_KANJIROM12_H

#include <stdint.h>

typedef struct ms_kanjirom12 {
	char* rom_path;
	uint8_t* rom_data;
	uint32_t rom_size;

	// 第一水準アドレス
	uint32_t addr1;
	// 第二水準アドレス
	uint32_t addr2;
} ms_kanjirom12_t;

ms_kanjirom12_t* ms_kanjirom12_alloc();
void ms_kanjirom12_init(ms_kanjirom12_t* instance, char* rom_path);
void ms_kanjirom12_deinit(ms_kanjirom12_t* instance);


// I/O port

// 第一水準
void write_kanji_D8(uint8_t port, uint8_t data);
void write_kanji_D9(uint8_t port, uint8_t data);
uint8_t read_kanji_D9(uint8_t port); 

// 第二水準
void write_kanji_DA(uint8_t port, uint8_t data);
void write_kanji_DB(uint8_t port, uint8_t data);
uint8_t read_kanji_DB(uint8_t port); 

#endif