#!/usr/bin/env bash
set -euo pipefail

img=${1:-}
doomdir=${2:-}
if [[ -z "$img" || -z "$doomdir" ]]; then
  echo "Usage: $0 BOOTABLE_DOS_FAT_IMAGE DOOM_DIRECTORY" >&2
  exit 2
fi
command -v mcopy >/dev/null || { echo "mtools is required (Arch: sudo pacman -S mtools)" >&2; exit 3; }
command -v mmd >/dev/null || { echo "mtools is required (Arch: sudo pacman -S mtools)" >&2; exit 3; }
[[ -f "$img" ]] || { echo "DOS image not found: $img" >&2; exit 4; }
[[ -d "$doomdir" ]] || { echo "DOOM directory not found: $doomdir" >&2; exit 5; }

exe=""
for f in "$doomdir"/DOOM.EXE "$doomdir"/doom.exe; do
  [[ -f "$f" ]] && exe="$f" && break
done
[[ -n "$exe" ]] || { echo "DOOM.EXE was not found in $doomdir" >&2; exit 6; }

wad=""
for f in "$doomdir"/DOOM1.WAD "$doomdir"/doom1.wad "$doomdir"/DOOM.WAD "$doomdir"/doom.wad; do
  [[ -f "$f" ]] && wad="$f" && break
done
[[ -n "$wad" ]] || { echo "DOOM1.WAD/DOOM.WAD was not found in $doomdir" >&2; exit 7; }

# A dedicated C:\DOOM folder makes the PCOS launcher deterministic.
mmd -i "$img" ::/DOOM 2>/dev/null || true
mcopy -s -o -i "$img" "$doomdir"/* ::/DOOM/

# Preserve the image's existing startup commands and append one PCOS block.
tmp=$(mktemp)
trap 'rm -f "$tmp" "$tmp.new"' EXIT
mcopy -i "$img" ::/AUTOEXEC.BAT "$tmp" 2>/dev/null || : > "$tmp"
# Remove an older PCOS Doom block when the installer is run again.
awk 'BEGIN{skip=0} /REM PCOS_DOOM_BEGIN/{skip=1;next} /REM PCOS_DOOM_END/{skip=0;next} !skip{print}' "$tmp" > "$tmp.new" || cp "$tmp" "$tmp.new"
{
  cat "$tmp.new"
  printf '\r\nREM PCOS_DOOM_BEGIN\r\n'
  printf 'C:\r\n'
  printf 'CD \\DOOM\r\n'
  printf 'DOOM.EXE\r\n'
  printf 'REM PCOS_DOOM_END\r\n'
} > "$tmp"
mcopy -o -i "$img" "$tmp" ::/AUTOEXEC.BAT

echo "DOOM installed into C:\\DOOM on $img"
echo "PCOS GAMES -> DOOM 1 will chainload this disk and AUTOEXEC.BAT will start DOOM.EXE."
