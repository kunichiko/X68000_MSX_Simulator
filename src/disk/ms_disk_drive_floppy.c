#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive_floppy.h"


static void set_motor(ms_disk_drive_floppy_t* d, uint8_t motoron);
static void set_side(ms_disk_drive_floppy_t* d, uint8_t side);
static void seek(ms_disk_drive_floppy_t* d, uint32_t track_no);
static void get_next_sector(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer);


/*
	確保ルーチン
 */
ms_disk_drive_floppy_t* ms_disk_drive_floppy_alloc() {
	return (ms_disk_drive_floppy_t*)new_malloc(sizeof(ms_disk_drive_floppy_t));
}

/*
	初期化ルーチン
 */
void ms_disk_drive_floppy_init(ms_disk_drive_floppy_t* instance, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	ms_disk_drive_init(&instance->base);
	// メソッドの登録 (override)
	instance->base.deinit = (ms_disk_drive_deinit_t)ms_disk_drive_floppy_deinit;
	instance->base.read_track = (ms_disk_drive_read_track_t)ms_disk_drive_floppy_read_track;
	instance->base.write_track = (ms_disk_drive_write_track_t)ms_disk_drive_floppy_write_track;
	instance->base.flush_track = (ms_disk_drive_flush_track_t)ms_disk_drive_floppy_flush_track;
	instance->base.is_disk_changed = (ms_disk_drive_is_disk_changed_t)ms_disk_drive_floppy_is_disk_changed;
	//
	instance->set_motor = set_motor;
	instance->set_side = set_side;
	instance->seek = seek;
	instance->get_next_sector = get_next_sector;

	//
	instance->container = container;
	instance->is_double_sided = 1;
	instance->is_write_protected = 1;
	instance->is_disk_inserted = 1;
	instance->is_track00 = 1;

	//
	instance->_present_cylinder_number = 0;

	return;
}

void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance) {
	ms_disk_drive_deinit(&instance->base);
}


void ms_disk_drive_floppy_read_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->read_track(d->container, track_no, side, raw_track);
}

void ms_disk_drive_floppy_write_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->write_track(d->container, track_no, side, raw_track);
}

void ms_disk_drive_floppy_flush_track(ms_disk_drive_floppy_t* d) {
	d->container->flush_track(d->container);
}

uint8_t ms_disk_drive_floppy_is_disk_changed(ms_disk_drive_floppy_t* d) {
	return d->container->is_disk_changed(d->container);
}


static void set_motor(ms_disk_drive_floppy_t* d, uint8_t motoron) {

}

static void set_side(ms_disk_drive_floppy_t* d, uint8_t side) {

}

static void seek(ms_disk_drive_floppy_t* d, uint32_t track_no) {

}

static void get_next_sector(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer) {

}
