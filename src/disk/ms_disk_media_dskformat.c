#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_media_dskformat.h"

/*
	確保ルーチン
 */
ms_disk_media_dskformat_t* ms_disk_media_dskformat_alloc() {
	return (ms_disk_media_dskformat_t*)new_malloc(sizeof(ms_disk_media_dskformat_t));
}

void ms_disk_media_dskformat_init(ms_disk_media_dskformat_t* instance, char* file_path) {
	if (instance == NULL) {
		return;
	}
	// baseクラスの初期化
	ms_disk_media_sectorbase_init(&instance->base);
	// メソッドの登録
	instance->base.base.deinit =  (void (*)(ms_disk_media_t*))ms_disk_media_dskformat_deinit; //override
	//
	instance->file_path = file_path;
	instance->file_handle = open(file_path, O_RDONLY | O_BINARY); // ひとまずリードオンリー
	if (instance->file_handle == -1) {
		printf("ファイルが開けません. %s\n", file_path);
		ms_exit();
		return;
	}
	instance->file_size = filelength(instance->file_handle);
	return;
}

void ms_disk_media_dskformat_deinit(ms_disk_media_dskformat_t* instance) {
	ms_disk_media_sectorbase_deinit(&instance->base);
	close(instance->file_handle);
}