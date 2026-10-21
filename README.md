# NEXUS OS

NEXUS OS is a real freestanding x86-64 kernel prototype. Phase 2D audits physical ownership and paging without enabling a live allocator.

## Current status

The kernel boots through Limine v12.9.3, reports inherited x86-64 architectural state, prints the Limine memory map and usable totals, reports its linked virtual image bounds, and reaches controlled halt. The physical-memory core remains unit-tested only. It is not initialized from the live map because the repository cannot yet prove physical reservations for the kernel image, Limine data, modules, framebuffer, or allocator metadata.

The linker-derived image is a virtual span from 0xffffffff80000000 to 0xffffffff80005010, 20496 bytes including BSS and alignment gaps. Its physical load range is explicitly unproven. The running kernel does not equate these virtual addresses with physical addresses.

## Phase 2D conclusions

The Limine request retained by NEXUS is only the memory-map request. The header defines type 0 as usable; reserved, ACPI, bad-memory, bootloader-reclaimable, executable/modules, and framebuffer entries are excluded from immediate allocation. No module, framebuffer, or boot-info response is currently requested or retained.

The ELF has entry point 0xffffffff80001000. Its loadable segment PhysAddr fields equal the higher-half virtual addresses because the linker has no physical AT mapping. This is not evidence of physical placement. NEXUS therefore does not activate PMM or reclaim bootloader memory.

## Toolchain and commands

The documented WSL environment uses x86_64-linux-gnu-gcc/binutils, GNU Make, Limine v12.9.3 assets, xorriso, and qemu-system-x86_64.

    make clean
    make
    make test-memory-accounting
    make test-physical-memory
    make test-memory-ranges
    make test-descriptors
    make check

make check creates fresh build/serial.log with the documented headless QEMU settings, accepts only the expected timeout, and validates all existing diagnostics. The invalid-opcode regression remains separate:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

The timeout is expected because the guest intentionally halts. Images are disposable files under build/; no physical disk or host boot record is touched. Read AGENTS.md before every change.
