#ifndef MS_DISK_DRIVE_FLOPPY_H
#define MS_DISK_DRIVE_FLOPPY_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_drive_floppy ms_disk_drive_floppy_t;

typedef struct ms_disk_drive_floppy {
	ms_disk_drive_t base;
	// 

} ms_disk_drive_floppy_t;

ms_disk_drive_floppy_t* ms_disk_drive_floppy_alloc();
void ms_disk_drive_floppy_init(ms_disk_drive_floppy_t* instance);
void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance);

#endif