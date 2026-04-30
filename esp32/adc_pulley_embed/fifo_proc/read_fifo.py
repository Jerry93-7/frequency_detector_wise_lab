import os
import time

FIFO_PATH = '/tmp/uart_data'

def read_from_fifo():
    # 1. Ensure the FIFO exists so we don't crash on startup
    if not os.path.exists(FIFO_PATH):
        print(f"Waiting for FIFO {FIFO_PATH} to be created...")
        while not os.path.exists(FIFO_PATH):
            time.sleep(1)

    print(f"Listening on {FIFO_PATH}...")

    while True:
        # 2. Open the FIFO. 
        # Note: This will block (pause) here until the UART script opens it for writing.
        with open(FIFO_PATH, 'rb') as fifo:
            print("--- Writer Connected ---")
            
            while True:
                # 3. Read data in chunks (e.g., 1024 bytes)
                data = fifo.read(1024)
                
                # 4. If read returns 0 bytes, the writer has closed the pipe
                if len(data) == 0:
                    print("--- Writer Disconnected (End of Stream) ---")
                    break
                
                # 5. Process your bytes here
                # Example: Print as hex to see the raw UART data
                # print(f"Received {len(data)} bytes: {data.hex(' ')}")
                print(f"{data}")

if __name__ == "__main__":
    try:
        read_from_fifo()
    except KeyboardInterrupt:
        print("\nReader stopped by user.")
