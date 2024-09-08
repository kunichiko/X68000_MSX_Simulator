#ifndef MS_RTC_H
#define MS_RTC_H

#include <stdint.h>
#include "../ms_iomap.h"

typedef struct ms_rtc {
	uint8_t regnum;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t block2[13];
	uint8_t block3[13];

	// I/O port アクセスを提供
	ms_ioport_t io_port_B4;
	ms_ioport_t io_port_B5;
} ms_rtc_t;

ms_rtc_t* ms_rtc_alloc();
void ms_rtc_init(ms_rtc_t* instance, ms_iomap_t* iomap);
void ms_rtc_deinit();

#endif