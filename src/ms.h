/*

		‚l‚r‚w.‚r‚‰‚‚•‚Œ‚‚”‚…‚’ [[ MS ]]
				[[ ƒwƒbƒ_ƒtƒ@ƒCƒ‹ ]]
								
				prpgramed by Kuni.
										1995.12.06

*/
#ifndef MS_H
#define MS_H

#include "memmap/ms_memmap.h"

#ifdef DEBUG
#define MS_LOG(level, ...) do { if (debug_log_level >= level) printf(__VA_ARGS__); } while (0)
#else
#define MS_LOG(level, ...)
#endif
#define MS_LOG_NONE 0
#define MS_LOG_ERROR 1
#define MS_LOG_INFO 1
#define MS_LOG_DEBUG 2
#define MS_LOG_TRACE 3
#define MS_LOG_INFO_ENABLED (debug_log_level >= MS_LOG_INFO)
#define MS_LOG_DEBUG_ENABLED (debug_log_level >= MS_LOG_DEBUG)
#define MS_LOG_TRACE_ENABLED (debug_log_level >= MS_LOG_TRACE)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void* new_malloc( int );
void new_free( void * );

void ms_iocs_ledctrl(uint8_t type, uint8_t onoff);

volatile extern unsigned short debug_log_level;

extern uint8_t ms_peripherals_led_caps;
extern uint8_t ms_peripherals_led_kana;

#endif // MS_H