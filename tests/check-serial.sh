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
grep -F "NEXUS OS - Phase 2A" "$log" >/dev/null
grep -F "[init] serial console: ready" "$log" >/dev/null
grep -F "[init] kernel entry: reached" "$log" >/dev/null
grep -F "[init] exception diagnostics: ready" "$log" >/dev/null
grep -Eq '^\[init\] memory map entries: 0x?[1-9][0-9]*[[:space:]]*$|^\[init\] memory map entries: [1-9][0-9]*[[:space:]]*$' "$log"
test "$(grep -Ec '^\[mem\] base=0x[0-9a-fA-F]+ length=0x[0-9a-fA-F]+ type=0x[0-9a-fA-F]+[[:space:]]*$' "$log")" -gt 0
grep -Eq '^\[mem\] usable regions: [1-9][0-9]*[[:space:]]*$' "$log"
grep -Eq '^\[mem\] usable bytes: [1-9][0-9]*[[:space:]]*$' "$log"
grep -Eq '^\[mem\] usable MiB: [1-9][0-9]*[[:space:]]*$' "$log"
grep -F "[init] scheduler: deferred to Phase 2" "$log" >/dev/null
grep -F "[init] IPC and recovery: deferred to later phases" "$log" >/dev/null
grep -F "[halt] initialization complete; controlled halt" "$log" >/dev/null
if grep -F "NEXUS PANIC:" "$log" >/dev/null; then
    echo "serial evidence contains a kernel panic" >&2
    exit 1
fi
echo "serial acceptance checks passed"
