import os
import time

BUFFER_FILE = 'buffer.dat'
SLOT_COUNT = 4
SLOT_SIZE = 1024
HEADER_SIZE = 4

def slot_offset(i):
    return HEADER_SIZE + i * SLOT_SIZE

def read_buffer():
    if not os.path.exists(BUFFER_FILE):
        print("Buffer file not found.")
        return

    with open(BUFFER_FILE, 'rb') as f:
        # Read status bytes
        f.seek(0)
        status_bytes = f.read(HEADER_SIZE)

        print(f"--- Buffer Status Inspector Tool :) ---")
        visual = ""
        for i in range(SLOT_COUNT):
            status = status_bytes[i]
            visual += "#" if status else "_"
            print(f"Slot {i}: {'FULL' if status == 1 else 'EMPTY'}")

            # Show preview
            f.seek(slot_offset(i))
            message = f.read(SLOT_SIZE).decode(errors='replace').strip('\x00')
            preview = message.split('\n')[0][:60]  # only first line, max 60 chars
            print(f"  → Preview: {preview if preview else '[EMPTY]'}")

        print(f"\nVisual: [{visual}]\n{'-'*50}\n")

if __name__ == "__main__":
    try:
        while True:
            os.system('clear')
            read_buffer()
            time.sleep(1)
    except KeyboardInterrupt:
        print("Stopped.")
