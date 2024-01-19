import time
import serial
import datetime
          
ser = serial.Serial(
              
    port='/dev/ttyAMA0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

while 1:
	message = "Ok"
	data = str.encode(message)
	ser.write(data)
	print('sended')
	time.sleep(1)

'''
Sekunde 
Minute 
Hour

temp1 
temp2 
temp3 
temp4 

altitude1
altitude2
altitude3
altitude4

pressure1
pressure2
pressure3
pressure4
'''