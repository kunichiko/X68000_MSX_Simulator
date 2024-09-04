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
	ms_disk_media_sectorbase_init(&instance->base, file_path);
	// メソッドの登録
	instance->base.base.deinit =  (void (*)(ms_disk_media_t*))ms_disk_media_dskformat_deinit; //override
	instance->base.read_sector = ms_disk_media_dskformat_read_sector; //override
	instance->base.write_sector = ms_disk_media_dskformat_write_sector; //override

	// プロパティの初期化
	instance->base.base.is_write_protected = 1; // デフォルトはライトプロテクト
	instance->file_path = file_path;
	instance->file_handle = open(file_path, O_RDONLY | O_BINARY); // ひとまずリードオンリー
	if (instance->file_handle == -1) {
		printf("ファイルが開けません. %s\n", file_path);
		ms_exit();
		return;
	}
	uint32_t file_size = filelength(instance->file_handle);

	instance->file_size	= file_size;
	instance->base.sectors_per_track = 9;
	instance->base.heads = file_size <= 512*9*80 ? 1 : 2;
	instance->base.tracks = (file_size / instance->base.heads) / 9 / 512;
	return;
}

void ms_disk_media_dskformat_deinit(ms_disk_media_dskformat_t* instance) {
	ms_disk_media_sectorbase_deinit(&instance->base);
	close(instance->file_handle);
}

/**
 * @brief セクターを読み込みます。
 * 
 * @param instance 
 * @param sector_id 1から始まることに注意
 * @param sector 
 */
void ms_disk_media_dskformat_read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	int index = sector_id - 1;
	ms_disk_media_dskformat_t* dsk = (ms_disk_media_dskformat_t*)instance;
	uint32_t offset = index * 512;
	lseek(dsk->file_handle, offset, SEEK_SET);
	read(dsk->file_handle, sector, 512);
}

void ms_disk_media_dskformat_write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	// write not supported
}
