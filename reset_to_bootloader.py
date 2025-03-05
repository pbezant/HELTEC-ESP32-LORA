#!/usr/bin/env python3
import serial
import time
import sys
import glob

def get_serial_ports():
    """ Lists serial ports """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/cu.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def reset_to_bootloader(port):
    """Reset ESP32-S3 into bootloader mode using DTR/RTS control"""
    print(f"Attempting to reset device on {port} into bootloader mode...")
    
    try:
        ser = serial.Serial(port, 115200)
        ser.dtr = False
        ser.rts = False
        time.sleep(0.1)
        
        # Toggle DTR and RTS to put ESP32-S3 into bootloader
        ser.dtr = True
        ser.rts = True
        time.sleep(0.1)
        ser.dtr = False
        time.sleep(0.1)
        
        # Release RTS
        ser.rts = False
        time.sleep(0.5)
        
        ser.close()
        print("Device should now be in bootloader mode")
        return True
    except Exception as e:
        print(f"Error resetting device: {e}")
        return False

if __name__ == "__main__":
    ports = get_serial_ports()
    
    if not ports:
        print("No serial ports found")
        sys.exit(1)
    
    print("Available ports:")
    for i, port in enumerate(ports):
        print(f"{i+1}. {port}")
    
    if len(ports) == 1:
        selected_port = ports[0]
        print(f"Using the only available port: {selected_port}")
    else:
        try:
            choice = int(input("Select port number: "))
            selected_port = ports[choice-1]
        except (ValueError, IndexError):
            print("Invalid selection")
            sys.exit(1)
    
    success = reset_to_bootloader(selected_port)
    if success:
        print("Now run the upload command immediately")
    else:
        print("Failed to reset device")
        sys.exit(1) 