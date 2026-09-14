#!/usr/bin/env python3
"""PCOS Web Bridge.

Runs a real Chromium browser on the host and exposes a tiny UDP text protocol to
PCOS. This keeps the bare-metal guest small while allowing modern TLS/HTML/CSS/
JavaScript sites to load through Chromium.

Protocol (UDP 7777):
    OPEN <url>
Response is a UTF-8 text snapshot (title + visible body text), capped to one UDP
packet so the current PCOS network stack stays tiny.
"""
import socket
import sys

HOST = "0.0.0.0"
PORT = 7777
MAX_REPLY = 1100

try:
    from playwright.sync_api import sync_playwright
except Exception:
    print("Playwright is required.")
    print("Install with:")
    print("  python -m pip install playwright")
    print("  python -m playwright install chromium")
    raise SystemExit(2)


def normalize_url(url: str) -> str:
    url = url.strip()
    if not url:
        return "https://example.com"
    if "://" not in url:
        url = "https://" + url
    return url


def snapshot(page, url: str) -> str:
    url = normalize_url(url)
    try:
        page.goto(url, wait_until="domcontentloaded", timeout=20000)
        try:
            page.wait_for_load_state("networkidle", timeout=5000)
        except Exception:
            pass
        title = page.title().strip() or url
        body = page.locator("body").inner_text(timeout=5000)
        body = "\n".join(line.strip() for line in body.splitlines() if line.strip())
        text = f"TITLE: {title}\nURL: {page.url}\n\n{body}"
        return text[:MAX_REPLY]
    except Exception as exc:
        return f"BROWSER ERROR\n{type(exc).__name__}: {exc}"[:MAX_REPLY]


def main() -> int:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    print(f"PCOS Web Bridge listening on UDP {HOST}:{PORT}")
    print("QEMU user-mode networking reaches the host as 10.0.2.2")
    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True)
        context = browser.new_context(viewport={"width": 1280, "height": 720})
        page = context.new_page()
        while True:
            data, peer = sock.recvfrom(1400)
            command = data.decode("utf-8", "replace").strip()
            if command.startswith("OPEN "):
                reply = snapshot(page, command[5:])
            else:
                reply = "PCOS WEB BRIDGE\nSUPPORTED: OPEN <url>"
            sock.sendto(reply.encode("utf-8", "replace")[:MAX_REPLY], peer)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        sys.exit(0)
