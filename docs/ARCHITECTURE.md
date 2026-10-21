# NEXUS OS architecture

## Phase 2C memory-map audit

The kernel requests Limine's memory-map response through the request structure in include/limine.h. Limine supplies an entry count and an array of pointers to entries containing physical base, byte length, and protocol memory type. Type 0 is the only type treated as immediately usable by the existing accounting code and the PMM core. Reserved, ACPI reclaimable, ACPI NVS, bad memory, bootloader reclaimable, executable/modules, and framebuffer types are excluded.

The current response does not identify the physical ranges occupied by the linked kernel image, Limine response structures, modules, framebuffer backing storage, or the allocator's own metadata. The linker places the kernel at the higher-half virtual address 0xffffffff80000000, while the source does not establish a physical load range or a complete virtual-to-physical mapping. Therefore the boot path does not instantiate a physical allocator. No memory is freed merely because it appears in a type-0 interval.

## Physical-memory core

kernel/physical_memory.c implements a fixed-region bitmap allocator core for later integration:

- Usable entries are converted to full 4096-byte frames using align-up for the base and align-down for the exclusive end.
- Zero-sized and sub-frame usable intervals contribute no frames.
- Frame zero is permanently reserved.
- Explicit reserved ranges are validated for base-plus-length overflow and reserve every overlapping frame.
- Overlapping normalized usable regions are rejected rather than double-counted.
- The caller supplies separate allocation and permanent-reservation bitmaps; no dynamic allocation bootstraps the core.
- Allocation scans regions in memory-map order and returns aligned physical addresses.
- Free accepts only aligned addresses belonging to a known frame, rejects frame zero, reserved frames, duplicate frees, and out-of-range addresses.
- Exhaustion returns failure.
- Bitmap capacity and all address/sum calculations are checked for overflow.

This core is compiled into the kernel to verify freestanding integration, but it is not invoked by kernel_main. Its bitmap pointers are not physical addresses and must not be dereferenced as physical memory by the kernel without a future mapping plan.

## Existing boot and architectural state

The target is x86-64 with Limine v12.9.3. Limine transfers control to _start in ring 0 with the inherited boot stack, GDT/TSS selectors, and long-mode paging. The kernel reads but does not replace GDT, TSS, IDT state before installing its existing five exception gates, CR3, or page tables. COM1 at I/O port 0x3f8 is configured for 115200 8N1.

The tested QEMU run reports CS=0x28, DS/ES/SS=0x30, TR=0x38, active CR0.PG/CR4.PAE/EFER.LMA, and a nonzero inherited CR3. These are observed diagnostics, not portable constants.

## Tests and limitations

tests/test-physical-memory.c uses freestanding synthetic Limine fixtures and tests empty maps, multiple usable regions, reserved gaps, unaligned boundaries, zero-sized regions, overflow, bitmap exhaustion, alignment, reserved-frame exclusion, invalid frees, duplicate frees, and reuse after a valid free. It does not claim that the live Limine map is safe for allocation.

Before activating the core, NEXUS must obtain reliable physical reservations for the kernel, bootloader data, modules, framebuffer and allocator metadata, plus a safe mapped location for bitmaps. It must also define page-table ownership and mappings. A dedicated TSS stack cannot safely be allocated until those prerequisites exist.

Scheduling, IPC, user-mode isolation, memory protection, service supervision, and recovery remain deferred.
