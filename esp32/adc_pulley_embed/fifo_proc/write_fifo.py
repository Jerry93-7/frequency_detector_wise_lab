import serial
import os
import sys
import time

# Configuration
SERIAL_PORT = '/dev/ttyUSB1'
BAUD_RATE = 115200
FIFO_PATH = '/tmp/uart_data'
MAGIC_START_SEQUENCE = b"RR UART OK\r\n"

# Ensure FIFO exists
if not os.path.exists(FIFO_PATH):
    os.mkfifo(FIFO_PATH)

with serial.Serial(SERIAL_PORT, BAUD_RATE) as ser:
    print("Waiting for magic sequence...")
    buf = b''
    remainder = b''
    while True:
        data = ser.read(ser.in_waiting or 1)
        if data:
            buf += data
            if MAGIC_START_SEQUENCE in buf:
                # Keep everything after the magic sequence
                remainder = buf.split(MAGIC_START_SEQUENCE, 1)[1]
                print("Found start sequence! Forwarding to FIFO...")
                break

    print("About to open FIFO", flush=True)
    with open(FIFO_PATH, 'wb') as fifo, open('trace_debug.bin', 'wb') as debug:
        start_time = time.time()
        total_bytes = 0
        if remainder:
            fifo.write(remainder)
            fifo.flush()
            # debug.write(remainder)
        while True:
            data = ser.read(ser.in_waiting or 1)

            if data:
                total_bytes += len(data)
                fifo.write(data)
                fifo.flush()
                # debug.write(data)
                # debug.flush()
            elapsed = time.time() - start_time
            # if elapsed > 0:
            rate = total_bytes / elapsed  # bytes per second
            print(f"{total_bytes} bytes read, {rate:.2f} B/s", flush=True)
