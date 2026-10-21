#ifndef NEXUS_MEMORY_RANGES_H
#define NEXUS_MEMORY_RANGES_H

#include <stdint.h>

#define NEXUS_MAX_RESERVATIONS 128U

struct nexus_memory_range {
    uint64_t start;
    uint64_t end;
};

struct nexus_reservation_set {
    struct nexus_memory_range ranges[NEXUS_MAX_RESERVATIONS];
    uint64_t count;
};

int nexus_range_from_length(uint64_t base, uint64_t length,
                            struct nexus_memory_range *range);
int nexus_range_from_bounds(uint64_t start, uint64_t end,
                            struct nexus_memory_range *range);
int nexus_range_overlaps(struct nexus_memory_range first,
                         struct nexus_memory_range second);
int nexus_range_align_frames(struct nexus_memory_range input,
                             struct nexus_memory_range *aligned);
int nexus_range_align_reservation(struct nexus_memory_range input,
                                  struct nexus_memory_range *aligned);
int nexus_reservation_add(struct nexus_reservation_set *set,
                          uint64_t base, uint64_t length);

#endif
