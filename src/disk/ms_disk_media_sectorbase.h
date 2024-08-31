#ifndef MS_DISK_MEDIA_SECTORBASE_H
#define MS_DISK_MEDIA_SECTORBASE_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media_sectorbase ms_disk_media_sectorbase_t;

/**
 * @brief 円盤上のメディア(Disk Media)をセクターベースで実現する構造体です。
 * 
 * トラックのギャップ情報やCRCエラーなどの生の情報を扱わずに、セクターデータのみでメディアを扱う場合に使用します。
 * .DSKフォーマットのイメージを扱うためのサブ構造体(ms_disk_media_dskformat_t) などがその代表例です。
 * 
 */
typedef struct ms_disk_media_sectorbase {
	ms_disk_media_t base;
	// methods

} ms_disk_media_sectorbase_t;


ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc();
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance);
void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance);

#endif