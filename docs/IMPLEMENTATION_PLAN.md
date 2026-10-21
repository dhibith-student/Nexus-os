# Implementation plan

## Phase 1 — boot milestone

1. Inspect host, WSL, compiler, linker, build tools, QEMU, and Limine assets.
2. Keep the repository rulebook and architecture decision under version control.
3. Build a freestanding ELF64 kernel with Limine.
4. Create an ISO in `build/` and boot it in QEMU with serial output.
5. Verify the banner, initialization stages, memory-map report, and controlled halt.

The acceptance evidence is the exact build output, QEMU command, captured serial output, and a successful exit after the kernel's halt condition. Compilation without QEMU evidence is not sufficient.

## Windows/WSL setup when dependencies are missing

These commands install packages inside the existing Ubuntu WSL2 distribution; they do not modify physical disks or Windows boot configuration:

```powershell
wsl --update
wsl -d Ubuntu -- sudo apt update
wsl -d Ubuntu -- sudo apt install -y gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu make xorriso qemu-system-x86 git curl ca-certificates
```

Verify the toolchain before building:

```powershell
wsl -d Ubuntu -- bash -lc 'command -v x86_64-linux-gnu-gcc x86_64-linux-gnu-ld make xorriso qemu-system-x86_64'
```

Limine binary assets must then be placed under `tools/limine/` with the three filenames required by the Makefile: `limine-bios.sys`, `limine-bios-cd.bin`, and `limine-uefi-cd.bin`. Obtain those files from a pinned Limine binary release and record the release version in the next Git commit; do not silently substitute assets from another boot protocol.

## Phase 2 — kernel foundations

Add reviewed GDT/IDT setup, exception diagnostics, physical memory accounting, paging, and a tested task/context representation. Preserve the Phase 1 boot image as a Git milestone.

## Phase 3 — scheduling and IPC

Add a bounded scheduler and kernel IPC endpoints only after their interfaces are agreed. Add deterministic unit/fixture tests and QEMU integration evidence. Do not claim isolation until privilege and memory mechanisms are present.

## Phase 4 — services and recovery

Add the registry, supervisor state machine, bounded restart policy, service generations, endpoint invalidation, reconnection, and failure-injection tests. Member B must integrate only against documented kernel interfaces.

## Required evidence per milestone

Each milestone must record changed files, commands, observed results, known defects, and the next milestone. A known-working bootable kernel must remain recoverable through a Git commit.
