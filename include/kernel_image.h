#ifndef NEXUS_KERNEL_IMAGE_H
#define NEXUS_KERNEL_IMAGE_H
#include "limine.h"
#include "memory_ranges.h"
int nexus_kernel_image_physical_range(const struct limine_executable_address_response *response,struct nexus_memory_range *range);
void nexus_kernel_image_print(const struct limine_executable_address_response *response);
#endif
