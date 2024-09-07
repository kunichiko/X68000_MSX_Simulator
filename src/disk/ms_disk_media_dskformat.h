#ifndef MS_DISK_MEDIA_DSKFORMAT_H
#define MS_DISK_MEDIA_DSKFORMAT_H

#include <stdint.h>
#include "ms_disk.h"
#include "ms_disk_media.h"
#include "ms_disk_media_sectorbase.h"

typedef struct ms_disk_media_dskformat ms_disk_media_dskformat_t;

/**
 * @brief .DSK�`���̃f�B�X�N�C���[�W���Ams_disk_media_t �ň������߂̍\���̂ł��B
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

void ms_disk_media_dskformat_read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
void ms_disk_media_dskformat_write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);

#endif