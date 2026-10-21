#include <stdint.h>
#include "kernel_image.h"
#include "serial.h"

extern char __kernel_image_start[];
extern char __kernel_image_end[];

void nexus_kernel_image_print(void) {
    uint64_t start = (uint64_t)(uintptr_t)__kernel_image_start;
    uint64_t end = (uint64_t)(uintptr_t)__kernel_image_end;
    serial_write("[kernel] linked virtual image: start="); serial_hex(start);
    serial_write(" end="); serial_hex(end);
    if (end >= start) {
        serial_write(" size="); serial_dec(end - start);
    }
    serial_write("\r\n");
    serial_write("[kernel] physical load range: unproven by current linker and boot requests\r\n");
}
