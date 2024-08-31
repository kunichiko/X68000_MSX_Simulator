#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive.h"

/*
	確保ルーチン
 */
ms_disk_drive_t* ms_disk_drive_init(char* file_path) {
	return (ms_disk_drive_t*)new_malloc(sizeof(ms_disk_drive_t));
}

/*
	初期化ルーチン
 */
void ms_disk_drive_init(ms_disk_drive_t* instance, char* file_path) {
	if (instance == NULL) {
		return;
	}
	// メソッドの登録
	instance->deinit = ms_disk_drive_deinit;
	return instance;
}

void ms_disk_drive_deinit(ms_disk_drive_t* instance) {
}