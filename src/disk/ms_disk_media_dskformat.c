#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <x68k/dos.h>
#include "ms_disk.h"
#include "ms_disk_media_dskformat.h"
#include "../ms.h"

#define THIS ms_disk_media_dskformat_t

static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);

/*
	確保ルーチン
 */
THIS* ms_disk_media_dskformat_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

int ms_disk_media_dskformat_init(THIS* instance, char* file_path) {
	if (instance == NULL) {
		return 0;
	}
	// baseクラスの初期化
	ms_disk_media_sectorbase_init(&instance->base, file_path);
	// メソッドの登録
	instance->base.base.deinit =  (void (*)(ms_disk_media_t*))ms_disk_media_dskformat_deinit; //override
	instance->base.read_sector = _read_sector; //override
	instance->base.write_sector = _write_sector; //override

	// プロパティの初期化
	instance->file_path = file_path;
	int attr = _dos_chmod(file_path, -1); // ファイル属性を取得
	if (attr == -1) {
		printf("ファイルが存在しません. %s\n", file_path);
		return 0;
	}
	if ( (attr & 0x20) == 0 ) {
		printf("ファイルが通常のファイルではありません. %s\n", file_path);
		return 0;
	}
	if ( (attr & 0x10) != 0 ) {
		printf("ディレクトリは指定できません. %s\n", file_path);
		return 0;
	}
	instance->base.base.is_write_protected = (attr & 0x1) == 1; // READ ONLY属性があればライトプロテクト
	int mode = instance->base.base.is_write_protected ? O_RDONLY : O_RDWR;
	instance->file_handle = open(file_path, mode | O_BINARY);
	if (instance->file_handle == -1) {
		printf("ファイルが開けません. %s\n", file_path);
		return 0;
	}
	uint32_t file_size = filelength(instance->file_handle);

	instance->file_size	= file_size;
	instance->base.sectors_per_track = 9;
	instance->base.heads = file_size <= 512*9*80 ? 1 : 2;
	instance->base.tracks = (file_size / instance->base.heads) / 9 / 512;
	return 1;
}

void ms_disk_media_dskformat_deinit(THIS* instance) {
	ms_disk_media_sectorbase_deinit(&instance->base);
	close(instance->file_handle);
}

/**
 * @brief セクターを読み込みます。
 * 
 * @param instance 
 * @param sector_id 1から始まることに注意
 * @param sector 
 */
static void _read_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	int index = sector_id - 1;
	THIS* dsk = (THIS*)instance;
	uint32_t offset = index * 512;
	lseek(dsk->file_handle, offset, SEEK_SET);
	read(dsk->file_handle, sector, 512);
}

static void _write_sector(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector) {
	if(instance->is_write_protected) {
		return;
	}
	int index = sector_id - 1;
	THIS* dsk = (THIS*)instance;
	uint32_t offset = index * 512;
	lseek(dsk->file_handle, offset, SEEK_SET);
	if( write(dsk->file_handle, sector, 512) != 512 ) {
		MS_LOG(MS_LOG_ERROR, "セクタの書き込みに失敗しました. %s\n", dsk->file_path);
		return;
	}
	MS_LOG(MS_LOG_FINE, "Wr logical sector: %d\n", sector_id);

	// ファイルのフラッシュ
	// HEiJ の HFS(Host File System)上で動かしていると、この _dos_flush() がないとファイルが更新されない
	// ことがわかる。
	// X68000の実機の場合、他のプロセスがこのファイルを覗き見ることが基本的にないので、実際にフラッシュされている
	// 可動かを調べるのが難しい（強制リセットかけてみるしかない）ので、実際に実機でも flush されていないのか、
	// それtも HFSの特性で、flush が呼ばれるまでHost側のファイルを更新しない仕様なのかはわからないが、
	// エミュレータ環境で使う人も多いので、とりあえず flush しておく。
	_dos_fflush();
}
