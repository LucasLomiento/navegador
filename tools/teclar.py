#!/usr/bin/env python3
"""Atalho de teclado de verdade pra testar o navegador (teclado virtual no
/dev/uinput, como o tools/clicar.py faz com o mouse).

O wtype troca o mapa de teclado e o Chromium lê algumas teclas erradas (a
vírgula chega como Esc); aqui vão os códigos reais das teclas. A janela de
teste precisa estar focada (tools/testar.sh teclas foca, ou um clique nela).

  tools/teclar.py ctrl+comma
  tools/teclar.py ctrl+alt+right ctrl+shift+s
"""

import fcntl
import os
import struct
import sys
import time

UI_SET_EVBIT = 0x40045564
UI_SET_KEYBIT = 0x40045565
UI_DEV_CREATE = 0x5501
UI_DEV_DESTROY = 0x5502
EV_SYN, EV_KEY = 0, 1

# Códigos do linux/input-event-codes.h.
TECLAS = {
    "ctrl": 29, "shift": 42, "alt": 56, "super": 125,
    "esc": 1, "backspace": 14, "tab": 15, "enter": 28, "space": 57,
    "comma": 51, "period": 52, "slash": 53, "minus": 12, "equal": 13,
    "left": 105, "right": 106, "up": 103, "down": 108,
    "home": 102, "end": 107, "pageup": 104, "pagedown": 109, "delete": 111,
}
for i, letra in enumerate("qwertyuiop"):
    TECLAS[letra] = 16 + i
for i, letra in enumerate("asdfghjkl"):
    TECLAS[letra] = 30 + i
for i, letra in enumerate("zxcvbnm"):
    TECLAS[letra] = 44 + i
for i, digito in enumerate("1234567890"):
    TECLAS[digito] = 2 + i
for n in range(1, 11):
    TECLAS[f"f{n}"] = 58 + n
TECLAS["f11"], TECLAS["f12"] = 87, 88


def evento(fd, tipo, codigo, valor):
    os.write(fd, struct.pack("llHHi", 0, 0, tipo, codigo, valor))


def main():
    combinacoes = [[TECLAS[t] for t in arg.lower().split("+")]
                   for arg in sys.argv[1:]]
    fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
    try:
        fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
        for codigo in set(TECLAS.values()):
            fcntl.ioctl(fd, UI_SET_KEYBIT, codigo)
        nome = b"navegador-teste-teclado"
        dev = struct.pack("80sHHHHi", nome, 0x03, 0x1234, 0x5679, 1, 0)
        dev += b"\0" * (4 * 64 * 4)
        os.write(fd, dev)
        fcntl.ioctl(fd, UI_DEV_CREATE)
        time.sleep(0.6)  # o compositor reconhecer o dispositivo
        for teclas in combinacoes:
            for codigo in teclas:
                evento(fd, EV_KEY, codigo, 1)
                evento(fd, EV_SYN, 0, 0)
                time.sleep(0.03)
            for codigo in reversed(teclas):
                evento(fd, EV_KEY, codigo, 0)
                evento(fd, EV_SYN, 0, 0)
                time.sleep(0.03)
            time.sleep(0.2)
        time.sleep(0.2)
        fcntl.ioctl(fd, UI_DEV_DESTROY)
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
