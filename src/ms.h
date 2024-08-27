/*

		‚l‚r‚w.‚r‚‰‚‚•‚Œ‚‚”‚…‚’ [[ MS ]]
				[[ ƒwƒbƒ_ƒtƒ@ƒCƒ‹ ]]
								
				prpgramed by Kuni.
										1995.12.06

*/
#ifndef MS_H
#define MS_H

#include "memmap/ms_memmap.h"

void* new_malloc( int );
void new_free( void * );

void ms_iocs_ledctrl(uint8_t type, uint8_t onoff);

extern uint8_t ms_peripherals_led_caps;
extern uint8_t ms_peripherals_led_kana;

#endif // MS_H