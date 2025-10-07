#include "ms_disk_media_9scdrv.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x68k/dos.h>

#include "../ms.h"
#include "ms_disk.h"

#define THIS ms_disk_media_9scdrv_t

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
    // baseクラスの初期化
    ms_disk_media_sectorbase_init(&instance->base, "9scdrv");
    // メソッドの登録
    instance->base.base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_9scdrv_deinit;  // override
    instance->base.read_sector = _read_sector;                                             // override
    instance->base.write_sector = _write_sector;                                           // override

    // プロパティの初期化
    instance->base.sectors_per_track = 9;
    instance->base.heads = 2;
    instance->base.tracks = 80;
    instance->base.base.is_write_protected = 1;  // 書き込み禁止
    instance->drive = drive;
    return 1;
}

void ms_disk_media_9scdrv_deinit(THIS* instance) {
    ms_disk_media_sectorbase_deinit(&instance->base);
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
    const int sectors_per_track = 9;
    const int heads = 2;
    int track = ((sector_id - 1) / (sectors_per_track * heads));
    int side = ((sector_id - 1) / (sectors_per_track)) % heads;
    int secnum = ((sector_id - 1) % sectors_per_track) + 1;
    uint32_t position = convert_to_position(MS_DISK_9SCDRV_SECTOR_SIZE_512, track, side, secnum);

    // ms_disk_9scdrv_read_result_t ms_disk_9scdrv_read(ms_disk_9scdrv_drive_t pda, uint8_t mode, uint32_t position, uint32_t size,
    //                                                  ms_disk_9scdrv_media_byte_t media_byte, uint8_t* buffer);
    // printf("Reading sector_id=%d (track=%d side=%d secnum=%d) position=0x%08X\n", sector_id, track, side, secnum, position);
    ms_disk_9scdrv_read_result_t* result =
        ms_disk_9scdrv_read(dsk->drive, 0x70, position, 512, MS_DSK_9SCDRV_MEDIABYTE_2DD, (uint8_t*)sector);

    // printf("Read result: status=%08x, last_sector=%08x\n", result->status, result->last_sector);
#if 0
    int i;
    for (i = 0; i < 16; i++) {
        printf("%02X ", (*sector)[i]);
        if ((i & 0x0F) == 0x0F) {
            printf("\n");
        }
    }
#endif
}

static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
    return;  // 書き込み禁止にする
}
