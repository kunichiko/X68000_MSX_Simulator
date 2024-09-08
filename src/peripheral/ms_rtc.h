#ifndef MS_RTC_H
#define MS_RTC_H

#include <stdint.h>

typedef struct ms_rtc {
	uint8_t regnum;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t block2[13];
	uint8_t block3[13];
} ms_rtc_t;

ms_rtc_t* ms_rtc_shared_instance();
void ms_rtc_shared_deinit();


// I/O port
void write_rtc_B4(uint8_t port, uint8_t data);
uint8_t read_rtc_B4(uint8_t port);
void write_rtc_B5(uint8_t port, uint8_t data);
uint8_t read_rtc_B5(uint8_t port);

#endif