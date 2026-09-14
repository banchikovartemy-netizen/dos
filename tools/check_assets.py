#!/usr/bin/env python3
from pathlib import Path
import struct, tarfile, wave, io
ROOT=Path(__file__).resolve().parents[1]
tar_path=ROOT/'build'/'pcfs.tar'
required={
 'pc/files/notes.txt','pc/games/hello.com','pc/games/vga13.com','pc/games/hello.exe','pc/photos/target.bmp','pc/photos/grid.bmp','pc/photos/optic.bmp',
 'pc/music/future.wav','pc/music/machine.wav','pc/music/night.wav','pc/video/scan.avi','pc/system/version.txt'
}
with tarfile.open(tar_path,'r') as tf:
    names=set(tf.getnames())
    miss=required-names
    if miss: raise SystemExit(f'FAIL missing assets: {sorted(miss)}')
    for name in ['pc/photos/target.bmp','pc/photos/grid.bmp','pc/photos/optic.bmp']:
        b=tf.extractfile(name).read()
        if b[:2]!=b'BM' or len(b)<54: raise SystemExit(f'FAIL BMP {name}')
        w,h=struct.unpack_from('<ii',b,18)
        if w<=0 or h<=0: raise SystemExit(f'FAIL BMP dimensions {name}')
    for name in ['pc/music/future.wav','pc/music/machine.wav','pc/music/night.wav']:
        b=tf.extractfile(name).read()
        with wave.open(io.BytesIO(b),'rb') as w:
            if w.getnchannels()!=1 or w.getsampwidth()!=1: raise SystemExit(f'FAIL WAV format {name}')
    b=tf.extractfile('pc/video/scan.avi').read()
    if b[:4]!=b'RIFF' or b[8:12]!=b'AVI ': raise SystemExit('FAIL AVI')
    if not tf.extractfile('pc/games/hello.com').read(): raise SystemExit('FAIL DOS COM')
    if tf.extractfile('pc/games/hello.exe').read()[:2]!=b'MZ': raise SystemExit('FAIL DOS MZ')
    if not tf.extractfile('pc/games/vga13.com').read(): raise SystemExit('FAIL DOS VGA13')
img=(ROOT/'data'/'pcos-data.img').read_bytes()
if len(img)<1024 or img[:8]!=b'PCOSDATA' or img[512:519]!=b'PCOSOV2': raise SystemExit('FAIL PCOS data image')
print('PASS: pcfs media, DOS COM/MZ/VGA13 samples, and PCOSDATA image are valid')
