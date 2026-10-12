#!/bin/sh
# Run the PC build in an isolated directory (build/pctest) so automated tests
# never touch the player's settings or save in build/pc.
# usage: port/tools/testrun.sh [env assignments...]   (run from the repo root)
set -e
mkdir -p build/pctest
cp -f build/pc/khcom.exe build/pc/SDL3.dll build/pctest/
cd build/pctest
rm -f kh_com.log shot_* out_* kh_com.sav
exec env "$@" ./khcom.exe
