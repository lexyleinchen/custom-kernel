#include "filesystem.h"
#include "fat32/fat32.h"

#define MAX_MOUNTED_FILESYSTEMS 16

int filesystem_mount(BlockDevice* device, Filesystem* filesystem) {
    if (!device || !filesystem) {
        return 0;
    }

    filesystem->device = 0;
    filesystem->type = FILESYSTEM_UNKNOWN;
    filesystem->filesystem_data;
    FAT32Filesystem* fat32 = (FAT32Filesystem*)0;
    (void)fat32;
    return 0;
}