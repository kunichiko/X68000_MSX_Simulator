#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive_floppy.h"
#include "../ms.h"

#define THIS ms_disk_drive_floppy_t

// override methods
static void _floppy_read_track(THIS* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static void _floppy_write_track(THIS* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
static void _floppy_flush_track(THIS* d);
static uint8_t _floppy_is_disk_changed(THIS* d);

// private methods
static void _set_motor(THIS* d, uint8_t motoron);
static void _set_side(THIS* d, uint8_t side);
static void _seek(THIS* d, uint32_t track_no);
static uint8_t _is_disk_inserted(THIS* d);
static uint8_t _get_next_sector(THIS* d, ms_disk_sector_t* sector_buffer);
static uint8_t _write_sector(THIS* d, ms_disk_sector_t* sector_buffer);

/*
	確保ルーチン
 */
THIS* ms_disk_drive_floppy_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	初期化ルーチン
 */
void ms_disk_drive_floppy_init(THIS* instance, ms_disk_container_t* container) {
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
	// メソッドの登録
	instance->set_motor = _set_motor;
	instance->set_side = _set_side;
	instance->seek = _seek;
	instance->is_disk_inserted = _is_disk_inserted;
	instance->get_next_sector = _get_next_sector;
	instance->write_sector = _write_sector;

	//
	instance->container = container;
	instance->is_double_sided = 1;
	instance->is_write_protected = 1;
	instance->is_track00 = 1;

	//
	instance->_present_cylinder_number = 0;

	return;
}

void ms_disk_drive_floppy_deinit(THIS* instance) {
	ms_disk_drive_deinit(&instance->base);
}

static void _refresh_status(THIS* d) {
	if (d->container != NULL && d->container->current_disk != NULL) {
		d->is_write_protected = d->container->current_disk->is_write_protected;
	}
}

static void _floppy_read_track(THIS* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->read_track(d->container, track_no, side, raw_track);
}

static void _floppy_write_track(THIS* d, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	d->container->write_track(d->container, track_no, side, raw_track);
}

static void _floppy_flush_track(THIS* d) {
	d->container->flush_track(d->container);
}

static uint8_t _floppy_is_disk_changed(THIS* d) {
	_refresh_status(d);
	return d->container->is_disk_changed(d->container);
}


static void _set_motor(THIS* d, uint8_t motoron) {

}

static void _set_side(THIS* d, uint8_t side) {
	d->_present_side_number = side;
	d->_track_buffer_ready = 0;
}

static void _seek(THIS* d, uint32_t track_no) {
	d->_present_cylinder_number = track_no;	
	d->_track_buffer_ready = 0;
}

static uint8_t _is_disk_inserted(THIS* d) {
	_refresh_status(d);
	return d->container->current_disk != NULL;
}

/**
 * @brief Get the next sector object
 * 
 * @param d 
 * @param sector_buffer 
 * @return uint8_t 成功時は1, 失敗時は0
 */
static uint8_t _get_next_sector(THIS* d, ms_disk_sector_t* sector_buffer) {
	if (d->_track_buffer_ready == 0) {
		ms_disk_raw_track_t* raw_track = &d->_track_buffer;
		if (!d->container->read_track(d->container, d->_present_cylinder_number, d->_present_side_number, raw_track)) {
			return 0; // 失敗
		}
		d->_track_buffer_ready = 1;
		d->_present_sector_number = 1;
	}

	// TODO 本当はここでトラックの生データを解析してセクタを取得するが、ここでは決めうちで読み出す
	sector_buffer->track = d->_present_cylinder_number;
	sector_buffer->head = d->_present_side_number;
	sector_buffer->sector = d->_present_sector_number;
	int offset = 80+12+3+1+50; // track header size
	offset += 658 * (sector_buffer->sector-1); // sector position
	offset += 12; // sector header size
	offset += 4 + 4; // id addr mark & id data
	offset += 2; // CRC
	offset += 22 + 12; // gap2 + sync
	offset += 3 + 1; // data mark
	int i;
	for(i=0; i<512; i++) {
		sector_buffer->data[i] = d->_track_buffer.data[offset++];
	}
	sector_buffer->crc = 0; // TODO CRC
	sector_buffer->crc_expected = 0; // TODO CRC

	// 次のセクタへ
	d->_present_sector_number++;
	if(d->_present_sector_number > 9) {
		d->_present_sector_number = 0;
	}
	return 1; // 成功
}

/**
 * @brief Write sector data to the current track
 * 
 * @param d 
 * @param sector_buffer 
 * @return uint8_t 成功時は1, 失敗時は0
 */
static uint8_t _write_sector(THIS* d, ms_disk_sector_t* sector_buffer) {
	if (d->_track_buffer_ready == 0) {
		ms_disk_raw_track_t* raw_track = &d->_track_buffer;
		if (!d->container->read_track(d->container, d->_present_cylinder_number, d->_present_side_number, raw_track)) {
			MS_LOG(MS_LOG_ERROR, "Load track failed: C:%d H:%d\n", d->_present_cylinder_number, d->_present_side_number);
			return 0; // 失敗
		}
		d->_track_buffer_ready = 1;
	}

	if( (sector_buffer->track != d->_present_cylinder_number) || //
		(sector_buffer->head != d->_present_side_number) ) {
		MS_LOG(MS_LOG_ERROR, "Track number is invalid:\n C:%d H:%d (expect C:%d H:%d)\n", //
			sector_buffer->track, sector_buffer->head, //
			d->_present_cylinder_number, d->_present_side_number);
		return 0; // 失敗
	}

	if(sector_buffer->sector > 9) {
		MS_LOG(MS_LOG_ERROR, "Sector number is invalid: %d\n", sector_buffer->sector);
		return 0; // 失敗
	}

	// TODO 本当はここでトラックの生データを解析してセクタを取得するが、ここでは決めうちで書き込む
	int offset = 80+12+3+1+50; // track header size
	offset += 658 * (sector_buffer->sector-1); // sector position
	offset += 12; // sector header size
	offset += 4 + 4; // id addr mark & id data
	offset += 2; // CRC
	offset += 22 + 12; // gap2 + sync
	offset += 3 + 1; // data mark
	int i;
	for(i=0; i<512; i++) {
		d->_track_buffer.data[offset++] = sector_buffer->data[i];
	}
	// TODO CRC

	// 書き込み
	d->container->write_track(d->container, d->_present_cylinder_number, d->_present_side_number, &d->_track_buffer);

	// 次のセクタへ (Writeの時はあまり関係ない?)
	d->_present_sector_number++;
	if(d->_present_sector_number > 9) {
		d->_present_sector_number = 0;
	}

	return 1; // 成功
}