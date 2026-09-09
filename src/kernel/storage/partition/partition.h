#ifndef PARTITION_H
#define PARTITION_H

#include <stdint.h>

#include "../block.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t bootable;
    uint8_t start_chs[3];
    uint8_t type;
    uint8_t end_chs[3];
    uint32_t start_lba;
    uint32_t sector_count;
} MBRPartition;

typedef struct {
    BlockDevice* parent;
    uint64_t start_lba;
    uint64_t sector_count;
} PartitionDevice;

typedef struct {
    uint8_t bootable;
    uint8_t type;
    uint32_t start_lba;
    uint32_t sector_count;
} MBRPartitionInfo;

void partition_scan(BlockDevice* device);

int partition_create_mbr(BlockDevice* device);

int partition_create_mbr_partition(BlockDevice* device, uint32_t partition_number, uint32_t start_lba, uint32_t sector_count, uint8_t type);

int partition_create_mbr_partition_size(BlockDevice* device, uint32_t size_mib, uint8_t type);

int partition_delete(BlockDevice* device, uint32_t partition_number);

int partition_get_mbr_partition(BlockDevice* device, uint32_t partition_number, MBRPartitionInfo* partition);

int partition_find_free_space(BlockDevice* device, uint32_t requested_sectors, uint32_t* start_lba);

int partition_has_mbr(BlockDevice* device);

#ifdef __cplusplus
}
#endif

#endif // PARTITION_H