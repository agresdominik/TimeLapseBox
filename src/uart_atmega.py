import sys
import time
import serial
          
ser = serial.Serial(
    port='/dev/ttyAMA0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

while True:
    ser.write(b'~')
    time.sleep(0.5)

    ser.write(str.encode('!'))
    time.sleep(0.5)

    ser.write(str.encode('"'))
    time.sleep(0.5)

    ser.write(str.encode('#'))
    time.sleep(0.5)

    ser.write(str.encode('$'))
    time.sleep(0.5)

    ser.write(str.encode('%'))
    time.sleep(0.5)

    ser.write(str.encode('&'))
    time.sleep(0.5)

    ser.write(str.encode(')'))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)

    ser.write(str.encode('*'))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)

    ser.write(str.encode('+'))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)
    ser.write(bytes([0x00]))
    time.sleep(0.5)