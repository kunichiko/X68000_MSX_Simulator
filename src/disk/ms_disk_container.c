#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_container.h"
#include "ms_disk_media_dskformat.h"

/*
	確保ルーチン
 */
ms_disk_container_t* ms_disk_container_alloc() {
	return (ms_disk_container_t*)new_malloc(sizeof(ms_disk_container_t));
}

/**
 * @brief ディスクコンテナを初期化します
 * 
 * @param instance 
 * @param argc ロードするディスクの数
 * @param argv ロードするディスクのパス (argc個の文字列)
 */
 void ms_disk_container_init(ms_disk_container_t* instance, int argc, char* argv[]) {
	if (instance == NULL) {
		return;
	}
	// メソッドの登録
	instance->deinit = ms_disk_container_deinit;
	instance->read_track = ms_disk_container_read_track;
	instance->write_track = ms_disk_container_write_track;
	instance->flush_track = ms_disk_container_flush_track;
	instance->change_disk = ms_disk_container_change_disk;
	instance->is_disk_changed = ms_disk_container_is_disk_changed;

	// ディスクのロード (最大16枚まで)
	if (argc > 0) {
		int i;
		for(i=0; i< min(argc,16); i++) {
			// ひとまず .DSK フォーマットのみ対応
			ms_disk_media_dskformat_t* disk = ms_disk_media_dskformat_alloc();
			ms_disk_media_dskformat_init(disk, argv[i]);
			instance->disk_set[i] = (ms_disk_media_t*)disk;
		}
		instance->disk_count = argc;
		instance->change_disk(instance, 0);
	}

	return;
}

void ms_disk_container_deinit(ms_disk_container_t* instance) {
	int i;
	for(i=0; i<instance->disk_count; i++) {
		ms_disk_media_deinit(instance->disk_set[i]);
		instance->disk_set[i] = NULL;
	}
	instance->current_disk = NULL;
	instance->disk_count = 0;
}


void ms_disk_container_read_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_t* current = d->current_disk;
	if (current == NULL) {
		return;
	}
	current->read_track(current, track_no, side, raw_track);
}

void ms_disk_container_write_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_t* current = d->current_disk;
	if (current == NULL) {
		return;
	}
	current->write_track(current, track_no, side, raw_track);
}

void ms_disk_container_flush_track(ms_disk_container_t* d) {

}

void ms_disk_container_change_disk(ms_disk_container_t* d, int disk_no) {
	if (disk_no < 0 || disk_no >= d->disk_count) {
		return;
	}
	d->current_disk = d->disk_set[disk_no];
	d->disk_changed = 1;
}

uint8_t ms_disk_container_is_disk_changed(ms_disk_container_t* d) {
	uint8_t ret = d->disk_changed;
	d->disk_changed = 0;
	return ret;
}
