#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive_floppy.h"


// override methods
static void _floppy_read_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static void _floppy_write_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static void _floppy_flush_track(ms_disk_drive_floppy_t* d);
static uint8_t _floppy_is_disk_changed(ms_disk_drive_floppy_t* d);

// private methods
static void _set_motor(ms_disk_drive_floppy_t* d, uint8_t motoron);
static void _set_side(ms_disk_drive_floppy_t* d, uint8_t side);
static void _seek(ms_disk_drive_floppy_t* d, uint32_t track_no);
static uint8_t _is_disk_inserted(ms_disk_drive_floppy_t* d);
static uint8_t _get_next_sector(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer);



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
	instance->base.read_track = (ms_disk_drive_read_track_t)_floppy_read_track;
	instance->base.write_track = (ms_disk_drive_write_track_t)_floppy_write_track;
	instance->base.flush_track = (ms_disk_drive_flush_track_t)_floppy_flush_track;
	instance->base.is_disk_changed = (ms_disk_drive_is_disk_changed_t)_floppy_is_disk_changed;
	//
	instance->set_motor = _set_motor;
	instance->set_side = _set_side;
	instance->seek = _seek;
	instance->is_disk_inserted = _is_disk_inserted;
	instance->get_next_sector = _get_next_sector;

	//
	instance->container = container;
	instance->is_double_sided = 1;
	instance->is_write_protected = 1;
	instance->is_track00 = 1;

	//
	instance->_present_cylinder_number = 0;

	return;
}

void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance) {
	ms_disk_drive_deinit(&instance->base);
}


static void _floppy_read_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->read_track(d->container, track_no, side, raw_track);
}

static void _floppy_write_track(ms_disk_drive_floppy_t* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->write_track(d->container, track_no, side, raw_track);
}

static void _floppy_flush_track(ms_disk_drive_floppy_t* d) {
	d->container->flush_track(d->container);
}

static uint8_t _floppy_is_disk_changed(ms_disk_drive_floppy_t* d) {
	return d->container->is_disk_changed(d->container);
}


static void _set_motor(ms_disk_drive_floppy_t* d, uint8_t motoron) {

}

static void _set_side(ms_disk_drive_floppy_t* d, uint8_t side) {
	d->_present_side_number = side;
	d->_track_buffer_ready = 0;
}

static void _seek(ms_disk_drive_floppy_t* d, uint32_t track_no) {
	d->_present_cylinder_number = track_no;	
	d->_track_buffer_ready = 0;
}

static uint8_t _is_disk_inserted(ms_disk_drive_floppy_t* d) {
	return d->container->current_disk != NULL;
}

/**
 * @brief Get the next sector object
 * 
 * @param d 
 * @param sector_buffer 
 * @return uint8_t 成功時は1, 失敗時は0
 */
static uint8_t _get_next_sector(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer) {
	if (d->_track_buffer_ready == 0) {
		ms_disk_raw_track_t* raw_track = &d->_track_buffer;
		if (!d->container->read_track(d->container, d->_present_cylinder_number, d->_present_side_number, raw_track)) {
			return 0; // 失敗
		}
		d->_track_buffer_ready = 1;
		d->_present_sector_number = 0;
	}

	// TODO 本当はここでトラックの生データを解析してセクタを取得するが、ここでは決めうちで読み出す
	sector_buffer->track = d->_present_cylinder_number;
	sector_buffer->head = d->_present_side_number;
	sector_buffer->sector = d->_present_sector_number;
	int offset = 80+12+3+1+50; // track header size
	offset += 658 * sector_buffer->sector; // sector position
	offset += 12; // sector header size
	int i;
	for(i=0; i<512; i++) {
		sector_buffer->data[i] = d->_track_buffer.data[offset++];
	}
	sector_buffer->crc = 0; // TODO CRC
	sector_buffer->crc_expected = 0; // TODO CRC

	// 次のセクタへ
	d->_present_sector_number++;
	if(d->_present_sector_number >= 9) {
		d->_present_sector_number = 0;
	}
}
