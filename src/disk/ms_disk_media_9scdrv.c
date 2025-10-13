#include "ms_disk_media_9scdrv.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x68k/dos.h>

#include "../ms.h"
#include "9scdrv/ms_disk_9scdrv.h"
#include "ms_disk.h"

#define THIS ms_disk_media_9scdrv_t

const int sectors_per_track = 9;
const int heads = 2;

static bool _setup_9scdrv(ms_disk_container_t* instance);
static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);

/*
        確保ルーチン
 */
THIS* ms_disk_media_9scdrv_alloc() {
    return (THIS*)new_malloc(sizeof(THIS));
}

int ms_disk_media_9scdrv_init(THIS* instance, ms_disk_9scdrv_drive_t drive) {
    if (instance == NULL) {
        return 0;
    }
    if (!_setup_9scdrv(NULL)) {
        return 0;
    }
    // baseクラスの初期化
    ms_disk_media_sectorbase_init(&instance->base, "9scdrv");
    // メソッドの登録
    instance->base.base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_9scdrv_deinit;  // override
    instance->base.read_sector = _read_sector;                                             // override
    instance->base.write_sector = _write_sector;                                           // override

    // プロパティの初期化
    instance->base.sectors_per_track = sectors_per_track;
    instance->base.heads = heads;
    instance->base.tracks = 80;
    instance->base.base.is_write_protected = 1;  // 書き込み禁止
    instance->drive = drive;
    int i;
    for (i = 0; i < MS_DISK_9SCDRV_CYLINDER_BUFFER_COUNT; i++) {
        instance->cylinder_buffer_info[i] = 0xFF;  // 未使用
    }
    return 1;
}

static bool _setup_9scdrv() {
    xkpchk_result_t result;
    result.table = NULL;
    ms_disk_9scdrv_init(&result);
    uint32_t minver = ('v' << 24) | ('3' << 16) | ('0' << 8) | ('0');
    printf("9SCDRV Version: %s, Revision: %s\n", (result.version != 0xffffffff) ? (char*)&result.version : "Not found",
           (result.revision != 0xffffffff) ? (char*)&result.revision : "N/A");
    if (result.version == 0xffffffff) {
        printf("9SCDRV V3 が常駐していません。\n");
        return false;
    }
    return true;
}

void ms_disk_media_9scdrv_deinit(THIS* instance) {
    ms_disk_media_sectorbase_deinit(&instance->base);
}

static uint8_t* _get_sector_buffer(THIS* dsk, int cylinder, int head, int record) {
    static int last_cylinder = 0;
    int i;
    for (i = 0; i < MS_DISK_9SCDRV_CYLINDER_BUFFER_COUNT; i++) {
        if (dsk->cylinder_buffer_info[i] == cylinder) {
            return dsk->cylinder_buffer[i][head * sectors_per_track + record - 1];
        }
    }
    // バッファにない場合、トラック単位 (9セクタ単位) で読み込む
    int buffer_index = -1;
    for (i = 0; i < MS_DISK_9SCDRV_CYLINDER_BUFFER_COUNT; i++) {
        if (dsk->cylinder_buffer_info[i] == 0xFF) {
            buffer_index = i;
            break;
        }
    }
    if (buffer_index == -1) {
        buffer_index = (last_cylinder + 1) % MS_DISK_9SCDRV_CYLINDER_BUFFER_COUNT;
    }
    uint32_t position = convert_to_position(MS_DISK_9SCDRV_SECTOR_SIZE_512, cylinder, 0, 1);
    ms_disk_9scdrv_read_result_t* result =
        ms_disk_9scdrv_read(dsk->drive, 0x70, position, heads * sectors_per_track * 512, MS_DSK_9SCDRV_MEDIABYTE_2DD,
                            (uint8_t*)dsk->cylinder_buffer[buffer_index][0]);

    last_cylinder = buffer_index;
    dsk->cylinder_buffer_info[buffer_index] = cylinder;
    return dsk->cylinder_buffer[buffer_index][head * sectors_per_track + record - 1];
}

/**
 * @brief セクターを読み込みます。
 *
 * @param instance
 * @param sector_id 1から始まることに注意
 * @param sector
 */
static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
    THIS* dsk = (THIS*)instance;
    int cylinder = ((sector_id - 1) / (sectors_per_track * heads));
    int head = ((sector_id - 1) / (sectors_per_track)) % heads;
    int record = ((sector_id - 1) % sectors_per_track) + 1;

    printf("Read sector=%d (C=%d H=%d R=%d)\n", sector_id, cylinder, head, record);
    uint8_t* buf = _get_sector_buffer(dsk, cylinder, head, record);
    if (buf == NULL) {
        // 読み込み失敗
        memset(sector, 0xE5, 512);  // 0xE5で埋める
        return;
    }
    memcpy(sector, buf, 512);
}

static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
    return;  // 書き込み禁止にする
}
