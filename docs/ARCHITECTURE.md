# NEXUS OS architecture

## Phase 2B setup

The target is x86-64 and the boot protocol is Limine v12.9.3. Limine supplies the ELF entry path and structured boot requests, including the memory map. The build produces a higher-half ELF64 kernel and a BIOS/UEFI-capable ISO using pinned assets; no physical disk is modified.

The WSL toolchain is x86_64-linux-gnu-gcc/ld/objcopy, with an equivalent x86_64-elf-* toolchain supported by the Makefile. Kernel compilation is freestanding C plus minimal assembly with -ffreestanding, -fno-stack-protector, -fno-pie, -mcmodel=kernel, -mno-red-zone, and MMX/SSE disabled. Linking uses -nostdlib and linker.ld; the image is linked at 0xffffffff80000000.

Limine transfers control to _start, which calls kernel_main. The kernel is in ring 0 and uses the existing boot stack. There is no privilege transition, process isolation, context switching, scheduler, allocator, or user address space. COM1 at I/O port 0x3f8 is configured for 115200 8N1.

## Inherited GDT, TSS, and IDT state

Phase 2B reads, but does not replace, the architectural state established before kernel_main. The diagnostic uses SGDT, SIDT, STR, segment-register reads, and control-register reads. In the verified QEMU boot it observed:

- CS=0x28 and DS/ES/SS=0x30, all used at ring 0.
- TR=0x38 and GDTR limit=0x47, indicating that Limine supplied a GDT and loaded a task-register selector before entry.
- GDTR base=0xffff800000018538 in this QEMU run. This address and selector layout are observations, not hard-coded kernel assumptions.
- IDTR base=0 and limit=0 before NEXUS initializes its existing exception IDT.
- No TSS descriptor is loaded or owned by NEXUS. The nonzero TR is reported only; its TSS stack fields are not used for a privilege transition.

NEXUS therefore preserves the inherited GDT, TSS, segment selectors, and boot stack. It does not execute LGDT, LTR, reload segment registers, or enable user-mode transitions.

## Paging state

NEXUS reads CR0, CR2, CR3, CR4, and EFER. The verified QEMU run reported CR0=0x80010011, CR2=0, CR3=0x7f9b000, CR4=0x20, and EFER=0xd00. CR0.PG, CR4.PAE, and EFER.LMA show inherited long-mode paging. The kernel does not walk the inherited page tables, replace CR3, change mappings, enable or disable paging, allocate pages, or claim ownership of physical memory. The linker address and boot behavior are compatible with the mappings supplied by Limine, but the current diagnostic does not establish a complete mapping inventory.

## Descriptor and TSS construction foundations

The descriptor module provides a pure constructor for 64-bit segment descriptors and a packed 104-byte x86-64 TSS layout initializer. Unit tests validate the expected kernel code/data descriptor encodings and zeroed TSS fields with the I/O-map base set to the structure size. These helpers are not installed into the processor and do not constitute privilege separation.

## Existing exception and memory diagnostics

The kernel retains interrupt gates for vectors 0, 6, 8, 13, and 14. Their assembly stubs normalize error-code and non-error-code entries and the C dispatcher reports useful state before halting. Limine memory accounting validates pointers within the boot-stage limits, counts only type-0 usable entries, checks overflow, and reports bytes and integer MiB.

## Planned but unimplemented interfaces

Task records will later carry IDs, state, saved context, address-space references, and scheduling metadata. IPC will use bounded copied messages and generation-checked endpoints. A service registry and supervisor will track instances, bounded restart policy, endpoint generations, and recovery states. User-mode isolation requires explicit privilege transitions, page-table ownership, and tests and is not claimed.

Deferred: NEXUS-owned GDT/TSS installation, TSS stack allocation/mapping, complete exception coverage, external interrupts, page-table management, physical/virtual allocation, task switching, scheduling, IPC, privilege separation, services, supervision, endpoint reconnection, and recovery tests.
