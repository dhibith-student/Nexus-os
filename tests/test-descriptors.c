#include <stdint.h>
#include "descriptors.h"

int main(void) {
    uint64_t code = nexus_make_segment_descriptor(0, 0xfffffU, 0x9aU, 0x0aU);
    uint64_t data = nexus_make_segment_descriptor(0, 0xfffffU, 0x92U, 0x0cU);
    if (code != 0x00af9a000000ffffULL) return 1;
    if (data != 0x00cf92000000ffffULL) return 2;

    struct nexus_tss64 tss;
    nexus_tss_init(&tss);
    if (tss.rsp0 != 0 || tss.ist1 != 0 || tss.iomap_base != sizeof(tss)) return 3;
    return 0;
}
