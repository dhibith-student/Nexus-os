# NEXUS OS initial architecture

## Phase 1 decision

The target is x86-64. The boot protocol is Limine, because it provides a maintained x86-64 boot path and structured boot requests for memory-map information without forcing kernel code to parse BIOS data directly. The build produces an ELF64 kernel and a bootable ISO containing Limine BIOS assets. No physical disk or host boot record is modified.

The selected build strategy is freestanding C with a small x86-64 assembly entry shim. The default WSL toolchain names are `x86_64-linux-gnu-gcc`, `x86_64-linux-gnu-ld`, and `x86_64-linux-gnu-objcopy`; an `x86_64-elf-*` cross-toolchain can be selected with `make TARGET=x86_64-elf`. The compiler must produce `-mno-red-zone` code because interrupts and future low-level entry paths may use the current stack. `-ffreestanding` removes hosted-runtime assumptions; `-fno-stack-protector` avoids an unavailable compiler runtime; `-nostdlib -nodefaultlibs -nostartfiles` prevents hosted libraries and startup objects from entering the image. The linker script defines the Limine request sections and the kernel entry point.

## Entry and diagnostics

Limine loads the ELF image and transfers control to `_start`, which clears direction flags and calls `kernel_main`. `kernel_main` initializes COM1 at I/O port `0x3f8`, prints the NEXUS banner and stage messages, reports the number and types of memory-map entries returned by Limine, and executes `cli; hlt` in a loop. Serial output is 115200 baud, 8 data bits, no parity, one stop bit. The first validation uses QEMU `-serial stdio -display none`.

The panic path prints a fixed diagnostic string and halts. It is intentionally not an exception or interrupt subsystem.

## Memory-map strategy

The Limine memory-map request is read-only in Phase 1. The kernel reports entry count and each entry's base, length, and type in hexadecimal. No allocator, page-table mutation, or memory ownership claim is made yet.

## Planned interfaces (not implemented)

- Task model: a kernel-owned task record will contain an ID, state, saved architectural context, address-space reference, and scheduling metadata. The scheduler interface will enqueue, dequeue, block, wake, and select tasks. Context switching will not be added until its assembly contract is reviewed and tested.
- IPC: a bounded message object will carry a sender task ID, receiver endpoint ID, length, and payload ownership state. Send transfers a copied bounded payload into a kernel-owned queue; receive transfers data out and invalidates the queue slot. Endpoint generation numbers prevent stale references.
- Service registry and supervisor: the registry will track service name, instance generation, endpoint handles, state, restart count, and restart-window timestamps. The supervisor will validate eligibility and bounded restart policy before creating a new instance.
- Recovery lifecycle: `RUNNING -> FAILED -> STOPPING -> RESTARTING -> RECONNECTED`, with `QUARANTINED` for policy exhaustion. Old endpoints are invalidated by generation mismatch before the replacement is published.

Kernel-mode tasks and isolated user-mode services are distinct design targets. User-mode isolation is not implemented or claimed in Phase 1; it requires privilege transitions, page-table/address-space ownership, and tests.

## Deferred work

Interrupt handling, GDT/IDT, paging and allocators, task switching, scheduling, IPC, privilege separation, user-mode services, service supervision, endpoint reconnection, persistent logging, and recovery tests are deferred. No Phase 1 document should imply that they already work.
