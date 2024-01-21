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

if len(sys.argv) > 1:
    exit_value = int(sys.arg[1])
    if exit_value == 0:
        message = 'a5'  #96
        print('--- From now on an ack \'a5\' will be sent to ATMEGA. ---')
    else:
        message = '96'
        print('--- From now on an error response \'96\' will be sent to ATMEAG. ---')
else:
    message = '96'
    print('--- From now on an error response \'96\' will be sent to ATMEAG. ---')

data = str.encode(message)
for x in range(0, 10):
	ser.write(data)
	time.sleep(0.5)