#ifndef NEXUS_LIMINE_H
#define NEXUS_LIMINE_H
#include <stdint.h>
#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88ULL, 0x0a82e883a194f07bULL
#define LIMINE_HHDM_REQUEST_ID 0x48dcf1cb8ad2b852ULL, 0x63984e959a98244BULL
#define LIMINE_FRAMEBUFFER_REQUEST_ID 0x9d5827dcd881dd75ULL, 0xa3148604f6fab11BULL
#define LIMINE_MEMORY_MAP_REQUEST_ID 0x67cf3d9d378a806fULL, 0xe304acdfc50c3c62ULL
#define LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID 0x71ba76863cc55f63ULL, 0xb2644a48c516a487ULL
#define LIMINE_MODULE_REQUEST_ID 0x3e7e279702be32afULL, 0xca1c4f3bd1280ceeULL
#define LIMINE_BASE_REVISION(N) 0xf9562b2d5c95a6c8ULL, 0x6a7b384944536bdcULL, (N)
#define LIMINE_REQUESTS_START_MARKER 0xf6b8f4b39de7d1aeULL, 0xfab91a6940fcb9cfULL, 0x785c6ed015d3e316ULL, 0x181e920a7852b9d9ULL
#define LIMINE_REQUESTS_END_MARKER 0xadc0e0531bb10d03ULL, 0x9572709f31764c62ULL
struct limine_hhdm_response { uint64_t revision; uint64_t offset; };
struct limine_hhdm_request { uint64_t id[4]; uint64_t revision; struct limine_hhdm_response *response; };
struct limine_uuid { uint32_t a; uint16_t b; uint16_t c; uint8_t d[8]; };
struct limine_file { uint64_t revision; void *address; uint64_t size; char *path; char *string; uint32_t media_type; uint32_t unused; uint8_t tftp_ipv4[4]; uint32_t tftp_port; uint32_t partition_index; uint32_t mbr_disk_id; struct limine_uuid gpt_disk_uuid; struct limine_uuid gpt_part_uuid; struct limine_uuid part_uuid; };
struct limine_video_mode { uint64_t pitch; uint64_t width; uint64_t height; uint16_t bpp; uint8_t memory_model; uint8_t red_mask_size; uint8_t red_mask_shift; uint8_t green_mask_size; uint8_t green_mask_shift; uint8_t blue_mask_size; uint8_t blue_mask_shift; };
#define LIMINE_FRAMEBUFFER_RGB 1
struct limine_framebuffer { void *address; uint64_t width; uint64_t height; uint64_t pitch; uint16_t bpp; uint8_t memory_model; uint8_t red_mask_size; uint8_t red_mask_shift; uint8_t green_mask_size; uint8_t green_mask_shift; uint8_t blue_mask_size; uint8_t blue_mask_shift; uint8_t unused[7]; uint64_t edid_size; void *edid; uint64_t mode_count; struct limine_video_mode **modes; };
struct limine_framebuffer_response { uint64_t revision; uint64_t framebuffer_count; struct limine_framebuffer **framebuffers; };
struct limine_framebuffer_request { uint64_t id[4]; uint64_t revision; struct limine_framebuffer_response *response; };
struct limine_executable_address_response { uint64_t revision; uint64_t physical_base; uint64_t virtual_base; };
struct limine_executable_address_request { uint64_t id[4]; uint64_t revision; struct limine_executable_address_response *response; };
struct limine_memmap_entry { uint64_t base; uint64_t length; uint64_t type; };
struct limine_memmap_response { uint64_t revision; uint64_t entry_count; struct limine_memmap_entry **entries; };
struct limine_memmap_request { uint64_t id[4]; uint64_t revision; struct limine_memmap_response *response; };
#define LIMINE_INTERNAL_MODULE_REQUIRED (1U << 0)
#define LIMINE_INTERNAL_MODULE_COMPRESSED (1U << 1)
struct limine_internal_module { const char *path; const char *string; uint64_t flags; };
struct limine_module_response { uint64_t revision; uint64_t module_count; struct limine_file **modules; };
struct limine_module_request { uint64_t id[4]; uint64_t revision; struct limine_module_response *response; uint64_t internal_module_count; struct limine_internal_module **internal_modules; };
enum limine_memmap_type { LIMINE_MEMMAP_USABLE=0, LIMINE_MEMMAP_RESERVED=1, LIMINE_MEMMAP_ACPI_RECLAIMABLE=2, LIMINE_MEMMAP_ACPI_NVS=3, LIMINE_MEMMAP_BAD_MEMORY=4, LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE=5, LIMINE_MEMMAP_EXECUTABLE_AND_MODULES=6, LIMINE_MEMMAP_FRAMEBUFFER=7, LIMINE_MEMMAP_RESERVED_MAPPED=8 };
#endif
