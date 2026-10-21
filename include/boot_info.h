#ifndef NEXUS_BOOT_INFO_H
#define NEXUS_BOOT_INFO_H
#include <stdint.h>
#include "memory_ranges.h"
#define NEXUS_MAX_BOOT_RESPONSE_ENTRIES 256U
int nexus_boot_response_array_valid(uint64_t count, const void *array, uint64_t maximum);
int nexus_boot_hhdm_range_to_physical(uint64_t hhdm_offset, uint64_t virtual_address, uint64_t length, struct nexus_memory_range *range);
#endif
