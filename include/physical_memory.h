#ifndef NEXUS_PHYSICAL_MEMORY_H
#define NEXUS_PHYSICAL_MEMORY_H

#include <stdint.h>
#include "limine.h"

#define NEXUS_PMM_FRAME_SIZE 4096ULL
#define NEXUS_PMM_MAX_REGIONS 64U
#define NEXUS_PMM_MAX_RESERVED_RANGES 64U

struct nexus_pmm_reserved_range {
    uint64_t base;
    uint64_t length;
};

struct nexus_pmm_region {
    uint64_t base;
    uint64_t frame_count;
    uint64_t bitmap_bit_offset;
};

struct nexus_pmm_allocator {
    struct nexus_pmm_region regions[NEXUS_PMM_MAX_REGIONS];
    uint64_t region_count;
    uint64_t total_frames;
    uint8_t *allocation_bitmap;
    uint8_t *reserved_bitmap;
    uint64_t bitmap_bytes;
};

int nexus_pmm_init(struct nexus_pmm_allocator *allocator,
                   const struct limine_memmap_response *response,
                   const struct nexus_pmm_reserved_range *reserved_ranges,
                   uint64_t reserved_count,
                   uint8_t *allocation_bitmap,
                   uint8_t *reserved_bitmap,
                   uint64_t bitmap_bytes);

int nexus_pmm_alloc(struct nexus_pmm_allocator *allocator,
                    uint64_t *physical_address);

int nexus_pmm_free(struct nexus_pmm_allocator *allocator,
                   uint64_t physical_address);

#endif
