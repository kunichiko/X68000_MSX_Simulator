#ifndef MS_DISK_DRIVE_H
#define MS_DISK_DRIVE_H

#include <stdint.h>

typedef struct ms_disk_drive ms_disk_drive_t;

typedef struct ms_disk_drive {
	void (*deinit)(ms_disk_drive_t* drive);	// baseクラスだけ deinitを持つ (個クラスはオーバーライドする)
	// methods
	uint8_t (*read_track)(ms_disk_drive_t* d, uint32_t offset);
	void (*write_track)(ms_disk_drive_t* d, uint32_t offset, uint8_t data);
	void (*flush_track)(ms_disk_drive_t* d);
	uint8_t (*is_disk_changed)(ms_disk_drive_t* d);
} ms_disk_drive_t;

ms_disk_drive_t* ms_disk_drive_alloc();
void ms_disk_drive_init(ms_disk_drive_t* instance);
void ms_disk_drive_deinit(ms_disk_drive_t* instance);

#endif