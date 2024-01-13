#!/bin/python3
import serial
import sys
from time import sleep

def print_everything():
	while True:
		line = ser.readline()
		if line != b'' and line != b'\x1b[0m\r\n':
			print(line)

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'
with serial.Serial(port, 115200, timeout=3) as ser:
	ser.dtr = False # Drop DTR
	sleep(0.022)    # Read somewhere that 22ms is what the UI does.
	ser.dtr = True  # UP the DTR back
	ser.write(b"reset\n")

	print_everything()
