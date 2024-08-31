#ifndef MS_DISK_DRIVE_FLOPPY_H
#define MS_DISK_DRIVE_FLOPPY_H

#include <stdint.h>
#include "ms_disk.h"
#include "ms_disk_drive.h"
#include "ms_disk_container.h"

typedef struct ms_disk_drive_floppy ms_disk_drive_floppy_t;

typedef struct ms_disk_drive_floppy {
	ms_disk_drive_t base;

	// properties
	ms_disk_container_t* container;
} ms_disk_drive_floppy_t;

ms_disk_drive_floppy_t* ms_disk_drive_floppy_alloc();
void ms_disk_drive_floppy_init(ms_disk_drive_floppy_t* instance, ms_disk_container_t* container);
void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance);

void ms_disk_drive_floppy_read_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_drive_floppy_write_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_drive_floppy_flush_track(ms_disk_drive_floppy_t* d);
uint8_t ms_disk_drive_floppy_is_disk_changed(ms_disk_drive_floppy_t* d);

#endif