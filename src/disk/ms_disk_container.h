#ifndef MS_DISK_CONTAINER_H
#define MS_DISK_CONTAINER_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_container ms_disk_container_t;

typedef struct ms_disk_container {
	void (*deinit)(ms_disk_container_t* drive);	// baseクラスだけ deinitを持つ (個クラスはオーバーライドする)
	// methods
	ms_disk_media_t* (*get_inserted_disk)(ms_disk_container_t* d);
	void (*write_track)(ms_disk_container_t* d, uint32_t offset, uint8_t data);
	void (*flush_track)(ms_disk_container_t* d);
	uint8_t (*is_disk_changed)(ms_disk_container_t* d);
} ms_disk_container_t;

ms_disk_container_t* ms_disk_container_alloc();
void ms_disk_container_init(ms_disk_container_t* instance);
void ms_disk_container_deinit(ms_disk_container_t* instance);

#endif