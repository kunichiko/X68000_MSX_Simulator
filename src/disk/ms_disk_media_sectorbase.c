#include "ms_disk_media_sectorbase.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ms.h"
#include "ms_disk.h"

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
    instance->base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_sectorbase_deinit;  // override
    instance->base.read_track = ms_disk_media_sectorbase_read_track;                      // override
    instance->base.write_track = ms_disk_media_sectorbase_write_track;                    // override
    return;
}

void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance) {
    ms_disk_media_deinit(&instance->base);
}

void _write_data_to_track(int count, uint8_t data, uint8_t* track, int* offset) {
    int i;
    for (i = 0; i < count; i++) {
        track[*offset] = data;
        (*offset)++;
    }
}

/* CRC16-CCITT(多項式 0x1021、上位ビットから)。1 バイト分の 8 回転を先に畳んだ表 */
static const uint16_t _crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/**
 * @brief CRC16 を 1 バイト分進めます。
 *
 * @param CRC
 * @param data
 * @return uint16_t
 */
uint16_t ms_disk_update_CRC16(uint16_t CRC, uint8_t data) {
    return (uint16_t)((CRC << 8) ^ _crc16_table[(uint8_t)((CRC >> 8) ^ data)]);
}

uint16_t ms_disk_calc_CRC16(uint8_t* data, int length) {
    uint16_t CRC = 0;
    int i;
    for (i = 0; i < length; i++) {
        CRC = (uint16_t)((CRC << 8) ^ _crc16_table[(uint8_t)((CRC >> 8) ^ data[i])]);
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

    _write_data_to_track(80, 0x4e, track, &offset);  // gap4a
    _write_data_to_track(12, 0x00, track, &offset);  // sync
    _write_data_to_track(3, 0xc2, track, &offset);   // index mark (1)
    _write_data_to_track(1, 0xfc, track, &offset);   // index mark (2)
    _write_data_to_track(50, 0x4e, track, &offset);  // gap1

    // セクター番号は1から始まる
    uint32_t sector_id = (track_no * instance->heads + side) * instance->sectors_per_track + 1;
    ms_sector_t sector;
    uint16_t CRC;
    int i;
    // printf("Reading track %d side %d\n", track_no, side);
    for (i = 0; i < instance->sectors_per_track; i++, sector_id++) {
        _write_data_to_track(12, 0x00, track, &offset);     // sync
        _write_data_to_track(3, 0xa1, track, &offset);      // ID addr mark (1)
        _write_data_to_track(1, 0xfe, track, &offset);      // ID addr mark (2)
        _write_data_to_track(1, track_no, track, &offset);  // C
        _write_data_to_track(1, side, track, &offset);      // H
        _write_data_to_track(1, i + 1, track, &offset);     // R
        _write_data_to_track(1, 2, track, &offset);         // N (length of sector: 512 = 128 << 2)

        CRC = ms_disk_calc_CRC16(track + offset - 8, 8);
        _write_data_to_track(1, CRC >> 8, track, &offset);    // CRC(H)
        _write_data_to_track(1, CRC & 0xff, track, &offset);  // CRC(L)

        _write_data_to_track(22, 0x4e, track, &offset);  // gap2
        _write_data_to_track(12, 0x00, track, &offset);  // sync

        _write_data_to_track(3, 0xa1, track, &offset);  // data mark (1)
        _write_data_to_track(1, 0xfb, track, &offset);  // data mark (2)

        // printf(" Reading sector %d (sector_id=%d)\n", i + 1, sector_id);
        instance->read_sector(media, sector_id, &sector);
        memcpy(track + offset, &sector, 512);
        offset += 512;

        CRC = ms_disk_calc_CRC16(track + offset - (512 + 4), (512 + 4));
        _write_data_to_track(1, CRC >> 8, track, &offset);    // CRC(H)
        _write_data_to_track(1, CRC & 0xff, track, &offset);  // CRC(L)

        _write_data_to_track(84, 0x4e, track, &offset);  // gap3
    }

    _write_data_to_track(182, 0x4e, track, &offset);  // gap4b

    if (1) {
        if (offset != 6250) {
            MS_LOG(MS_LOG_ERROR, "track byte count %d is not match 6250\n", offset);
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
    for (i = 0; i < 9; i++) {
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
            ms_exit_failure();
        }
        uint8_t C = track[offset++];
        uint8_t H = track[offset++];
        uint8_t R = track[offset++];
        uint8_t N = track[offset++];
        if (C != track_no || H != side || N != 2) {
            printf("C=%d, H=%d, R=%d, N=%d\n", C, H, R, N);
            printf("track_no=%d, side=%d\n", track_no, side);
            printf("C, H, R, N is not match\n");
            ms_exit_failure();
        }
        CRC = ms_disk_calc_CRC16(track + offset - 8, 8);
        uint16_t CRC_in_data = track[offset] << 8 | track[offset + 1];
        offset += 2;
        if (CRC != CRC_in_data) {
            printf("CRC error\n");
            ms_exit_failure();
        }
        offset += 22;  // gap2
        offset += 12;  // sync
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
            ms_exit_failure();
        }
        // セクター番号は1から始まるが、Rが1から始まるので、そのまま使えばOK
        uint32_t sector_id = (track_no * instance->heads + side) * instance->sectors_per_track + R;
        instance->write_sector(media, sector_id, (ms_sector_t*)(track + offset));
    }
}
