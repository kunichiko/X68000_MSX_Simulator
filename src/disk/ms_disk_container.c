#include "ms_disk_container.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ms.h"
#include "ms_disk.h"
#include "ms_disk_media_9scdrv.h"
#include "ms_disk_media_dskformat.h"

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
void ms_disk_container_init(ms_disk_container_t* instance, int dskimage_count, char* dskimage_paths[],
                            ms_disk_9scdrv_drive_t drive_for_9scdrv) {
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
    if (dskimage_count > 0) {
        int i;
        for (i = 0; i < min(dskimage_count, 16); i++) {
            // ひとまず .DSK フォーマットのみ対応
            ms_disk_media_dskformat_t* disk = ms_disk_media_dskformat_alloc();
            if (ms_disk_media_dskformat_init(disk, dskimage_paths[i])) {
                instance->disk_set[i] = (ms_disk_media_t*)disk;
            }
        }
        instance->disk_count = dskimage_count;
        instance->change_disk(instance, 0);  // 1枚目のディスクをセット
    }

    // 9scdrvドライブの初期化
    instance->disk_9scdrv = NULL;
    if (drive_for_9scdrv != MS_DISK_9SCDRV_NONE) {
        instance->disk_9scdrv = ms_disk_media_9scdrv_alloc();
        if (instance->disk_9scdrv != NULL) {
            if (ms_disk_media_9scdrv_init(instance->disk_9scdrv, drive_for_9scdrv) == 0) {
                // 初期化失敗
                free(instance->disk_9scdrv);
                instance->disk_9scdrv = NULL;
                printf("9scdrv drive initialization failed.\n");
            } else {
                printf("9scdrv drive initialized.\n");
                instance->change_disk(instance, -1);  // 9scdrvに切り替え
            }
        }
    }

    return;
}

void ms_disk_container_deinit(ms_disk_container_t* instance) {
    int i;
    for (i = 0; i < instance->disk_count; i++) {
        ms_disk_media_deinit(instance->disk_set[i]);
        instance->disk_set[i] = NULL;
    }
    instance->current_disk = NULL;
    instance->disk_count = 0;
}

static uint8_t _read_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
    ms_disk_media_t* current = d->current_disk;
    if (current == NULL) {
        return 0;  // 失敗
    }
    current->read_track(current, track_no, side, raw_track);

    return 1;  // 成功
}

static uint8_t _write_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
    ms_disk_media_t* current = d->current_disk;
    if (current == NULL) {
        return 0;  // 失敗
    }
    if (current->is_write_protected) {
        MS_LOG(MS_LOG_INFO, "Disk is write protected\n");
        return 0;  // 失敗
    }
    current->write_track(current, track_no, side, raw_track);

    return 1;  // 成功
}

static void _flush_track(ms_disk_container_t* d) {
}

static void _eject_disk(ms_disk_container_t* d) {
    d->current_disk = NULL;
    d->disk_changed = 1;
    MS_LOG(MS_LOG_INFO, "Disk ejected2.\n");
}

static void _change_disk(ms_disk_container_t* d, int disk_no) {
    if (disk_no < -1 || disk_no >= d->disk_count) {
        MS_LOG(MS_LOG_INFO, "Unknown disk number: %d\n", disk_no);
        return;
    }
    MS_LOG(MS_LOG_INFO, "Changing disk to: %d\n", disk_no);
    if (disk_no == -1) {
        // 9scdrvに切り替え
        if (d->disk_9scdrv == NULL) {
            printf("9scdrv drive is not available.\n");
        } else {
            d->current_disk = &d->disk_9scdrv->base.base;
            d->disk_changed = 1;
            MS_LOG(MS_LOG_INFO, "Disk changed to 9scdrv drive.\n");
        }
    } else {
        ms_disk_media_t* disk = d->disk_set[disk_no];
        if (disk == NULL) {
            _eject_disk(d);
        } else {
            d->current_disk = disk;
            d->disk_changed = 1;
            MS_LOG(MS_LOG_INFO, "Disk changed to %d : \"%s\"\n", disk_no, disk->name);
        }
    }
}

static uint8_t _is_disk_changed(ms_disk_container_t* d) {
    uint8_t ret = d->disk_changed;
    d->disk_changed = 0;
    return ret;
}
