#include <stdint.h>
#include "kernel_image.h"
#include "memory_ranges.h"
#include "serial.h"
extern char __kernel_image_start[]; extern char __kernel_image_end[];
int nexus_kernel_image_physical_range(const struct limine_executable_address_response *response,struct nexus_memory_range *range){uint64_t start=(uint64_t)(uintptr_t)__kernel_image_start,end=(uint64_t)(uintptr_t)__kernel_image_end;if(response==0||range==0||end<start||response->virtual_base!=start)return 0;return nexus_range_from_length(response->physical_base,end-start,range);}
void nexus_kernel_image_print(const struct limine_executable_address_response *response){uint64_t start=(uint64_t)(uintptr_t)__kernel_image_start,end=(uint64_t)(uintptr_t)__kernel_image_end;struct nexus_memory_range physical;serial_write("[kernel] linked virtual image: start=");serial_hex(start);serial_write(" end=");serial_hex(end);if(end>=start){serial_write(" size=");serial_dec(end-start);}serial_write("\r\n");if(nexus_kernel_image_physical_range(response,&physical)){serial_write("[kernel] physical load range: base=");serial_hex(physical.start);serial_write(" end=");serial_hex(physical.end);serial_write(" size=");serial_dec(physical.end-physical.start);serial_write(" source=Limine executable-address response\r\n");}else serial_write("[kernel] physical load range: unproven; executable-address response does not match linked image\r\n");}
