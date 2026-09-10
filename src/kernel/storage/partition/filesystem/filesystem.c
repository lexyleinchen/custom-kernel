#include "filesystem.h"
#include "fat32/fat32.h"

#define MAX_MOUNTED_FILESYSTEMS 16

static FAT32Filesystem fat32_instances[MAX_MOUNTED_FILESYSTEMS];
static Filesystem* mounted_filesystems[MAX_MOUNTED_FILESYSTEMS];
static uint32_t mounted_filesystem_count = 0;

int filesystem_mount(BlockDevice* device, Filesystem* filesystem) {
    if (!device || !filesystem) {
        return 0;
    }

    if (mounted_filesystem_count >= MAX_MOUNTED_FILESYSTEMS) {
        return 0;
    }

    FAT32Filesystem* fat32 = &fat32_instances[mounted_filesystem_count];
    filesystem->device = device;
    filesystem->type = FILESYSTEM_UNKNOWN;
    filesystem->filesystem_data = fat32;

    if (fat32_mount(device, filesystem)) {
        mounted_filesystems[mounted_filesystem_count] = filesystem;
        mounted_filesystem_count++;
        return 1;
    }

    filesystem->device = 0;
    filesystem->type = FILESYSTEM_UNKNOWN;
    filesystem->filesystem_data = 0;
    return 0;
}

uint32_t filesystem_get_count(void) {
    return mounted_filesystem_count;
}

Filesystem* filesystem_get(uint32_t index) {
    if (index >= mounted_filesystem_count) {
        return 0;
    }

    return mounted_filesystems[index];
}

int filesystem_read_directory(Filesystem* filesystem, uint32_t cluster, FilesystemEntry* entries, uint32_t max_entries, uint32_t* entry_count) {
    if (!filesystem || !entries || !entry_count) {
        return 0;
    }

    *entry_count = 0;

    if (filesystem->type == FILESYSTEM_FAT32) {
        return fat32_read_directory(filesystem, cluster, entries, max_entries, entry_count);
    }

    return 0;
}

int filesystem_create_directory(Filesystem* filesystem, uint32_t parent_cluster, const char* name) {
    if (!filesystem || !name) {
        return 0;
    }

    if (filesystem->type == FILESYSTEM_FAT32) {
        return fat32_create_directory(filesystem, parent_cluster, name);
    }

    return 0;
}

int filesystem_create_file(Filesystem* filesystem, uint32_t parent_cluster, const char* name) {
    if (!filesystem || !name) {
        return 0;
    }

    if (filesystem->type == FILESYSTEM_FAT32) {
        return fat32_create_file(filesystem, parent_cluster, name);
    }

    return 0;
}