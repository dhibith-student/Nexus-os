#include <stdint.h>
#include "boot_info.h"
int nexus_boot_response_array_valid(uint64_t count, const void *array, uint64_t maximum) { if (count > maximum) return 0; return count == 0 || array != 0; }
int nexus_boot_hhdm_range_to_physical(uint64_t hhdm_offset, uint64_t virtual_address, uint64_t length, struct nexus_memory_range *range) { if (range == 0 || virtual_address < hhdm_offset) return 0; return nexus_range_from_length(virtual_address - hhdm_offset, length, range); }
