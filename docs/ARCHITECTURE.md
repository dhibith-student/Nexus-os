# NEXUS OS architecture

## Phase 2A setup

Target architecture: x86-64. Boot protocol: Limine v12.9.3, selected for its x86-64 ELF path and structured memory-map request. The build produces an ELF64 kernel and BIOS/UEFI-capable ISO using pinned assets; no physical disk is modified.

The WSL toolchain is x86_64-linux-gnu-gcc/ld/objcopy, with an equivalent x86_64-elf-* toolchain supported by the Makefile. Kernel compilation is freestanding C plus minimal assembly with -ffreestanding, -fno-stack-protector, -fno-pie, -mcmodel=kernel, -mno-red-zone, and MMX/SSE disabled. Linking uses -nostdlib and linker.ld; the image is linked at 0xffffffff80000000.

Limine transfers control to _start, which calls kernel_main. The kernel remains in bootloader-provided ring 0, uses the existing boot stack, and has no GDT/TSS, privilege transition, page-table ownership, user address space, or context switching. COM1 at I/O port 0x3f8 is configured for 115200 8N1 and receives synchronous diagnostics.

## Exception diagnostics

Phase 2A creates a 256-entry IDT and installs gates for vectors 0 divide error, 6 invalid opcode, 8 double fault, 13 general protection, and 14 page fault. Assembly stubs normalize error-code and non-error-code entries, preserve general registers, and call the C dispatcher. The dispatcher prints vector/name, RIP, CS, RFLAGS, normalized error code, and CR0/CR2/CR3/CR4, then disables interrupts and halts. The frame omits privilege-transition RSP/SS because no ring transition exists. This is not complete exception or external-interrupt coverage.

EXCEPTION_TEST=1 executes ud2 after normal initialization. tests/check-exception.sh validates the invalid-opcode diagnostic and fatal halt separately from normal acceptance.

## Memory accounting

The Limine response is read-only bootloader input. The implementation validates non-null response/entries, caps the count at 4096, checks base-plus-length overflow, and checks total-sum overflow. Only type 0 (usable) entries count; other types are excluded. Output uses bytes and integer MiB computed as bytes / (1024 * 1024). No allocator, page-table mutation, physical reservation, or arbitrary physical-pointer validation is implemented.

## Planned but unimplemented interfaces

- Task records will later carry IDs, state, saved context, address-space references, and scheduling metadata.
- IPC will use bounded copied messages and generation-checked endpoints.
- A service registry and supervisor will track instances, bounded restart policy, endpoint generations, and recovery states.
- User-mode isolation requires privilege transitions and page-table ownership and is not claimed.

Deferred: complete exception coverage, GDT/TSS, external interrupts, paging/allocation, task switching, scheduling, IPC, privilege separation, services, supervision, endpoint reconnection, and recovery tests.
