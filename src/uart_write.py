import sys
import time
import serial
          
def initialize_serial():
    """
    Initialize UART communication.
    """
    return serial.Serial(
        port='/dev/ttyAMA0',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
    )

def send_message(serial_port, message, repeat_count):
    """
    Send the specified message through the serial port.
    """
    data = str.encode(message)
    
    if repeat_count == 0:
        for _ in range(10):
            serial_port.write(data)
            time.sleep(0.5)
    elif repeat_count == 1:
        while True:
            serial_port.write(data)
            time.sleep(0.5)

serial_port = initialize_serial()

if len(sys.argv) > 1:
    exit_value = int(sys.argv[1])
    if exit_value == 0:
        message = 'a5'  # Acknowledgment message
        print('--- Acknowledgment message \'a5\' will be sent to ATmega. ---')
        exit_nr = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    else:
        message = '96'  # Error message
        print('--- Error response \'96\' will be sent to ATmega. ---')
        exit_nr = 0
else:
    message = '96'  # Default error message
    print('--- Error response \'96\' will be sent to ATmega. ---')
    exit_nr = 0

#send_message(serial_port, message, exit_nr)

# Close the serial port after sending the message
serial_port.close()