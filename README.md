# NEXUS OS

NEXUS OS is a real freestanding x86-64 kernel prototype. Phase 2B is implemented and verified: Limine boot, COM1 diagnostics, exception diagnostics, usable-memory accounting, inherited GDT/TSS/paging inspection, and descriptor/TSS construction tests. Scheduling, IPC, user-mode isolation, memory allocation, service supervision, and recovery remain unimplemented.

## Verified status

Normal headless QEMU boot reports the inherited ring-0 segmentation state and active long-mode paging, then reports the Limine memory map, 3 usable regions, 132702208 usable bytes (126 MiB with the tested 128 MiB QEMU configuration), deferred-feature messages, and controlled halt. NEXUS does not replace CR3, load a new GDT, load a TSS, or change paging.

The current QEMU environment reports CS=0x28, DS/ES/SS=0x30, TR=0x38, GDTR limit 0x47, CR0.PG=1, CR4.PAE=1, EFER.LMA=1, and a nonzero inherited CR3. The initially inherited IDTR is empty; NEXUS later installs its existing five exception gates. These values are diagnostics from the tested boot and are not portable assumptions for future boot environments.

The descriptor helpers construct legacy segment descriptors and a correctly sized x86-64 TSS layout without loading them. Focused tests validate descriptor encodings and TSS initialization. No privilege transition or user-mode execution exists.

## Toolchain and commands

The documented WSL environment uses x86_64-linux-gnu-gcc/binutils, GNU Make, Limine v12.9.3 assets, xorriso, and qemu-system-x86_64. See docs/ARCHITECTURE.md and docs/IMPLEMENTATION_PLAN.md.

    make limine-assets
    make
    make check
    make run

make check builds the kernel, runs the focused tests, boots QEMU with the documented headless settings, captures fresh serial evidence in build/serial.log, and validates it. The controlled exception test remains separate:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

The timeout is expected because the guest intentionally halts. Images are disposable files under build/; no physical disk or host boot record is touched. Read AGENTS.md before every change.
