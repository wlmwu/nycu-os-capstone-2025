from serial import Serial
import struct
import argparse
import time
from tqdm import tqdm

def send_kernel_image(serial_port, kernel_path, baudrate):
    get_checksum = lambda data: sum(data) & 0xffffffff
    with Serial(serial_port, baudrate) as ser:
        try:
            with open(kernel_path, 'rb') as kernel_file:
                kernel_data = kernel_file.read()
                kernel_size = len(kernel_data)

                # Send the kernel size and checksum (4 bytes, little-endian unsigned int)
                ser.write(struct.pack('<II', 
                                      kernel_size,
                                      get_checksum(kernel_data)))
                ser.flush()
                print(f"Kernel image size: {kernel_size} bytes")

                time.sleep(1)      # Pause to let the board print messages

                chunk_size = 1024 # bytes
                with tqdm(total=kernel_size, unit='B', desc="Sending kernel") as pbar:
                    bytes_sent = 0
                    while bytes_sent < kernel_size:
                        chunk = kernel_data[bytes_sent : bytes_sent + chunk_size]
                        ser.write(chunk)
                        ser.flush()
                        bytes_sent += len(chunk)
                        pbar.update(len(chunk))

                print("Kernel image sent successfully.")
        
        except FileNotFoundError:
            print(f"Error: Kernel image '{kernel_path}' not found!")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send a kernel image over UART to the Raspberry Pi.")
    parser.add_argument("kernel", metavar="KERNEL_PATH", type=str, nargs='?', default="kernel8.img", 
                        help="The path to the kernel image file to be sent (default: kernel8.img).")
    parser.add_argument("-p", "--port", metavar="PORT", type=str, default="/dev/ttyUSB0", 
                        help="The serial port to use (default: /dev/ttyUSB0).")
    parser.add_argument("-b", "--baudrate", metavar="BAUDRATE", type=int, default=115200, 
                        help="The baud rate for the serial communication (default: 115200).")
    
    args = parser.parse_args()

    send_kernel_image(args.port, args.kernel, args.baudrate)
