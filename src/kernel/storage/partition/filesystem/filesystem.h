#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

#include "../../block.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FILESYSTEM_UNKNOWN,
    FILESYSTEM_FAT32
} FilesystemType;

typedef struct {
    char name[256];
    uint8_t is_directory;
    uint64_t size;
    uint32_t cluster;
} FilesystemEntry;

typedef struct {
    BlockDevice* device;
    FilesystemType type;
    void* filesystem_data;
} Filesystem;

int filesystem_mount(BlockDevice* device, Filesystem* filesystem);

int filesystem_read_directory(Filesystem* filesystem, uint32_t cluster, FilesystemEntry* entries, uint32_t max_entries, uint32_t* entry_count);

uint32_t filesystem_get_count(void);

Filesystem* filesystem_get(uint32_t index);

int filesystem_create_directory(Filesystem* filesystem, uint32_t parent_cluster, const char* name);

int filesystem_create_file(Filesystem* filesystem, uint32_t parent_cluster, const char* name);

#ifdef __cplusplus
}
#endif

#endif // FILESYSTEM_H