#!/bin/sh
# ReKHCoM for Linux. Runs with its own loader and libraries (lib/), so no
# 32-bit system packages are needed. Put your cartridge dump in roms/ (any
# file name): the region is read from the ROM header.
#   ./ReKHCoM.sh                 first ROM found in roms/
#   ./ReKHCoM.sh --region us     a specific version (eu, us or jp)
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REGION=
ROM=
if [ "$1" = "--region" ] && [ -n "$2" ]; then
    REGION=$2
    shift 2
fi

code_of() {
    dd if="$1" bs=1 skip=172 count=4 2>/dev/null
}

for f in "$DIR"/roms/*.gba "$DIR"/*.gba; do
    [ -f "$f" ] || continue
    case "$(code_of "$f")" in
        B8CP) r=eu ;;
        B8CE) r=us ;;
        B8CJ) r=jp ;;
        *) continue ;;
    esac
    if [ -z "$REGION" ] || [ "$REGION" = "$r" ]; then
        REGION=$r
        ROM=$f
        break
    fi
done

if [ -z "$ROM" ]; then
    msg="Kingdom Hearts: Chain of Memories ROM not found. Copy your cartridge dump (EU, US or JP, 32 MiB) into: $DIR/roms"
    echo "$msg" >&2
    command -v zenity >/dev/null 2>&1 && zenity --error --text="$msg" 2>/dev/null
    command -v kdialog >/dev/null 2>&1 && kdialog --error "$msg" 2>/dev/null
    exit 1
fi

# X11/XWayland and PulseAudio/PipeWire are reached through the bundled client
# libraries. The system's 64-bit GPU driver cannot be loaded into this 32-bit
# process, so SDL draws with its software renderer.
cd "$DIR" || exit 1
LOADER="$DIR/lib/rekhcom" # the bundled ld-linux, named after the game
exec "$LOADER" --library-path "$DIR/lib" "$DIR/bin/khcom_$REGION" \
    --data-dir "$DIR" --rom "$ROM" "$@"
