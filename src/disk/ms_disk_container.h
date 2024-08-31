#ifndef MS_DISK_CONTAINER_H
#define MS_DISK_CONTAINER_H

#include <stdint.h>
#include "ms_disk.h"
#include "ms_disk_media.h"

typedef struct ms_disk_container ms_disk_container_t;

typedef struct ms_disk_container {
	void (*deinit)(ms_disk_container_t* drive);	// baseクラスだけ deinitを持つ (子クラスはオーバーライドする)
	// methods
	void (*read_track)(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*write_track)(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*flush_track)(ms_disk_container_t* d);
	void (*change_disk)(ms_disk_container_t* d, int disk_no);
	uint8_t (*is_disk_changed)(ms_disk_container_t* d);
	// properties
	ms_disk_media_t* current_disk;
	int disk_count;
	ms_disk_media_t* disk_set[16];	// 最大16枚まで
	// private prroperties
	uint8_t disk_changed;
} ms_disk_container_t;

ms_disk_container_t* ms_disk_container_alloc();
void ms_disk_container_init(ms_disk_container_t* instance, int argc, char* argv[]);
void ms_disk_container_deinit(ms_disk_container_t* instance);

void ms_disk_container_read_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_container_write_track(ms_disk_container_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_container_flush_track(ms_disk_container_t* d);
void ms_disk_container_change_disk(ms_disk_container_t* d, int disk_no);
uint8_t ms_disk_container_is_disk_changed(ms_disk_container_t* d);

#endif