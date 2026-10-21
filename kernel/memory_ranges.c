#include <stdint.h>
#include "memory_ranges.h"
#include "physical_memory.h"

static uint64_t max_value(void) {
    return ~(uint64_t)0;
}

static int align_up(uint64_t value, uint64_t *aligned) {
    const uint64_t mask = NEXUS_PMM_FRAME_SIZE - 1;
    if (value > max_value() - mask) return 0;
    *aligned = (value + mask) & ~mask;
    return 1;
}

int nexus_range_from_length(uint64_t base, uint64_t length,
                            struct nexus_memory_range *range) {
    if (range == 0 || base > max_value() - length) return 0;
    range->start = base;
    range->end = base + length;
    return 1;
}

int nexus_range_from_bounds(uint64_t start, uint64_t end,
                            struct nexus_memory_range *range) {
    if (range == 0 || end < start) return 0;
    range->start = start;
    range->end = end;
    return 1;
}

int nexus_range_overlaps(struct nexus_memory_range first,
                         struct nexus_memory_range second) {
    return first.start < second.end && second.start < first.end;
}

int nexus_range_align_frames(struct nexus_memory_range input,
                             struct nexus_memory_range *aligned) {
    uint64_t start;
    if (aligned == 0 || input.end < input.start) return 0;
    if (input.start == input.end) {
        *aligned = input;
        return 1;
    }
    if (!align_up(input.start, &start)) return 0;
    aligned->start = start;
    aligned->end = input.end & ~(NEXUS_PMM_FRAME_SIZE - 1);
    if (aligned->start > aligned->end) {
        aligned->start = aligned->end;
    }
    return 1;
}

int nexus_reservation_add(struct nexus_reservation_set *set,
                          uint64_t base, uint64_t length) {
    struct nexus_memory_range candidate;
    if (set == 0 || !nexus_range_from_length(base, length, &candidate)) return 0;
    if (candidate.start == candidate.end) return 1;
    if (set->count >= NEXUS_MAX_RESERVATIONS) return 0;
    for (uint64_t index = 0; index < set->count; ++index) {
        if (nexus_range_overlaps(candidate, set->ranges[index])) return 0;
    }
    set->ranges[set->count++] = candidate;
    return 1;
}
