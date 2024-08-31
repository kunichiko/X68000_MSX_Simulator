#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_container.h"

/*
	確保ルーチン
 */
ms_disk_container_t* ms_disk_container_init() {
	return (ms_disk_container_t*)new_malloc(sizeof(ms_disk_container_t));
}

/*
	初期化ルーチン
 */
void ms_disk_container_init(ms_disk_container_t* instance) {
	if (instance == NULL) {
		return;
	}
	// メソッドの登録
	instance->deinit = ms_disk_container_deinit;
	return instance;
}

void ms_disk_container_deinit(ms_disk_container_t* instance) {
}