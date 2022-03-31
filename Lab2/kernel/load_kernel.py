import sys
import time

with open('/dev/ttyUSB0', 'rb+', buffering=0) as tty:
    with open('kernel8.img', 'rb') as kernel_img:
        kernel_code = kernel_img.read()
        sptl = "START"
        tty.write(sptl.encode())
        print(len(kernel_code))
        tty.write(len(kernel_code).to_bytes(4,'little'))
        time.sleep(0.1)
        tty.write(kernel_code)
        time.sleep(0.1)

