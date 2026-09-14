#!/usr/bin/env python3
from pathlib import Path
import math, struct, tarfile, wave
ROOT=Path(__file__).resolve().parents[1]
A=ROOT/'assets'/'pc'
for d in ['apps','files','games','music','photos','video','system','config','temp']:
    (A/d).mkdir(parents=True,exist_ok=True)
(A/'files'/'readme.txt').write_text('PCOS 0.6\nРЕТРО-ФУТУРИСТИЧЕСКАЯ ЛЕГКАЯ ОПЕРАЦИОННАЯ СИСТЕМА\n\nСтрелки вверх/вниз переключают приложения. F1..F11 - быстрый выбор.\nТерминал: команда help показывает справку.\n',encoding='utf-8')
(A/'files'/'notes.txt').write_text('PCOS // ЗАМЕТКИ\n\nАВТОСОХРАНЕНИЕ ВКЛЮЧЕНО. ПРИ ПОДКЛЮЧЕННОМ ДИСКЕ PCOSDATA ЗАМЕТКИ СОХРАНЯЮТСЯ ПОСЛЕ ПЕРЕЗАГРУЗКИ.\n',encoding='utf-8')
(A/'config'/'display.cfg').write_text('РЕЖИМ=АВТО\nТЕМА=КРАСНЫЙ_ТЕРМИНАЛ\nОТРИСОВКА=ASCII_HUD\n',encoding='utf-8')
(A/'config'/'network.cfg').write_text('РЕЖИМ=DHCP\nДРАЙВЕР=RTL8139\n',encoding='utf-8')
app_ru={
    'terminal':'ТЕРМИНАЛ','files':'ФАЙЛЫ','notes':'ЗАМЕТКИ','player':'ПЛЕЕР','photos':'ФОТО',
    'video':'ВИДЕО','calculator':'КАЛЬКУЛЯТОР','games':'ИГРЫ','network':'СЕТЬ','system':'СИСТЕМА','settings':'НАСТРОЙКИ'
}
for n,ru in app_ru.items():
    (A/'apps'/f'{n}.app').write_text(f'ОПИСАТЕЛЬ ПРИЛОЖЕНИЯ PCOS\nИМЯ={ru}\n',encoding='utf-8')
# BMP 160x100 24-bit, геометрическая HUD-графика.
w,h=160,100
row=(w*3+3)&~3
pix=bytearray(row*h)
for y in range(h):
    for x in range(w):
        dx=x-w/2;dy=y-h/2
        ring=abs(math.hypot(dx,dy)-30)<1.5 or abs(math.hypot(dx,dy)-42)<1.0
        cross=abs(dx)<1 or abs(dy)<1
        grid=(x%20==0 or y%20==0)
        scan=(y%6==0)
        if ring or cross: r,g,b=255,45,30
        elif grid: r,g,b=80,8,8
        elif scan: r,g,b=35,0,0
        else: r,g,b=8,0,0
        o=y*row+x*3;pix[o:o+3]=bytes((b,g,r))
filesz=54+len(pix)
hdr=b'BM'+struct.pack('<IHHI',filesz,0,0,54)+struct.pack('<IiiHHIIiiII',40,w,h,1,24,0,len(pix),2835,2835,0,0)
(A/'photos'/'target.bmp').write_bytes(hdr+pix)
def write_hud_bmp(path, kind):
    pix2=bytearray(row*h)
    for y in range(h):
        for x in range(w):
            dx=x-w/2;dy=y-h/2
            if kind==1:
                hot=(x%16==0 or y%16==0 or abs(dx-dy)<1 or abs(dx+dy)<1)
                r,g,b=(230,35,25) if hot else ((50,3,3) if y%5==0 else (5,0,0))
            else:
                rr=math.hypot(dx,dy);hot=(abs(rr-18)<1.3 or abs(rr-34)<1.3 or (abs(dy)<2 and abs(dx)<50))
                r,g,b=(255,55,35) if hot else ((65,5,5) if x%13==0 else (7,0,0))
            o=y*row+x*3;pix2[o:o+3]=bytes((b,g,r))
    path.write_bytes(hdr+pix2)
write_hud_bmp(A/'photos'/'grid.bmp',1)
write_hud_bmp(A/'photos'/'optic.bmp',2)
# WAV: 8-bit mono 11025 Hz для слабого железа.
for name,freq in [('future.wav',110),('machine.wav',165),('night.wav',220)]:
    path=A/'music'/name
    rate=11025;seconds=2.5
    with wave.open(str(path),'wb') as wf:
        wf.setnchannels(1);wf.setsampwidth(1);wf.setframerate(rate)
        out=bytearray()
        for i in range(int(rate*seconds)):
            t=i/rate
            v=128+int(42*math.sin(2*math.pi*freq*t))+int(18*(1 if math.sin(2*math.pi*(freq/2)*t)>=0 else -1))
            out.append(max(0,min(255,v)))
        wf.writeframes(out)
# Минимальный несжатый AVI, 80x60, 12 кадров, 6 FPS.
def chunk(tag,data):
    pad=b'\0' if len(data)&1 else b''
    return tag+struct.pack('<I',len(data))+data+pad
def list_chunk(kind,data):
    return chunk(b'LIST',kind+data)
vw,vh,fps,nf=80,60,6,12
vrow=(vw*3+3)&~3
frames=[]
for f in range(nf):
    buf=bytearray(vrow*vh)
    tx=8+f*5
    ty=30+int(12*math.sin(f/2))
    for y in range(vh):
        for x in range(vw):
            grid=(x%10==0 or y%10==0)
            target=((x-tx)**2+(y-ty)**2)<36
            scan=(y+f)%8==0
            if target:r,g,b=255,50,35
            elif grid:r,g,b=80,8,8
            elif scan:r,g,b=35,0,0
            else:r,g,b=5,0,0
            sy=vh-1-y;o=sy*vrow+x*3;buf[o:o+3]=bytes((b,g,r))
    frames.append(bytes(buf))
maxbps=vrow*vh*fps
avih=struct.pack('<IIIIIIIIIIIIII',int(1_000_000/fps),maxbps,0,0x10,nf,0,1,vrow*vh,vw,vh,0,0,0,0)
strh=struct.pack('<4s4sIHHIIIIIIIIhhhh',b'vids',b'DIB ',0,0,0,0,1,fps,0,nf,vrow*vh,0xFFFFFFFF,0,0,0,vw,vh)
strf=struct.pack('<IiiHHIIiiII',40,vw,vh,1,24,0,vrow*vh,0,0,0,0)
hdrl=list_chunk(b'hdrl',chunk(b'avih',avih)+list_chunk(b'strl',chunk(b'strh',strh)+chunk(b'strf',strf)))
movi=list_chunk(b'movi',b''.join(chunk(b'00db',fr) for fr in frames))
body=b'AVI '+hdrl+movi
(A/'video'/'scan.avi').write_bytes(b'RIFF'+struct.pack('<I',len(body))+body)
# Тестовая DOS COM программа оставлена ASCII: гостевая DOS-среда использует старую кодировку.
msg=b'PCOS DOS86 RUNTIME ONLINE!$'
code=bytearray([0xBA,0x0C,0x01,0xB4,0x09,0xCD,0x21,0xB8,0x00,0x4C,0xCD,0x21])+msg
(A/'games'/'hello.com').write_bytes(code)
# VGA mode 13h COM demo.
vga13=bytearray([
 0xB8,0x13,0x00,0xCD,0x10,
 0xB8,0x00,0xA0,0x8E,0xC0,
 0x31,0xFF,
 0xB9,0x00,0x7D,
 0xB8,0x04,0x0C,
 0xF3,0xAB,
 0xB4,0x00,0xCD,0x16,
 0xB8,0x03,0x00,0xCD,0x10,
 0xB8,0x00,0x4C,0xCD,0x21])
(A/'games'/'vga13.com').write_bytes(vga13)
# Минимальный MZ EXE для проверки загрузчика EXE.
exe_code=bytearray([0x8C,0xC8,0x8E,0xD8,0xBA,0x10,0x00,0xB4,0x09,0xCD,0x21,0xB8,0x00,0x4C,0xCD,0x21])+b'PCOS MZ EXE LOADER ONLINE!$'
hdr=bytearray(32);hdr[0:2]=b'MZ';total=32+len(exe_code);pages=(total+511)//512;last=total%512
struct.pack_into('<H',hdr,2,last);struct.pack_into('<H',hdr,4,pages);struct.pack_into('<H',hdr,6,0);struct.pack_into('<H',hdr,8,2)
struct.pack_into('<H',hdr,14,0);struct.pack_into('<H',hdr,16,0xFFFE);struct.pack_into('<H',hdr,20,0);struct.pack_into('<H',hdr,22,0);struct.pack_into('<H',hdr,24,0x1C)
(A/'games'/'hello.exe').write_bytes(hdr+exe_code)
(A/'games'/'README.TXT').write_text('PCOS ИГРЫ 0.6\n\nDOS86 В ОКНЕ: COM + MZ EXE, 8086/80186, BIOS-клавиатура/таймер, DOS API файлов, VGA13/ModeX и мост SB16.\nLEGACY DOS: нажмите L в приложении ИГРЫ или выполните legacydos, чтобы загрузить второй BIOS-диск для нативных игр 386/DOS4GW. Для возврата в PCOS нужна перезагрузка.\n',encoding='utf-8')
(A/'system'/'version.txt').write_text('PCOS 0.6\nЯДРО=i386\nИНТЕРФЕЙС=ASCII_HUD_RU\nDOS86=REALMODE_COM_MZ_VGA13_MODEX_SB16\nLEGACY_DOS=BIOS_HDD2_NATIVE\n',encoding='utf-8')
# USTAR initrd с корнем pc/.
out=ROOT/'build'/'pcfs.tar';out.parent.mkdir(parents=True,exist_ok=True)
with tarfile.open(out,'w',format=tarfile.USTAR_FORMAT) as tf:
    tf.add(A,arcname='pc',recursive=True)
print(out)

# Безопасный шаблон записываемого диска PCOSDATA.
data_dir=ROOT/'data';data_dir.mkdir(parents=True,exist_ok=True)
data_img=data_dir/'pcos-data.img'
if not data_img.exists():
    total=2*1024*1024
    raw=bytearray(total)
    raw[0:8]=b'PCOSDATA'
    raw[8:12]=struct.pack('<I',2)
    h=512
    raw[h:h+8]=b'PCOSOV2\0'
    raw[h+8:h+12]=struct.pack('<I',20)
    raw[h+12:h+16]=struct.pack('<I',9)
    data_img.write_bytes(raw)
print(data_img)
