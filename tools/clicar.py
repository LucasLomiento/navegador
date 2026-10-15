#!/usr/bin/env python3
"""Clique de mouse de verdade pra testar a interface (aqui não tem ydotool).

Põe o cursor em x,y (coordenadas globais do Hyprland) e clica com um mouse
virtual criado no /dev/uinput (o usuário tem acesso por ACL). Só usar com a
janela de teste por baixo do cursor: o clique vai pra onde o cursor está.

  tools/clicar.py 700 330            # clique esquerdo
  tools/clicar.py 700 330 direito    # clique direito
"""

import fcntl
import os
import struct
import subprocess
import sys
import time

UI_SET_EVBIT = 0x40045564
UI_SET_KEYBIT = 0x40045565
UI_SET_RELBIT = 0x40045566
UI_DEV_CREATE = 0x5501
UI_DEV_DESTROY = 0x5502
EV_SYN, EV_KEY, EV_REL = 0, 1, 2
BTN_LEFT, BTN_RIGHT = 0x110, 0x111
REL_X, REL_Y = 0, 1


def evento(fd, tipo, codigo, valor):
    os.write(fd, struct.pack("llHHi", 0, 0, tipo, codigo, valor))


def main():
    x, y = int(sys.argv[1]), int(sys.argv[2])
    botao = BTN_RIGHT if len(sys.argv) > 3 and sys.argv[3] == "direito" \
        else BTN_LEFT

    fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
    try:
        fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
        fcntl.ioctl(fd, UI_SET_KEYBIT, BTN_LEFT)
        fcntl.ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT)
        fcntl.ioctl(fd, UI_SET_EVBIT, EV_REL)
        fcntl.ioctl(fd, UI_SET_RELBIT, REL_X)
        fcntl.ioctl(fd, UI_SET_RELBIT, REL_Y)
        nome = b"navegador-teste-mouse"
        dev = struct.pack("80sHHHHi", nome, 0x03, 0x1234, 0x5678, 1, 0)
        dev += b"\0" * (4 * 64 * 4)  # absmax, absmin, absfuzz, absflat
        os.write(fd, dev)
        fcntl.ioctl(fd, UI_DEV_CREATE)
        time.sleep(0.6)  # o compositor reconhecer o dispositivo

        subprocess.run(["hyprctl", "dispatch",
                        f"hl.dsp.cursor.move({{ x = {x}, y = {y} }})"],
                       check=True, capture_output=True)
        time.sleep(0.15)
        # Um movimento mínimo de ida e volta: o app recebe o hover antes.
        evento(fd, EV_REL, REL_X, 1)
        evento(fd, EV_SYN, 0, 0)
        evento(fd, EV_REL, REL_X, -1)
        evento(fd, EV_SYN, 0, 0)
        time.sleep(0.1)
        evento(fd, EV_KEY, botao, 1)
        evento(fd, EV_SYN, 0, 0)
        time.sleep(0.06)
        evento(fd, EV_KEY, botao, 0)
        evento(fd, EV_SYN, 0, 0)
        time.sleep(0.3)
        fcntl.ioctl(fd, UI_DEV_DESTROY)
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
