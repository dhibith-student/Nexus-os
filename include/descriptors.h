#ifndef NEXUS_DESCRIPTORS_H
#define NEXUS_DESCRIPTORS_H

#include <stdint.h>

struct nexus_tss64 {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

_Static_assert(sizeof(struct nexus_tss64) == 104, "unexpected x86-64 TSS size");

uint64_t nexus_make_segment_descriptor(uint32_t base, uint32_t limit,
                                       uint8_t access, uint8_t flags);
void nexus_tss_init(struct nexus_tss64 *tss);

#endif
