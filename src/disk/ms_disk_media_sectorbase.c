#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_media_sectorbase.h"


/*
	確保ルーチン
 */
ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc() {
	return (ms_disk_media_sectorbase_t*)new_malloc(sizeof(ms_disk_media_sectorbase_t));
}

/*
	初期化ルーチン
 */
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance, char* name) {
	if (instance == NULL) {
		return;
	}
	// baseクラスの初期化
	ms_disk_media_init(&instance->base, name);
	// メソッドの登録
	instance->base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_sectorbase_deinit; //override
	instance->base.read_track = ms_disk_media_sectorbase_read_track; //override
	instance->base.write_track = ms_disk_media_sectorbase_write_track; //override
	return;
}

void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance) {
	ms_disk_media_deinit(&instance->base);
}


void _write_data_to_track(int count, uint8_t data, uint8_t* track, int* offset) {
	int i;
	for(i=0; i<count; i++) {
		track[*offset++] = data;
	}
}

/**
 * @brief CRC16を計算します。
 * 
 * TODO 実装があっているかちゃんと確認する
 * 
 * @param CRC 
 * @param data 
 * @return uint16_t 
 */
uint16_t ms_disk_update_CRC16(uint16_t CRC, uint8_t data) {
	uint16_t poly = 0x1021;
	uint16_t mask = 0x8000;
	uint16_t bit;
	uint16_t i;

	for(i=0; i<8; i++) {
		bit = (CRC & mask) ? 1 : 0;
		CRC <<= 1;
		if (data & (1 << (7-i))) {
			bit ^= 1;
		}
		if (bit) {
			CRC ^= poly;
		}
	}
	return CRC;
}

uint16_t ms_disk_calc_CRC16(uint8_t* data, int length) {
	uint16_t CRC = 0;
	int i;
	for(i=0; i<length; i++) {
		CRC = ms_disk_update_CRC16(CRC, data[i]);
	}
	return CRC;
}

void ms_disk_media_sectorbase_read_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_sectorbase_t* instance = (ms_disk_media_sectorbase_t*)media;

	// openMSXの SectorBasedDisk.cc から引用
	// -- track --
	// gap4a         80 x 0x4e
	// sync          12 x 0x00
	// index mark     3 x 0xc2(*)
	//                1 x 0xfc
	// gap1          50 x 0x4e
	// 9 x [sector]   9 x [[658]]
	// gap4b        182 x 0x4e
	//
	// -- sector --
	// sync          12 x 0x00
	// ID addr mark   3 x 0xa1(*)
	//                1 x 0xfe
	// C H R N        4 x [..]
	// CRC            2 x [..]
	// gap2          22 x 0x4e
	// sync          12 x 0x00
	// data mark      3 x 0xa1(*)
	//                1 x 0xfb
	// data         512 x [..]    <-- actual sector data
	// CRC            2 x [..]
	// gap3          84 x 0x4e
	//
	// (*) Missing clock transitions in MFM encoding

	int offset = 0;
	uint8_t* track = raw_track->data;

	_write_data_to_track(80, 0x4e, track, &offset);	// gap4a
	_write_data_to_track(12, 0x00, track, &offset);	// sync
	_write_data_to_track( 3, 0xc2, track, &offset);	// index mark (1)
	_write_data_to_track( 1, 0xfc, track, &offset);	// index mark (2)
	_write_data_to_track(50, 0x4e, track, &offset);	// gap1

	// セクター番号は1から始まる
	uint32_t sector_id = (track_no * instance->heads + side) * instance->sectors_per_track + 1;
	ms_sector_t sector;
	uint16_t CRC;
	int i,j;
	for(i=0; i<instance->sectors_per_track; i++, sector_id++) {
		instance->read_sector(media, sector_id, &sector);

		_write_data_to_track(12, 0x00, track, &offset);	// sync
		_write_data_to_track( 3, 0xa1, track, &offset);	// ID addr mark (1)
		_write_data_to_track( 1, 0xfe, track, &offset);	// ID addr mark (2)
		_write_data_to_track( 1, track_no, track, &offset);// C
		_write_data_to_track( 1, side, track, &offset);	// H
		_write_data_to_track( 1, i+1, track, &offset);		// R
		_write_data_to_track( 1, 2, track, &offset);		// N (length of sector: 512 = 128 << 2)

		CRC = ms_disk_calc_CRC16(track + offset - 8, 8);
		_write_data_to_track( 1, CRC >> 8,   track, &offset);	// CRC(H)
		_write_data_to_track( 1, CRC & 0xff, track, &offset);	// CRC(L)

		_write_data_to_track(22, 0x4e, track, &offset);	// gap2
		_write_data_to_track(12, 0x00, track, &offset);	// sync

		_write_data_to_track( 3, 0xa1, track, &offset);	// data mark (1)
		_write_data_to_track( 1, 0xfb, track, &offset);	// data mark (2)

		instance->read_sector(media, sector_id + i, &sector);
		memcpy(raw_track->data + offset, &sector, 512);
		for(j=0; j<512; j++) {
			_write_data_to_track(1, sector[j], track, &offset);
		}

		CRC = ms_disk_calc_CRC16(track + offset - (512 + 4), (512+4));
		_write_data_to_track( 1, CRC >> 8,   track, &offset);	// CRC(H)
		_write_data_to_track( 1, CRC & 0xff, track, &offset);	// CRC(L)

		_write_data_to_track(84, 0x4e, track, &offset);	// gap3
	}

	_write_data_to_track(182, 0x4e, track, &offset);	// gap4b

	if (1) {
		if (offset != 6250) {
			printf("track byte count %d is not match 6250\n", offset);
			ms_exit();
		}
	}
}

void ms_disk_media_sectorbase_write_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track) {
	ms_disk_media_sectorbase_t* instance = (ms_disk_media_sectorbase_t*)media;

	int offset = 0;
	uint8_t* track = raw_track->data;

	// 本当は、raw_trackのバイト列をデコードして、ID addr mark を見つける必要があるが、
	// ひとまず固定アドレスで抽出する
	uint16_t CRC;
	int i;
	for(i=0; i< 9; i++) {
		int track_header_size = 80 + 12 + 3 + 1 + 50;
		int sector_header_size = 12;
		offset = track_header_size + i * 658 + sector_header_size;
		uint32_t id_addr_mark;
		id_addr_mark = track[offset++];
		id_addr_mark <<= 8;
		id_addr_mark |= track[offset++];
		id_addr_mark <<= 8;
		id_addr_mark |= track[offset++];
		id_addr_mark <<= 8;
		id_addr_mark |= track[offset++];
		if (id_addr_mark != 0xa1a1a1fe) {
			printf("ID addr mark is not found\n");
			ms_exit();
		}
		uint8_t C = track[offset++];
		uint8_t H = track[offset++];
		uint8_t R = track[offset++];
		uint8_t N = track[offset++];
		if( C != track_no || H != side || N != 2) {
			printf("C=%d, H=%d, R=%d, N=%d\n", C, H, R, N);
			printf("track_no=%d, side=%d\n", track_no, side);
			printf("C, H, R, N is not match\n");
			ms_exit();
		}
		CRC = ms_disk_calc_CRC16(track + offset - 8, 8);
		uint16_t CRC_in_data = track[offset] << 8 | track[offset+1];
		offset += 2;
		if (CRC != CRC_in_data) {
			printf("CRC error\n");
			ms_exit();
		}
		offset += 22;	// gap2
		uint32_t data_mark;
		data_mark = track[offset++];
		data_mark <<= 8;
		data_mark |= track[offset++];
		data_mark <<= 8;
		data_mark |= track[offset++];
		data_mark <<= 8;
		data_mark |= track[offset++];
		if (data_mark != 0xa1a1a1fb) {
			printf("data mark is not found\n");
			ms_exit();
		}
		// セクター番号は1から始まるが、Rが1から始まるので、そのまま使えばOK
		uint32_t sector_id = (track_no * instance->heads + side) * instance->sectors_per_track + H + R;
		instance->write_sector(media, sector_id, (ms_sector_t*)(track+offset));
	}

}
