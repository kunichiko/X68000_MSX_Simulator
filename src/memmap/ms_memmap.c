#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_memmap.h"

ms_memmap_t* ms_memmap_init() {
	ms_memmap_t* instance;
	instance = (ms_memmap_t*)new_malloc(sizeof(ms_memmap_t));
	if ( instance == NULL) {
		printf("メモリが確保できません\n");
		return NULL;
	}

	instance->main_mem = (uint8_t*)new_malloc(64 * 1024 + 8 * MS_MEMMAP_NUM_SEGMENTS); /* ６４Ｋ + ８バイト＊総セグメント数	*/
	if (instance->main_mem == NULL) { 
		printf("メモリが確保できません\n");
		new_free(instance);
		return NULL;
	}
	ms_memmap_set_main_mem(instance->main_mem, (int)MS_MEMMAP_NUM_SEGMENTS); /* アセンブラのルーチンへ引き渡し		*/

	ms_memmap_init_mac();

	return NULL;
}

void ms_memmap_deinit(ms_memmap_t* memmap) {
	if (memmap->main_mem == NULL) {
		return;
	}
	new_free(memmap->main_mem);
	new_free(memmap);

	ms_memmap_deinit_mac();
}