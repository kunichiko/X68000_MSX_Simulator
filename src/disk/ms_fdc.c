#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_fdc.h"

// Singleton instance
ms_fdc_t*  ms_fdc_shared = NULL;


ms_fdc_t* ms_fdc_init() {
	if( ms_fdc_shared != NULL ) {
		return ms_fdc_shared;
	}
	if ( (ms_fdc_shared = (ms_fdc_t*)new_malloc(sizeof(ms_fdc_t))) == NULL)
	{
		printf("メモリが確保できません\n");
		return NULL;
	}
	return ms_fdc_shared;
}

void ms_fdc_deinit(ms_fdc_t* fdc) {
}

/*
  TC8566AFの I/O エミュレーション
*/

