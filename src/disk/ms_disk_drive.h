#ifndef MS_DISK_DRIVE_H
#define MS_DISK_DRIVE_H

#include <stdint.h>

typedef struct ms_disk_drive ms_disk_drive_t;

typedef void (*ms_disk_drive_deinit_t)(ms_disk_drive_t* drive);
typedef void (*ms_disk_drive_read_track_t)(ms_disk_drive_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
typedef void (*ms_disk_drive_write_track_t)(ms_disk_drive_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
typedef void (*ms_disk_drive_flush_track_t)(ms_disk_drive_t* d);
typedef uint8_t (*ms_disk_drive_is_disk_changed_t)(ms_disk_drive_t* d);

typedef struct ms_disk_drive {
	void (*deinit)(ms_disk_drive_t* drive);	// baseクラスだけ deinitを持つ (個クラスはオーバーライドする)
	// virtual methods (should be overrided)
	void (*read_track)(ms_disk_drive_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*write_track)(ms_disk_drive_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*flush_track)(ms_disk_drive_t* d);
	uint8_t (*is_disk_changed)(ms_disk_drive_t* d);
} ms_disk_drive_t;

ms_disk_drive_t* ms_disk_drive_alloc();
void ms_disk_drive_init(ms_disk_drive_t* instance);
void ms_disk_drive_deinit(ms_disk_drive_t* instance);

#endif