#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_media_sectorbase.h"


/*
	確保ルーチン
 */
ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc() {
	return (ms_disk_media_sectorbase_t*)new_malloc(sizeof(ms_disk_media_sectorbase_t));
}

/*
	初期化ルーチン
 */
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance) {
	if (instance == NULL) {
		return;
	}
	// baseクラスの初期化
	ms_disk_media_init(&instance->base);
	// メソッドの登録
	instance->base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_sectorbase_deinit; //override
	return;
}

void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance) {
	ms_disk_media_deinit(&instance->base);
}