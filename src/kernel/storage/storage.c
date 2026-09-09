#include "storage.h"
#include "block.h"
#include "partition/partition.h"

#include "../core/log.h"

void storage_init(void) {
    kernel_log("initializing storage...");
    uint32_t device_count = block_get_device_count();
    kernel_log("block devices %u", device_count);

    for (uint32_t i = 0; i < device_count; i++) {
        BlockDevice* device = block_get_device(i);

        if (!device) {
            continue;
        }

        partition_scan(device);
    }

    kernel_log("block devices after partition scan %u", block_get_device_count());
    kernel_log("storage initialized.");
}