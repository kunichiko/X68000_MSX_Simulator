#ifndef MS_H
#define MS_H

/*

		‚l‚r‚w.‚r‚‰‚‚•‚Œ‚‚”‚…‚’ [[ MS ]]
				[[ ƒwƒbƒ_ƒtƒ@ƒCƒ‹ ]]
								
				prpgramed by Kuni.
										1995.12.06

*/

#include "memmap/ms_memmap.h"

#define MS_dot_X_VERSION	"0.4.8"

#ifdef DEBUG
#define MS_LOG(level, ...) do { if (debug_log_level >= level) printf(__VA_ARGS__); } while (0)
#else
#define MS_LOG(level, ...)
#endif
#define MS_LOG_NONE 0
#define MS_LOG_FATAL 1
#define MS_LOG_ERROR 2
#define MS_LOG_INFO 3
#define MS_LOG_DEBUG 4
#define MS_LOG_FINE 5
#define MS_LOG_TRACE 6
#define MS_LOG_CPUTRACE 7
#define MS_LOG_INFO_ENABLED (debug_log_level >= MS_LOG_INFO)
#define MS_LOG_DEBUG_ENABLED (debug_log_level >= MS_LOG_DEBUG)
#define MS_LOG_FINE_ENABLED (debug_log_level >= MS_LOG_FINE)
#define MS_LOG_TRACE_ENABLED (debug_log_level >= MS_LOG_TRACE)
#define MS_LOG_CPUTRACE_ENABLED (debug_log_level >= MS_LOG_CPUTRACE)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

void* new_malloc( int );
void new_free( void * );

void ms_iocs_ledctrl(uint8_t type, uint8_t onoff);

volatile extern unsigned short debug_log_level;

extern uint8_t ms_peripherals_led_caps;
extern uint8_t ms_peripherals_led_kana;

extern uint8_t ms_fdd_led_1;
extern uint8_t ms_fdd_led_2;

typedef struct ms_init_params {
	uint8_t* buf;
	uint8_t* mainrom;
	uint8_t* subrom;
	uint8_t* diskrom;
	uint8_t* kanjirom;
	uint8_t* cartridge_path_slot1;
	int cartridge_kind_slot1;
	uint8_t* cartridge_path_slot2;
	int cartridge_kind_slot2;
	uint8_t* slot_path[4][4];
	int diskcount;
	char* diskimages[16];
	uint32_t max_wait;
} ms_init_params_t;

#endif // MS_H