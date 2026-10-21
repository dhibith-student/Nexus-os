#!/usr/bin/env bash
set -eu
if [ "$#" -ne 1 ]; then echo "usage: check-exception.sh exception.log" >&2; exit 2; fi
log="$1"
test -s "$log"
grep -F "[test] triggering invalid opcode exception" "$log" >/dev/null
grep -F "[exception] vector=0x0000000000000006 name=invalid opcode" "$log" >/dev/null
grep -F "[exception] rip=" "$log" >/dev/null
grep -F "cs=" "$log" >/dev/null
grep -F "rflags=" "$log" >/dev/null
grep -F "error=" "$log" >/dev/null
grep -F "[exception] cr0=" "$log" >/dev/null
grep -F "cr2=" "$log" >/dev/null
grep -F "cr3=" "$log" >/dev/null
grep -F "cr4=" "$log" >/dev/null
grep -F "[exception] fatal exception; controlled halt" "$log" >/dev/null
if grep -F "[halt] initialization complete; controlled halt" "$log" >/dev/null; then echo "exception test reached normal halt unexpectedly" >&2; exit 1; fi
echo "exception acceptance checks passed"
