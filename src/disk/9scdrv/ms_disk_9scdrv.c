#include "ms_disk_9scdrv.h"

inline uint32_t convert_to_position(ms_disk_9scdrv_sector_size_t sector_size_code, uint8_t track, uint8_t side, uint8_t sector) {
    return ((uint32_t)(sector_size_code & 0x03) << 24) | ((uint32_t)(track & 0xFF) << 16) | ((uint32_t)(side & 0xFF) << 8) |
           (sector & 0xFF);
}
