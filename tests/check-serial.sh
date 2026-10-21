#!/usr/bin/env bash
set -eu
log="${1:?usage: check-serial.sh serial.log}"
grep -F "NEXUS OS - Phase 1" "$log"
grep -F "[init] serial console: ready" "$log"
grep -F "[init] memory map entries:" "$log"
grep -F "[halt] initialization complete; controlled halt" "$log"
echo "serial acceptance checks passed"
