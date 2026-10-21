#include <stdint.h>
#include "memory_ranges.h"
#include "physical_memory.h"

int main(void) {
    struct nexus_memory_range range;
    struct nexus_memory_range aligned;

    if (!nexus_range_from_length(0x1000, 0x2000, &range) ||
        range.start != 0x1000 || range.end != 0x3000) return 1;
    if (nexus_range_from_length(~(uint64_t)0 - 1, 4, &range)) return 2;
    if (nexus_range_from_bounds(0x3000, 0x2000, &range)) return 3;
    if (!nexus_range_from_bounds(0x3000, 0x3000, &range) ||
        range.start != range.end) return 4;

    struct nexus_memory_range first = { .start = 0x1000, .end = 0x3000 };
    struct nexus_memory_range adjacent = { .start = 0x3000, .end = 0x5000 };
    struct nexus_memory_range overlap = { .start = 0x2fff, .end = 0x4000 };
    if (nexus_range_overlaps(first, adjacent)) return 5;
    if (!nexus_range_overlaps(first, overlap)) return 6;

    struct nexus_memory_range unaligned = { .start = 0x1234, .end = 0x5678 };
    if (!nexus_range_align_frames(unaligned, &aligned) ||
        aligned.start != 0x2000 || aligned.end != 0x5000) return 7;
    if (!nexus_range_align_reservation(unaligned, &aligned) ||
        aligned.start != 0x1000 || aligned.end != 0x6000) return 8;
    struct nexus_memory_range subframe = { .start = 0x1234, .end = 0x1fff };
    if (!nexus_range_align_reservation(subframe, &aligned) ||
        aligned.start != 0x1000 || aligned.end != 0x2000) return 9;
    if (!nexus_range_align_frames(subframe, &aligned) ||
        aligned.start != aligned.end) return 10;
    struct nexus_memory_range empty = { .start = 0x4000, .end = 0x4000 };
    if (!nexus_range_align_frames(empty, &aligned) ||
        aligned.start != aligned.end) return 11;

    struct nexus_reservation_set reservations = {0};
    if (!nexus_reservation_add(&reservations, 0x100000, 0x5000)) return 11;
    if (!nexus_reservation_add(&reservations, 0x105000, 0x1000)) return 12;
    if (nexus_reservation_add(&reservations, 0x102000, 0x1000)) return 13;
    if (nexus_reservation_add(&reservations, 0x100000, 0x5000)) return 14;
    if (!nexus_reservation_add(&reservations, 0xdead000, 0)) return 15;
    if (nexus_reservation_add(&reservations, ~(uint64_t)0, 2)) return 16;

    struct nexus_memory_range kernel_image = { .start = 0x100000, .end = 0x105000 };
    struct nexus_memory_range usable = { .start = 0x104000, .end = 0x110000 };
    if (!nexus_range_overlaps(kernel_image, usable)) return 17;

    struct limine_memmap_entry unsupported = {
        .base = 0x100000, .length = 0x1000,
        .type = LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE
    };
    if (unsupported.type == LIMINE_MEMMAP_USABLE) return 18;
    return 0;
}
