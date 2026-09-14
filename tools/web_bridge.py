#!/usr/bin/env python3
"""PCOS Web Bridge.

A real Chromium instance runs on the host. PCOS stays tiny and exchanges small
UDP commands/ASCII frames with the bridge. Chromium therefore handles HTTPS,
HTML, CSS and JavaScript while PCOS renders the page in its native HUD style.

UDP 7777 commands:
  OPEN <url>
  CLICK <x> <y>       Chromium viewport coordinates
  TYPE <text>
  KEY <name>          ENTER, BACKSPACE, ESCAPE, LEFT, RIGHT, UP, DOWN
  SCROLL <pixels>
  REFRESH

Every command returns a 56x14 ASCII screenshot plus the current page title.
"""
import io
import socket
import sys

HOST = "0.0.0.0"
PORT = 7777
MAX_REPLY = 1100
VIEW_W = 896
VIEW_H = 448
ASCII_W = 56
ASCII_H = 14
RAMP = "@%#*+=-:. "

try:
    from playwright.sync_api import sync_playwright
    from PIL import Image
except Exception:
    print("PCOS Web Bridge requires Playwright and Pillow.")
    print("Install with:")
    print("  python -m pip install playwright pillow")
    print("  python -m playwright install chromium")
    raise SystemExit(2)


def normalize_url(url: str) -> str:
    url = url.strip()
    if not url:
        return "https://example.com"
    if "://" not in url:
        url = "https://" + url
    return url


def clean_ascii(text: str, limit: int = 52) -> str:
    text = " ".join(text.split())
    return "".join(ch if 32 <= ord(ch) < 127 else "?" for ch in text)[:limit]


def render(page) -> str:
    try:
        png = page.screenshot(type="png")
        image = Image.open(io.BytesIO(png)).convert("L").resize((ASCII_W, ASCII_H))
        px = image.load()
        lines = []
        for y in range(ASCII_H):
            line = []
            for x in range(ASCII_W):
                value = px[x, y]
                line.append(RAMP[(value * (len(RAMP) - 1)) // 255])
            lines.append("".join(line))
        title = clean_ascii(page.title() or page.url)
        return ("PAGE: " + title + "\n" + "\n".join(lines))[:MAX_REPLY]
    except Exception as exc:
        return f"BROWSER ERROR\n{type(exc).__name__}: {exc}"[:MAX_REPLY]


def execute(page, command: str) -> str:
    try:
        if command.startswith("OPEN "):
            url = normalize_url(command[5:])
            page.goto(url, wait_until="domcontentloaded", timeout=25000)
            try:
                page.wait_for_load_state("networkidle", timeout=4000)
            except Exception:
                pass
        elif command.startswith("CLICK "):
            _, sx, sy = command.split(maxsplit=2)
            x = max(0, min(VIEW_W - 1, int(sx)))
            y = max(0, min(VIEW_H - 1, int(sy)))
            page.mouse.click(x, y)
            page.wait_for_timeout(250)
        elif command.startswith("TYPE "):
            page.keyboard.type(command[5:], delay=15)
            page.wait_for_timeout(80)
        elif command.startswith("KEY "):
            key = command[4:].strip().upper()
            names = {
                "ENTER": "Enter", "BACKSPACE": "Backspace", "ESCAPE": "Escape",
                "LEFT": "ArrowLeft", "RIGHT": "ArrowRight",
                "UP": "ArrowUp", "DOWN": "ArrowDown",
            }
            if key in names:
                page.keyboard.press(names[key])
                page.wait_for_timeout(100)
        elif command.startswith("SCROLL "):
            amount = int(command[7:].strip())
            page.mouse.wheel(0, amount)
            page.wait_for_timeout(150)
        elif command == "REFRESH":
            page.reload(wait_until="domcontentloaded", timeout=25000)
        else:
            return "PCOS WEB BRIDGE\nUNKNOWN COMMAND"
        return render(page)
    except Exception as exc:
        return f"BROWSER ERROR\n{type(exc).__name__}: {exc}"[:MAX_REPLY]


def main() -> int:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    print(f"PCOS Web Bridge listening on UDP {HOST}:{PORT}")
    print("QEMU user networking exposes this host to PCOS as 10.0.2.2")
    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True)
        context = browser.new_context(viewport={"width": VIEW_W, "height": VIEW_H})
        page = context.new_page()
        page.goto("https://example.com", wait_until="domcontentloaded")
        while True:
            data, peer = sock.recvfrom(1400)
            command = data.decode("utf-8", "replace").strip()
            reply = execute(page, command)
            sock.sendto(reply.encode("ascii", "replace")[:MAX_REPLY], peer)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        sys.exit(0)
