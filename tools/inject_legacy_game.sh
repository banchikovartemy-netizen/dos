#!/usr/bin/env bash
set -euo pipefail
img=${1:-}
game=${2:-}
if [[ -z "$img" || -z "$game" ]]; then
  echo "Usage: $0 BOOTABLE_DOS_FAT_IMAGE GAME_FILE_OR_DIRECTORY" >&2
  exit 2
fi
command -v mcopy >/dev/null || { echo "mtools is required (Arch: sudo pacman -S mtools)" >&2; exit 3; }
[[ -f "$img" ]] || { echo "DOS image not found: $img" >&2; exit 4; }
[[ -e "$game" ]] || { echo "Game not found: $game" >&2; exit 5; }
# Keep user files separate from DOS system files. mmd failure is okay if GAMES exists.
mmd -i "$img" ::/GAMES 2>/dev/null || true
if [[ -d "$game" ]]; then
  base=$(basename "$game")
  mmd -i "$img" "::/GAMES/$base" 2>/dev/null || true
  mcopy -s -o -i "$img" "$game"/* "::/GAMES/$base/"
else
  mcopy -o -i "$img" "$game" ::/GAMES/
fi
echo "Injected into $img under C:\\GAMES"
