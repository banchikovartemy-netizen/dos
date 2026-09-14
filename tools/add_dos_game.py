#!/usr/bin/env python3
"""Import user-owned DOS game files into PCOS pc/games and rebuild with `make assets`.
Usage: python3 tools/add_dos_game.py /path/to/GAME.EXE
       python3 tools/add_dos_game.py /path/to/game_directory
Directory contents are copied into pc/games, preserving subdirectories and normalizing names to lowercase.
"""
from pathlib import Path
import shutil, sys
ROOT=Path(__file__).resolve().parents[1]
DST=ROOT/'assets'/'pc'/'games'
if len(sys.argv)!=2:
    raise SystemExit('usage: add_dos_game.py FILE_OR_DIRECTORY')
src=Path(sys.argv[1]).expanduser().resolve()
if not src.exists(): raise SystemExit(f'not found: {src}')
DST.mkdir(parents=True,exist_ok=True)
def cp_file(s:Path,d:Path):
    d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(s,d);print(f'+ {d.relative_to(ROOT)}')
if src.is_file():
    cp_file(src,DST/src.name.lower())
else:
    for s in src.rglob('*'):
        if s.is_file():
            rel=Path(*[x.lower() for x in s.relative_to(src).parts])
            cp_file(s,DST/rel)
print('Run: make assets   (or make iso)')
