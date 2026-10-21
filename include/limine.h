#ifndef NEXUS_LIMINE_H
#define NEXUS_LIMINE_H

#include <stdint.h>

#define LIMINE_COMMON_MAGIC 0xc7b1dd30e0a4e6b7ULL
#define LIMINE_MEMORY_MAP_REQUEST_ID 0x67cf3d9d378a806fULL, 0xe304acdfc50c3c62ULL

struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;
    struct limine_memmap_entry **entries;
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_memmap_response *response;
};

enum limine_memmap_type {
    LIMINE_MEMMAP_USABLE = 0,
    LIMINE_MEMMAP_RESERVED = 1,
    LIMINE_MEMMAP_ACPI_RECLAIMABLE = 2,
    LIMINE_MEMMAP_ACPI_NVS = 3,
    LIMINE_MEMMAP_BAD_MEMORY = 4,
    LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE = 5,
    LIMINE_MEMMAP_EXECUTABLE_AND_MODULES = 6,
    LIMINE_MEMMAP_FRAMEBUFFER = 7
};

#endif
