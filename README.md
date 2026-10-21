# NEXUS OS

NEXUS OS is a real freestanding x86-64 kernel prototype. Phase 2C adds a conservative, unit-tested physical-memory allocator core. The running kernel still reports diagnostics and halts; it does not activate physical allocation.

## Current status

The kernel boots through Limine v12.9.3, reports inherited x86-64 architectural state, handles the existing five exception vectors, reports the Limine memory map and usable totals, and reaches controlled halt. The PMM core accepts only Limine type-0 usable regions, aligns them to 4096-byte frames, always reserves frame zero, and applies explicit caller-supplied reserved ranges. It uses caller-supplied allocation and reservation bitmaps and rejects unsafe or ambiguous inputs.

The PMM core is compiled into the kernel but is not called by the boot path. This is intentional: the current project cannot yet prove the physical ranges occupied by the linked kernel, Limine data, modules, framebuffer, or a safe physical location for the allocator bitmap. Bootloader-reclaimable memory remains reserved. No heap, page-table mutation, CR3 change, scheduler, IPC, user mode, supervision, or recovery exists.

## Toolchain and commands

The documented WSL environment uses x86_64-linux-gnu-gcc/binutils, GNU Make, Limine v12.9.3 assets, xorriso, and qemu-system-x86_64.

    make clean
    make
    make test-memory-accounting
    make test-physical-memory
    make test-descriptors
    make check

make check creates fresh build/serial.log by booting QEMU with the documented headless settings, accepts only the expected timeout, and validates the serial assertions. The invalid-opcode regression remains separate:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

The timeout is expected because the guest intentionally halts. Images are disposable files under build/; no physical disk or host boot record is touched. Read AGENTS.md before every change.
