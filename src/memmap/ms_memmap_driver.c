#include "ms_memmap_driver.h"

#include <stddef.h>
#include <stdint.h>

/*
        確保ルーチン(abstract class)なので使わない
 */
// ms_memmap_driver_t* ms_disk_media_init(char* file_path) {
// 	return (ms_memmap_driver_t*)new_malloc(sizeof(ms_memmap_driver_t));
// }

/*
        初期化ルーチン
 */
void ms_memmap_driver_init(ms_memmap_driver_t* instance, ms_memmap_t* memmap, uint8_t* buffer) {
    if (instance == NULL) {
        return;
    }
    // メソッドの登録
    instance->deinit = ms_memmap_driver_deinit;
    instance->memmap = memmap;
    instance->buffer = buffer;
    // new_malloc はゼロクリアを保証しないため、必ず安全側（ドライバ経由）で初期化する
    instance->direct_read_mask = 0;
    instance->direct_write_mask = 0;
    return;
}

void ms_memmap_driver_deinit(ms_memmap_driver_t* instance) {
    new_free(instance->buffer);
}