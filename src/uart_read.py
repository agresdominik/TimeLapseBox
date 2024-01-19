from time import sleep
import os
import serial
import numpy

ser = serial.Serial(
	port='/dev/ttyAMA0',
    baudrate = 9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

# Create time variables
second = 0
minute = 0
hour = 0

day = 0
month = 0
year = 0

# Create sensore lists
temperature_list = []
pressure_list = []
altitude_list = []

# Flag to indicate the start of a message block
start_flag = False
message = 0
print('--- UART Connection opened ---')
   
while start_flag:
    x = ser.read()	#.decode('utf-8')
    hex_data = ' '.join(hex(byte)[2:] for byte in x)
    print(hex_data)
    sleep(1)
	
while True:
    x = ser.read()
    hex_data = ' '.join(hex(byte)[2:] for byte in x)

    if start_flag:
        # Update variables
        if message == 0:
            second = hex_data
            print('--- second: {} ---'.format(hex_data))
        elif message == 1:
            minute = hex_data
            print('--- minute: {} ---'.format(hex_data))
        elif message == 2:
            hour = hex_data
            print('--- hour: {} ---'.format(hex_data))
        elif message == 3:
            day = hex_data
            print('--- day: {} ---'.format(hex_data))
        elif message == 4:
            month = hex_data
            print('--- month: {} ---'.format(hex_data))
        elif message == 5:
            year = hex_data
            print('--- year: {} ---'.format(hex_data))
        elif message <= 9:      # message = 6, 7, 8, 9
            temperature_list.append(hex_data)
            print('--- temperature_list: {} ---'.format(hex_data))
            if len(temperature_list) == 4:
                print("--- Temperature List:", temperature_list)
        elif message <= 13:     # message = 10, 11, 12, 13
            pressure_list.append(hex_data)
            print('--- pressure_list: {} ---'.format(hex_data))
            if len(pressure_list) == 4:
                print("Pressure List:", pressure_list)
        elif message == 17:     # message = 14, 15, 16, 17
            altitude_list.append(hex_data)
            print('--- altitude_list: {} ---'.format(hex_data))
            if len(altitude_list) == 4:
                print("Altitude List:", altitude_list)
        else: 
            print('Already received all values')
            print('Received data:')
            print('Day:', day)
            print('Month:', month)
            print('Year:', year)
            print('Time:', f'{second}:{minute}:{hour}')
            print('Temperature List:', temperature_list)
            print('Pressure List:', pressure_list)
            print('Altitude List:', altitude_list)
            start_flag = False  # Reset start_flag after processing a message block
            message = 0
            break  # End the script after processing the message block
        message = message + 1

    if numpy.bitwise_not(start_flag) and (hex_data != '0' and hex_data != ''):   
        print('--- A message which is not nothing is found ---')
        if hex_data == 'ff':
            start_flag = True
            print('--- Start of message block found: ff ---')

def setEnviron():
    os.environ["DATETIME"] = '{}.{}.20{} {}:{}:{}'.format(day, month, year, hour, minute, second)
    os.environ["TEMPERATURE"] = second
    os.environ["PRESSURE"] = second
    os.environ["ALTITUDE"] = second