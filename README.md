# NEXUS OS

NEXUS OS is a real freestanding x86-64 kernel prototype. Phase 2E audits Limine boot information and physical load ownership without enabling a live allocator.

## Current status

The kernel boots through Limine v12.9.3, reports inherited x86-64 architectural state, prints the Limine memory map and usable totals, reports its linked virtual image bounds, obtains the Limine executable physical base and HHDM offset, audits boot-object reservations, and reaches controlled halt. The physical-memory core remains unit-tested only and is not initialized from the live map.

The linker-derived image is a virtual span from 0xffffffff80000000 to 0xffffffff80005010, 20496 bytes including BSS and alignment gaps. Limine runtime evidence supplies a physical base of 0x7f44000 in the tested QEMU boot, giving a physical span through 0x7f49010. The running kernel derives this only when the Limine virtual base matches the linked image start.

## Phase 2E conclusions

NEXUS requests HHDM, executable-address, memory-map, module, and framebuffer responses from the pinned Limine v12.9.3 protocol. Type 0 is the only type treated as usable; reserved, ACPI, bad-memory, bootloader-reclaimable, executable/modules, framebuffer, and reserved-mapped entries remain unavailable to the PMM. Optional module responses may be absent.

The ELF has entry point 0xffffffff80001000. Its loadable segment PhysAddr fields equal the higher-half virtual addresses because the linker has no physical AT mapping. Limine executable-address runtime evidence, rather than those ELF PAddr fields, establishes the tested physical load base. NEXUS does not activate PMM or reclaim bootloader memory.

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


## Phase 2E conclusions

The pinned Limine v12.9.3 protocol header and documentation define the HHDM, executable-address, module, and framebuffer responses used by this phase. The runtime reports HHDM offset 0xffff800000000000, one framebuffer, and no modules. Response pointers are used only as protocol-provided virtual pointers; the kernel does not dereference arbitrary physical addresses.

The reservation audit retains frame zero, the complete physical kernel image, all bootloader-reclaimable memory, and identified module/framebuffer ranges. Bootloader-reclaimable memory covers retained Limine response data under the protocol, but it is not reclaimed. The audit is diagnostic only, so PMM activation remains deferred.
