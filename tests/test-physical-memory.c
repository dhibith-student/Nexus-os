#include <stdint.h>
#include "physical_memory.h"

static int contains(const uint64_t *values, uint64_t count, uint64_t value) {
    for (uint64_t index = 0; index < count; ++index)
        if (values[index] == value) return 1;
    return 0;
}

int main(void) {
    struct limine_memmap_entry entries_data[] = {
        { .base = 0, .length = 0x1000, .type = LIMINE_MEMMAP_USABLE },
        { .base = 0x1000, .length = 0x9000, .type = LIMINE_MEMMAP_USABLE },
        { .base = 0xa000, .length = 0x6000, .type = LIMINE_MEMMAP_RESERVED },
        { .base = 0x10000, .length = 0x5000, .type = LIMINE_MEMMAP_USABLE },
        { .base = 0x15345, .length = 0x2345, .type = LIMINE_MEMMAP_USABLE },
        { .base = 0x20000, .length = 0, .type = LIMINE_MEMMAP_USABLE }
    };
    struct limine_memmap_entry *entries[] = {
        &entries_data[0], &entries_data[1], &entries_data[2],
        &entries_data[3], &entries_data[4], &entries_data[5]
    };
    struct limine_memmap_response response = { .entry_count = 6, .entries = entries };
    struct nexus_pmm_reserved_range reserved[] = {
        { .base = 0x3000, .length = 0x1000 },
        { .base = 0x11000, .length = 0x1000 }
    };
    uint8_t allocation_bitmap[8] = {0};
    uint8_t reserved_bitmap[8] = {0};
    struct nexus_pmm_allocator allocator;
    if (!nexus_pmm_init(&allocator, &response, reserved, 2,
                        allocation_bitmap, reserved_bitmap, sizeof(allocation_bitmap))) return 1;

    uint64_t allocated[32];
    uint64_t count = 0;
    uint64_t address;
    while (nexus_pmm_alloc(&allocator, &address)) {
        if (address == 0 || address % NEXUS_PMM_FRAME_SIZE != 0 ||
            address == 0x3000 || address == 0x11000 ||
            contains(allocated, count, address)) return 2;
        allocated[count++] = address;
    }
    if (count != 13) return 3;
    if (nexus_pmm_alloc(&allocator, &address)) return 4;
    if (nexus_pmm_free(&allocator, 0x3001)) return 5;
    if (nexus_pmm_free(&allocator, 0xdead000)) return 6;
    if (nexus_pmm_free(&allocator, 0x3000)) return 7;
    if (!nexus_pmm_free(&allocator, allocated[2])) return 8;
    if (nexus_pmm_free(&allocator, allocated[2])) return 9;
    if (!nexus_pmm_alloc(&allocator, &address) || address != allocated[2]) return 10;

    struct limine_memmap_entry overflow_entry = {
        .base = ~(uint64_t)0 - 1, .length = 4, .type = LIMINE_MEMMAP_USABLE
    };
    struct limine_memmap_entry *overflow_entries[] = { &overflow_entry };
    struct limine_memmap_response overflow_response = {
        .entry_count = 1, .entries = overflow_entries
    };
    if (nexus_pmm_init(&allocator, &overflow_response, 0, 0,
                       allocation_bitmap, reserved_bitmap, sizeof(allocation_bitmap))) return 11;

    struct nexus_pmm_reserved_range overflow_reserved = {
        .base = ~(uint64_t)0, .length = 1
    };
    if (nexus_pmm_init(&allocator, &response, &overflow_reserved, 1,
                       allocation_bitmap, reserved_bitmap, sizeof(allocation_bitmap))) return 12;

    struct limine_memmap_response empty = { .entry_count = 0, .entries = entries };
    if (!nexus_pmm_init(&allocator, &empty, 0, 0,
                       allocation_bitmap, reserved_bitmap, sizeof(allocation_bitmap))) return 13;
    if (nexus_pmm_alloc(&allocator, &address)) return 14;

    struct limine_memmap_entry too_large = {
        .base = 0x1000, .length = 0x100000, .type = LIMINE_MEMMAP_USABLE
    };
    struct limine_memmap_entry *too_large_entries[] = { &too_large };
    struct limine_memmap_response too_large_response = {
        .entry_count = 1, .entries = too_large_entries
    };
    if (nexus_pmm_init(&allocator, &too_large_response, 0, 0,
                       allocation_bitmap, reserved_bitmap, 1)) return 15;
    return 0;
}
