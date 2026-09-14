#!/usr/bin/env python3
import struct, sys
p=sys.argv[1]
data=open(p,'rb').read(8192)
magic=struct.pack('<I',0x1BADB002)
pos=data.find(magic)
if pos<0:
    print('FAIL: multiboot magic not found in first 8192 bytes');sys.exit(1)
M,F,C=struct.unpack_from('<III',data,pos)
print(f'header offset: 0x{pos:x}, flags=0x{F:x}, checksum=0x{C:08x}')
if (M+F+C)&0xffffffff:
    print('FAIL: checksum');sys.exit(1)
print('PASS: Multiboot v1 header is valid')
