#!/usr/bin/env bash
set -eu
if [ "$#" -ne 3 ]; then echo "usage: check-serial.sh serial.log kernel.elf kernel.iso" >&2; exit 2; fi
log="$1"
kernel="$2"
iso="$3"
test -s "$log"
test -s "$kernel"
test -s "$iso"
if test "$log" -ot "$kernel" || test "$log" -ot "$iso"; then
    echo "serial evidence is older than the kernel or ISO" >&2
    exit 1
fi
grep -F "NEXUS OS - Phase 2E" "$log" >/dev/null
grep -F "[init] serial console: ready" "$log" >/dev/null
grep -F "[init] kernel entry: reached" "$log" >/dev/null
grep -F "[init] architectural state: inspected; inherited GDT/TSS/paging preserved" "$log" >/dev/null
grep -F "[kernel] linked virtual image: start=" "$log" >/dev/null
grep -Eq "^\[kernel\] physical load range: base=0x[0-9a-fA-F]+ end=0x[0-9a-fA-F]+ size=[1-9][0-9]* source=Limine executable-address response[[:space:]]*$" "$log"
grep -F "[arch] segments cs=" "$log" >/dev/null
grep -F "[arch] gdtr base=" "$log" >/dev/null
grep -F "[arch] paging cr0=" "$log" >/dev/null
grep -F "[arch] paging mode: inherited long mode paging; NEXUS did not replace CR3" "$log" >/dev/null
grep -Eq "^\[paging\] CR3 root candidate: physical=0x[0-9a-fA-F]+ hhdm=0x[0-9a-fA-F]+[[:space:]]*$" "$log"
grep -Eq "^\[paging\] HHDM root coverage: confirmed; page_size=(4096|2097152|1073741824)[[:space:]]*$" "$log"
grep -Eq "^\[paging\] kernel mapping: physical=0x[0-9a-fA-F]+ page_size=(4096|2097152|1073741824) level=[1-4][[:space:]]*$" "$log"
grep -F "[paging] kernel mapping consistency: confirmed" "$log" >/dev/null
grep -Eq "^\[boot\] HHDM offset: 0x[0-9a-fA-F]+ \(protocol-provided virtual offset\)[[:space:]]*$" "$log"
grep -F "[boot] modules: no response; no modules configured" "$log" >/dev/null
grep -Eq "^\[boot\] framebuffer: count=[1-9][0-9]*[[:space:]]*$" "$log"
grep -F "[reserve] frame zero: reserved" "$log" >/dev/null
grep -F "[reserve] bootloader-reclaimable: retained until boot-info copy and verified reclamation" "$log" >/dev/null
grep -Eq "^\[reserve\] identified physical ranges: [1-9][0-9]*[[:space:]]*$" "$log"
grep -F "[reserve] model: conservative diagnostic coverage complete for identified boot objects" "$log" >/dev/null
grep -F "[init] exception diagnostics: ready" "$log" >/dev/null
grep -Eq '^\[init\] memory map entries: 0x?[1-9][0-9]*[[:space:]]*$|^\[init\] memory map entries: [1-9][0-9]*[[:space:]]*$' "$log"
test "$(grep -Ec '^\[mem\] base=0x[0-9a-fA-F]+ length=0x[0-9a-fA-F]+ type=0x[0-9a-fA-F]+[[:space:]]*$' "$log")" -gt 0
grep -Eq '^\[mem\] usable regions: [1-9][0-9]*[[:space:]]*$' "$log"
grep -Eq '^\[mem\] usable bytes: [1-9][0-9]*[[:space:]]*$' "$log"
grep -Eq '^\[mem\] usable MiB: [1-9][0-9]*[[:space:]]*$' "$log"
grep -F "[init] physical memory allocator: unit-tested only; runtime integration deferred" "$log" >/dev/null
grep -F "[init] scheduler: deferred to Phase 2" "$log" >/dev/null
grep -F "[init] IPC and recovery: deferred to later phases" "$log" >/dev/null
grep -F "[halt] initialization complete; controlled halt" "$log" >/dev/null
if grep -F "NEXUS PANIC:" "$log" >/dev/null; then
    echo "serial evidence contains a kernel panic" >&2
    exit 1
fi
echo "serial acceptance checks passed"
