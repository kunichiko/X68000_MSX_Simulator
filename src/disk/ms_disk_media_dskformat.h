#ifndef MS_DISK_MEDIA_DSKFORMAT_H
#define MS_DISK_MEDIA_DSKFORMAT_H

#include <stdint.h>

#include "ms_disk.h"
#include "ms_disk_media.h"
#include "ms_disk_media_sectorbase.h"

#define MS_DSK_SECTORS_PER_TRACK 9

typedef struct ms_disk_media_dskformat ms_disk_media_dskformat_t;

/**
 * @brief .DSK形式のディスクイメージを、ms_disk_media_t で扱うための構造体です。
 *
 */
typedef struct ms_disk_media_dskformat {
    ms_disk_media_sectorbase_t base;
    // methods

    // properties
    char* file_path;
    int file_handle;
    uint32_t file_size;
    /* トラック単位の読み込みバッファ。1 セクタごとに read を出すと
       ホスト側の入出力が効いて 1 回あたり数 ms かかるため、
       同じトラックの続きは貯めたものから返す。 */
    int32_t cached_track;   /* -1 ＝ 未読み込み */
    uint8_t track_buf[MS_DSK_SECTORS_PER_TRACK * 512];
} ms_disk_media_dskformat_t;

ms_disk_media_dskformat_t* ms_disk_media_dskformat_alloc();
int ms_disk_media_dskformat_init(ms_disk_media_dskformat_t* instance, char* file_path);
void ms_disk_media_dskformat_deinit(ms_disk_media_dskformat_t* instance);

#endif