#!/usr/bin/env python3
from pathlib import Path
import re, subprocess, sys, tempfile
obj=Path(sys.argv[1] if len(sys.argv)>1 else 'build/boot/legacy.o')
if not obj.exists():
    raise SystemExit(f'FAIL: missing {obj}')
nm=subprocess.check_output(['nm','-n',str(obj)],text=True)
syms={}
for line in nm.splitlines():
    m=re.match(r'^([0-9a-fA-F]+)\s+\w\s+(\S+)$',line)
    if m: syms[m.group(2)]=int(m.group(1),16)
for k in ('legacy_trampoline_start','legacy_trampoline_end','legacy_rm','legacy_msg'):
    if k not in syms: raise SystemExit(f'FAIL: missing symbol {k}')
start,end=syms['legacy_trampoline_start'],syms['legacy_trampoline_end']
if not (0 < end-start <= 512): raise SystemExit(f'FAIL: trampoline size {end-start}')
rel=subprocess.check_output(['objdump','-r',str(obj)],text=True)
for line in rel.splitlines():
    m=re.match(r'^([0-9a-fA-F]{8})\s+R_386_',line.strip())
    if m and start <= int(m.group(1),16) < end:
        raise SystemExit(f'FAIL: relocation inside copied trampoline at {m.group(1)}')
with tempfile.NamedTemporaryFile() as f:
    subprocess.check_call(['objcopy','-O','binary','--only-section=.text',str(obj),f.name],stdout=subprocess.DEVNULL)
    raw=Path(f.name).read_bytes()[start:end]
checks={
 'clear CR0.PE': b'\x0f\x22\xc0',
 'BIOS disk INT13': b'\xcd\x13',
 'BIOS video INT10': b'\xcd\x10',
 'boot jump 0000:7C00': b'\xea\x00\x7c\x00\x00',
 'boot signature test': b'\x55\xaa',
}
for name,pat in checks.items():
    if pat not in raw: raise SystemExit(f'FAIL: {name} pattern absent')
print(f'PASS: Legacy DOS trampoline is PIC, relocation-free, {len(raw)} bytes, BIOS chainloader patterns present')
