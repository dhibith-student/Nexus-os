#include <stdint.h>
#include "descriptors.h"

uint64_t nexus_make_segment_descriptor(uint32_t base, uint32_t limit,
                                       uint8_t access, uint8_t flags) {
    uint64_t descriptor = 0;
    descriptor |= (uint64_t)(limit & 0xffffU);
    descriptor |= (uint64_t)(base & 0xffffU) << 16;
    descriptor |= (uint64_t)((base >> 16) & 0xffU) << 32;
    descriptor |= (uint64_t)access << 40;
    descriptor |= (uint64_t)((limit >> 16) & 0x0fU) << 48;
    descriptor |= (uint64_t)(flags & 0x0fU) << 52;
    descriptor |= (uint64_t)((base >> 24) & 0xffU) << 56;
    return descriptor;
}

void nexus_tss_init(struct nexus_tss64 *tss) {
    uint8_t *bytes = (uint8_t *)tss;
    for (uint64_t index = 0; index < sizeof(*tss); ++index) bytes[index] = 0;
    tss->iomap_base = (uint16_t)sizeof(*tss);
}
