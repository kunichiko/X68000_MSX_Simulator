#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_container.h"
#include "ms_disk_media_dskformat.h"
#include "../ms.h"

static uint8_t _read_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static uint8_t _write_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static void _flush_track(ms_disk_container_t* d);
static void _eject_disk(ms_disk_container_t* d);
static void _change_disk(ms_disk_container_t* d, int disk_no);
static uint8_t _is_disk_changed(ms_disk_container_t* d);

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
	instance->read_track = _read_track;
	instance->write_track = _write_track;
	instance->flush_track = _flush_track;
	instance->eject_disk = _eject_disk;
	instance->change_disk = _change_disk;
	instance->is_disk_changed = _is_disk_changed;

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
		instance->change_disk(instance, 0);		// 1枚目のディスクをセット
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


static uint8_t _read_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_t* current = d->current_disk;
	if (current == NULL) {
		return 0; // 失敗
	}
	current->read_track(current, track_no, side, raw_track);

	return 1; // 成功
}

static uint8_t _write_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_t* current = d->current_disk;
	if (current == NULL) {
		return 0; // 失敗
	}
	if (current->is_write_protected) {
		MS_LOG(MS_LOG_INFO, "Disk is write protected\n");
		return 0; // 失敗
	}
	current->write_track(current, track_no, side, raw_track);

	return 1; // 成功
}

static void _flush_track(ms_disk_container_t* d) {

}

static void _eject_disk(ms_disk_container_t* d) {
	d->current_disk = NULL;
	d->disk_changed = 1;
	MS_LOG(MS_LOG_INFO, "Disk ejected.\n");
}

static void _change_disk(ms_disk_container_t* d, int disk_no) {
	if (disk_no < 0 || disk_no >= d->disk_count) {
		MS_LOG(MS_LOG_INFO, "Unknown disk number: %d\n", disk_no);
		return;
	}
	ms_disk_media_t* disk = d->disk_set[disk_no];
	if (disk == NULL) {
		_eject_disk(d);
	} else {
		d->current_disk = disk;
		d->disk_changed = 1;
		MS_LOG(MS_LOG_INFO, "Disk changed to %d : \"%s\"\n", disk_no, disk->name);
	}
}

static uint8_t _is_disk_changed(ms_disk_container_t* d) {
	uint8_t ret = d->disk_changed;
	d->disk_changed = 0;
	return ret;
}
