#ifndef MS_DISK_MEDIA_9SCDRV_H
#define MS_DISK_MEDIA_9SCDRV_H

#include <stdint.h>

#include "9scdrv/ms_disk_9scdrv.h"
#include "ms_disk.h"
#include "ms_disk_media.h"
#include "ms_disk_media_sectorbase.h"

typedef struct ms_disk_media_9scdrv ms_disk_media_9scdrv_t;

/**
 * @brief 9scdrv経由で実2DDメディアをms_disk_media_t で扱うための構造体です。
 *
 */
typedef struct ms_disk_media_9scdrv {
    ms_disk_media_sectorbase_t base;
    // methods

    // properties
    ms_disk_9scdrv_drive_t drive;
} ms_disk_media_9scdrv_t;

ms_disk_media_9scdrv_t* ms_disk_media_9scdrv_alloc();
int ms_disk_media_9scdrv_init(ms_disk_media_9scdrv_t* instance, ms_disk_9scdrv_drive_t drive);
void ms_disk_media_9scdrv_deinit(ms_disk_media_9scdrv_t* instance);

#endif