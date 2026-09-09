#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

#include "../filesystem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t fat_start_lba;
    uint32_t data_start_lba;
    uint32_t cluster_count;
} FAT32Filesystem;

int fat32_mount(BlockDevice* device, Filesystem* filesystem);

int fat32_read_directory(Filesystem* filesystem, uint32_t cluster, FilesystemEntry* entries, uint32_t max_entries, uint32_t* entry_count);

#ifdef __cplusplus
}
#endif

#endif // FAT32_H