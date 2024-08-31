#ifndef MS_DISK_MEDIA_SECTORBASE_H
#define MS_DISK_MEDIA_SECTORBASE_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media_sectorbase ms_disk_media_sectorbase_t;

// 512バイトの配列を ms_sector_t として定義 (Cの定義の書き方が特殊なので注意)
typedef uint8_t ms_sector_t[512];

/**
 * @brief 円盤上のメディア(Disk Media)をセクターベースで実現する構造体です。
 * 
 * トラックのギャップ情報やCRCエラーなどの生の情報を扱わずに、セクターデータのみでメディアを扱う場合に使用します。
 * .DSKフォーマットのイメージを扱うためのサブ構造体(ms_disk_media_dskformat_t) などがその代表例です。
 * 
 */
typedef struct ms_disk_media_sectorbase {
	ms_disk_media_t base;
	// virtual methods
	void (*read_sector)(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
	void (*write_sector)(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
	// properties
	uint16_t sectors_per_track;
	uint16_t heads;
	uint16_t tracks;
} ms_disk_media_sectorbase_t;


ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc();
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance);
void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance);


void ms_disk_media_sectorbase_read_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_media_sectorbase_write_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);

#endif