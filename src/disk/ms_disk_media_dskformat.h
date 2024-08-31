#ifndef MS_DISK_MEDIA_DSKFORMAT_H
#define MS_DISK_MEDIA_DSKFORMAT_H

#include <stdint.h>
#include "ms_disk.h"

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
} ms_disk_media_dskformat_t;

ms_disk_media_dskformat_t* ms_disk_media_dskformat_alloc();
void ms_disk_media_dskformat_init(ms_disk_media_dskformat_t* instance, char* file_path);
void ms_disk_media_dskformat_deinit(ms_disk_media_dskformat_t* instance);

#endif