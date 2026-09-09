#include "fat32.h"

#include "../../../../core/log.h"

static uint16_t read_u16_le(const uint8_t* buffer) {
    return (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
}

static uint32_t read_u32_le(const uint8_t* buffer) {
    return (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) |  ((uint32_t)buffer[3] << 24);
}

int fat32_mount(BlockDevice* device, Filesystem* filesystem) {
    if (!device || !filesystem) {
        return 0;
    }

    uint8_t sector[512];

    if (!block_read(device, 0, 1, sector)) {
        kernel_log("fat32 failed to read boot sector.");
        return 0;
    }

    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        kernel_log("fat32 invalid boot sector signature.");
        return 0;
    }

    FAT32Filesystem* fat32 = (FAT32Filesystem*)filesystem->filesystem_data;

    if (!fat32) {
        return 0;
    }

    fat32->bytes_per_sector = read_u16_le(&sector[11]);
    fat32->sectors_per_cluster = sector[13];
    fat32->reserved_sector_count = read_u16_le(&sector[14]);
    fat32->fat_count = sector[16];
    fat32->sectors_per_fat = read_u32_le(&sector[36]);
    fat32->root_cluster = read_u32_le(&sector[44]);

    if (fat32->bytes_per_sector == 0 || fat32->sectors_per_cluster == 0 || fat32->sectors_per_fat == 0) {
        kernel_log("invalid fat32 filesystem parameters.");
        return 0;
    }

    fat32->fat_start_lba = fat32->reserved_sector_count;
    fat32->data_start_lba = fat32->reserved_sector_count + fat32->fat_count * fat32->sectors_per_fat;

    if (fat32->data_start_lba >= device->sector_count) {
        kernel_log("fat32 data region is outside partition.");
        return 0;
    }

    uint32_t data_sectors = (uint32_t)device->sector_count - fat32->data_start_lba;
    fat32->cluster_count = data_sectors / fat32->sectors_per_cluster;

    if (fat32->cluster_count < 65525) {
        kernel_log("not actually fat32.");
        return 0;
    }

    filesystem->device = device;
    filesystem->type = FILESYSTEM_FAT32;
    kernel_log("fat32 filesystem mounted.");
    kernel_log("bytes per sector %u.", fat32->bytes_per_sector);
    kernel_log("sector per cluster %u.", fat32->sectors_per_cluster);
    kernel_log("reversed sectors %u.", fat32->reserved_sector_count);
    kernel_log("fat count %u.", fat32->fat_count);
    kernel_log("sectors per fat %u.", fat32->sectors_per_fat);
    kernel_log("root cluster %u.", fat32->root_cluster);
    kernel_log("data start lba %u.", fat32->data_start_lba);
    return 1;
}