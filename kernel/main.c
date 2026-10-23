#include <stdint.h>
#include "arch_state.h"
#include "boot_info.h"
#include "exceptions.h"
#include "kernel_image.h"
#include "limine.h"
#include "memory_accounting.h"
#include "memory_ranges.h"
#include "paging_audit.h"
#include "serial.h"
#ifndef NEXUS_EXCEPTION_TEST
#define NEXUS_EXCEPTION_TEST 0
#endif
__attribute__((used,section(".limine_requests"))) volatile struct limine_hhdm_request hhdm_request={{LIMINE_COMMON_MAGIC,LIMINE_HHDM_REQUEST_ID},0,0};
__attribute__((used,section(".limine_requests"))) volatile struct limine_framebuffer_request framebuffer_request={{LIMINE_COMMON_MAGIC,LIMINE_FRAMEBUFFER_REQUEST_ID},0,0};
__attribute__((used,section(".limine_requests"))) volatile struct limine_memmap_request memmap_request={{LIMINE_COMMON_MAGIC,LIMINE_MEMORY_MAP_REQUEST_ID},0,0};
__attribute__((used,section(".limine_requests"))) volatile struct limine_executable_address_request executable_address_request={{LIMINE_COMMON_MAGIC,LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID},0,0};
__attribute__((used,section(".limine_requests"))) volatile struct limine_module_request module_request={{LIMINE_COMMON_MAGIC,LIMINE_MODULE_REQUEST_ID},0,0,0,0};
__attribute__((used,section(".limine_requests"))) volatile uint64_t limine_base_revision[]={LIMINE_BASE_REVISION(6)};
__attribute__((used,section(".limine_requests_start"))) volatile uint64_t limine_requests_start_marker[]={LIMINE_REQUESTS_START_MARKER};
__attribute__((used,section(".limine_requests_end"))) volatile uint64_t limine_requests_end_marker[]={LIMINE_REQUESTS_END_MARKER};
__attribute__((noreturn)) static void halt_forever(void){for(;;)__asm__ volatile("cli; hlt");}
__attribute__((noreturn)) static void panic(const char *message){serial_write("NEXUS PANIC: ");serial_write(message);serial_write("\r\n");halt_forever();}
static void print_memory_map(const struct limine_memmap_response *response){serial_write("[init] memory map entries: ");serial_dec(response->entry_count);serial_write("\r\n");for(uint64_t i=0;i<response->entry_count;++i){const struct limine_memmap_entry *e=response->entries[i];if(e==0)panic("null Limine memory-map entry");serial_write("[mem] base=");serial_hex(e->base);serial_write(" length=");serial_hex(e->length);serial_write(" type=");serial_hex(e->type);serial_write("\r\n");}}
static int reserve_physical(struct nexus_reservation_set *set,struct nexus_memory_range range){struct nexus_memory_range aligned;if(!nexus_range_align_reservation(range,&aligned))return 0;return nexus_reservation_add(set,aligned.start,aligned.end-aligned.start);}
static void print_boot_objects(const struct limine_hhdm_response *hhdm,const struct limine_module_response *modules,const struct limine_framebuffer_response *framebuffer){
    if(hhdm==0) serial_write("[boot] HHDM: response unavailable; physical-pointer conversion unknown\r\n");
    else {serial_write("[boot] HHDM offset: ");serial_hex(hhdm->offset);serial_write(" (protocol-provided virtual offset)\r\n");}
    if(modules==0) serial_write("[boot] modules: no response; no modules configured\r\n");
    else {if(!nexus_boot_response_array_valid(modules->module_count,modules->modules,NEXUS_MAX_BOOT_RESPONSE_ENTRIES))panic("invalid Limine module response");serial_write("[boot] modules: count=");serial_dec(modules->module_count);serial_write("\r\n");for(uint64_t i=0;i<modules->module_count;++i){struct limine_file *f=modules->modules[i];if(f==0||f->address==0)panic("invalid Limine module file");serial_write("[boot] module hhdm=");serial_hex((uint64_t)(uintptr_t)f->address);serial_write(" size=");serial_dec(f->size);serial_write("\r\n");}}
    if(framebuffer==0) serial_write("[boot] framebuffer: no response; no framebuffer retained\r\n");
    else {if(!nexus_boot_response_array_valid(framebuffer->framebuffer_count,framebuffer->framebuffers,NEXUS_MAX_BOOT_RESPONSE_ENTRIES))panic("invalid Limine framebuffer response");serial_write("[boot] framebuffer: count=");serial_dec(framebuffer->framebuffer_count);serial_write("\r\n");for(uint64_t i=0;i<framebuffer->framebuffer_count;++i){struct limine_framebuffer *f=framebuffer->framebuffers[i];if(f==0||f->address==0)panic("invalid Limine framebuffer");serial_write("[boot] framebuffer hhdm=");serial_hex((uint64_t)(uintptr_t)f->address);serial_write(" pitch=");serial_dec(f->pitch);serial_write(" height=");serial_dec(f->height);serial_write("\r\n");}}
}
static int build_reservation_model(const struct limine_memmap_response *map,const struct limine_executable_address_response *exec,const struct limine_hhdm_response *hhdm,const struct limine_module_response *modules,const struct limine_framebuffer_response *framebuffer,uint64_t *count){
    struct nexus_reservation_set set={0};struct nexus_memory_range range;int complete=1;
    if(!nexus_reservation_add(&set,0,4096))complete=0;
    if(!nexus_kernel_image_physical_range(exec,&range)||!reserve_physical(&set,range))complete=0;
    for(uint64_t i=0;i<map->entry_count;++i){const struct limine_memmap_entry *e=map->entries[i];if(e->type==LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE){if(!nexus_range_from_length(e->base,e->length,&range)||!reserve_physical(&set,range))complete=0;}}
    if(modules!=0){for(uint64_t i=0;i<modules->module_count;++i){struct limine_file *f=modules->modules[i];if(hhdm==0||!nexus_boot_hhdm_range_to_physical(hhdm->offset,(uint64_t)(uintptr_t)f->address,f->size,&range)||!reserve_physical(&set,range))complete=0;}}
    if(framebuffer!=0){for(uint64_t i=0;i<framebuffer->framebuffer_count;++i){struct limine_framebuffer *f=framebuffer->framebuffers[i];uint64_t size;if(f->height!=0&&f->pitch>~(uint64_t)0/f->height)complete=0;else{size=f->pitch*f->height;if(hhdm==0||!nexus_boot_hhdm_range_to_physical(hhdm->offset,(uint64_t)(uintptr_t)f->address,size,&range)||!reserve_physical(&set,range))complete=0;}}}
    *count=set.count;return complete;
}
extern char __kernel_image_start[];

static void print_paging_audit(const struct limine_memmap_response *map,
                               const struct nexus_arch_state *state,
                               const struct limine_hhdm_response *hhdm,
                               const struct limine_executable_address_response *exec) {
    uint64_t root_physical;
    uint64_t root_virtual;
    struct nexus_paging_mapping mapping;
    int status;

    if (hhdm == 0 ||
        !nexus_paging_cr3_physical(state->cr3, &root_physical) ||
        !nexus_paging_physical_to_hhdm(hhdm->offset, root_physical, 48, &root_virtual)) {
        serial_write("[paging] CR3 root: not proven safely accessible through HHDM\r\n");
        return;
    }
    serial_write("[paging] CR3 root candidate: physical="); serial_hex(root_physical);
    serial_write(" hhdm="); serial_hex(root_virtual); serial_write("\r\n");

    status = nexus_paging_find_mapping(map, hhdm->offset, state->cr0, state->cr3,
                                       state->cr4, state->efer, root_virtual,
                                       &mapping);
    if (status == NEXUS_PAGING_WALK_MAPPED) {
        serial_write("[paging] HHDM root coverage: confirmed; page_size=");
        serial_dec(mapping.page_size); serial_write("\r\n");
    } else {
        serial_write("[paging] HHDM root coverage: not proven by bounded table walk\r\n");
    }

    status = nexus_paging_find_mapping(map, hhdm->offset, state->cr0, state->cr3,
                                       state->cr4, state->efer,
                                       (uint64_t)(uintptr_t)__kernel_image_start,
                                       &mapping);
    if (status == NEXUS_PAGING_WALK_MAPPED) {
        uint64_t offset = (uint64_t)(uintptr_t)__kernel_image_start %
                          mapping.page_size;
        uint64_t physical = mapping.physical_base + offset;
        serial_write("[paging] kernel mapping: physical="); serial_hex(physical);
        serial_write(" page_size="); serial_dec(mapping.page_size);
        serial_write(" level="); serial_dec(mapping.level); serial_write("\r\n");
        struct nexus_memory_range expected;
        if (nexus_kernel_image_physical_range(exec, &expected) &&
            physical == expected.start) {
            serial_write("[paging] kernel mapping consistency: confirmed\r\n");
        } else {
            serial_write("[paging] kernel mapping consistency: mismatch or physical source unavailable\r\n");
        }
    } else {
        serial_write("[paging] kernel mapping: not proven by bounded table walk\r\n");
    }
}

static void trigger_exception_test(void){
#if NEXUS_EXCEPTION_TEST
    serial_write("[test] triggering invalid opcode exception\r\n");__asm__ volatile("ud2");
#endif
}
void kernel_main(void){
    serial_init();serial_write("NEXUS OS - Phase 2E\r\n");serial_write("[init] serial console: ready\r\n");serial_write("[init] kernel entry: reached\r\n");
    struct nexus_arch_state state;nexus_arch_state_read(&state);nexus_arch_state_print(&state);serial_write("[init] architectural state: inspected; inherited GDT/TSS/paging preserved\r\n");
    nexus_kernel_image_print(executable_address_request.response);
    exceptions_init();serial_write("[init] exception diagnostics: ready (vectors 0, 6, 8, 13, 14)\r\n");
    const struct limine_memmap_response *map=memmap_request.response;if(map==0||map->entries==0||map->entry_count>NEXUS_MAX_MEMMAP_ENTRIES)panic("invalid Limine memory-map response");
    print_memory_map(map);struct nexus_memory_summary summary;if(!nexus_memory_accounting(map,&summary))panic("memory accounting validation failed");
    print_paging_audit(map,&state,hhdm_request.response,executable_address_request.response);
    serial_write("[mem] usable regions: ");serial_dec(summary.usable_regions);serial_write("\r\n[mem] usable bytes: ");serial_dec(summary.usable_bytes);serial_write("\r\n[mem] usable MiB: ");serial_dec(summary.usable_bytes/(1024ULL*1024ULL));serial_write("\r\n");
    print_boot_objects(hhdm_request.response,module_request.response,framebuffer_request.response);
    uint64_t reservations=0;int complete=build_reservation_model(map,executable_address_request.response,hhdm_request.response,module_request.response,framebuffer_request.response,&reservations);
    serial_write("[reserve] frame zero: reserved\r\n");serial_write("[reserve] bootloader-reclaimable: retained until boot-info copy and verified reclamation\r\n");serial_write("[reserve] identified physical ranges: ");serial_dec(reservations);serial_write("\r\n");
    if(complete)serial_write("[reserve] model: conservative diagnostic coverage complete for identified boot objects\r\n");else serial_write("[reserve] model: incomplete; unresolved address or overlap prevents PMM activation\r\n");
    serial_write("[init] physical memory allocator: unit-tested only; runtime integration deferred\r\n");
    trigger_exception_test();serial_write("[init] scheduler: deferred to Phase 2\r\n");serial_write("[init] IPC and recovery: deferred to later phases\r\n");serial_write("[halt] initialization complete; controlled halt\r\n");halt_forever();
}
