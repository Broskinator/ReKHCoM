#!/bin/sh
# Resolve the addresses logged in kh_com.log (crash / watchdog) to source lines.
# usage: port/tools/symbolize.sh [build/pc/kh_com.log] [build/pc/khcom.exe]
LOG=${1:-build/pc/kh_com.log}
EXE=${2:-build/pc/khcom.exe}
grep -oE "(at|eip=) ?[0-9a-f]{8}" "$LOG" | grep -oE "[0-9a-f]{8}$" | sed 's/^/0x/' |
    xargs llvm-addr2line -f -C -e "$EXE" | paste - - | sed 's|.*/khcom/||'
