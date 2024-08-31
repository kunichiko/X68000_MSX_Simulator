#ifndef MS_DISK_MEDIA_H
#define MS_DISK_MEDIA_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media ms_disk_media_t;

/**
 * @brief 円盤上のメディア(Disk Media)を表す構造体です。
 * 
 * ms_disk_rawtrack_t で表される円盤上のトラックを使ってアクセスします。トラック単位なので、
 * そこからセクタに分解する処理はコントローラー (ms_disk_controller_t) の責務です。
 * 
 */
typedef struct ms_disk_media {
	// methods
	void (*deinit)(ms_disk_media_t* media);
	void (*read_track)(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*write_track)(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);

} ms_disk_media_t;

ms_disk_media_t* ms_disk_media_alloc();
void ms_disk_media_init(ms_disk_media_t* instance);
void ms_disk_media_deinit(ms_disk_media_t* instance);

#endif