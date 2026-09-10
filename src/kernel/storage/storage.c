#include "storage.h"
#include "block.h"
#include "partition/partition.h"
#include "../core/log.h"

#define MAX_STORAGE_FILESYSTEMS 16

static Filesystem storage_filesystems[MAX_STORAGE_FILESYSTEMS];
static uint32_t storage_filesystem_count = 0;

void storage_init(void) {
    kernel_log("initializing storage...");
    uint32_t device_count = block_get_device_count();
    kernel_log("block devices %u", device_count);

    for (uint32_t i = 0; i < device_count; i++) {
        BlockDevice* device = block_get_device(i);

        if (!device) {
            continue;
        }

        if (device->type != BLOCK_DEVICE_PHYSICAL) { 
            continue;
        }

        partition_scan(device);
    }

    for (uint32_t i = 0; i < device_count; i++) {
        BlockDevice* disk = block_get_device(i);

        if (!disk) {
            continue;
        }

        if (disk->type != BLOCK_DEVICE_PHYSICAL) { 
            continue;
        }

        for (uint32_t partition_number = 0; partition_number < 4; partition_number++) {
            if (storage_filesystem_count >= MAX_STORAGE_FILESYSTEMS) {
                break;
            }

            BlockDevice* partition = partition_get_device(disk, partition_number);

            if (!partition) {
                continue;
            }

            Filesystem* filesystem = &storage_filesystems[storage_filesystem_count];
            kernel_log("trying filesystem on disk %u partition %u", i, partition_number);

            if (filesystem_mount(partition, filesystem))  {
                kernel_log("filesystem mounted on disk %u partition %u", i, partition_number);
                storage_filesystem_count++;
            }
            else {
                kernel_log("no supported filesystem on disk %u partition %u", i, partition_number);
            }
        }
    }

    kernel_log("mounted filesystems %u", storage_filesystem_count);
    kernel_log("block devices after partition scan %u", block_get_device_count());
    kernel_log("storage initialized.");
}

uint32_t storage_get_filesystem_count(void) {
    return storage_filesystem_count;
}

Filesystem* storage_get_filesystem(uint32_t index) {
    if (index >= storage_filesystem_count) {
        return 0;
    }

    return &storage_filesystems[index];
}

int storage_mount_partition(BlockDevice* partition) {
    if (!partition) {
        return 0;
    }

    if (partition->type != BLOCK_DEVICE_PARTITION) {
        return 0;
    }

    for (uint32_t i = 0; i < storage_filesystem_count; i++) {
        if (storage_filesystems[i].device == partition) {
            return 1;
        }
    }

    if (storage_filesystem_count >= MAX_STORAGE_FILESYSTEMS) {
        kernel_log("storage filesystem list is full.");
        return 0;
    }

    Filesystem* filesystem = &storage_filesystems[storage_filesystem_count];
    kernel_log("trying to mount runtime partition as filesystem.");

    if (!filesystem_mount(partition, filesystem)) {
        kernel_log("runtime filesystem mount failed.");
        return 0;
    }

    storage_filesystem_count++;
    kernel_log("runtime filesystem mounted. total %u", storage_filesystem_count);
    return 1;
}

int storage_unmount_partition(BlockDevice* partition) {
    if (!partition) {
        return 0;
    }

    for (uint32_t i = 0; i < storage_filesystem_count; i++) {
        if (storage_filesystems[i].device != partition) {
            continue;
        }

        for (uint32_t j = i; j + 1 < storage_filesystem_count; j++) {
            storage_filesystems[j] = storage_filesystems[j + 1];
        }

        storage_filesystem_count--;
        storage_filesystems[storage_filesystem_count].device = 0;
        storage_filesystems[storage_filesystem_count].type = FILESYSTEM_UNKNOWN;
        storage_filesystems[storage_filesystem_count].filesystem_data = 0;
        kernel_log("filesystem unmounted. total %u", storage_filesystem_count);
        return 1;
    }

    return 0;
}