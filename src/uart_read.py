import subprocess
import sys
import serial
from time import sleep

class ReadUartPi:
    """
    A class for initializing and managing a UART connection to receive sensor data.

    Attributes:
        - ser: Serial object for UART communication with specified parameters.
        - second, minute, hour: Time variables for timestamping sensor data.
        - day, month, year: Date variables for timestamping sensor data.
        - temperature_list, pressure_list, altitude_list: Lists to store sensor data.
        - start_flag (boolean): Flag to indicate the start of a message block.
        - messageNr (int): Variable to keep track of the received message number.

    Methods:
        - readLoop():
        - processLoop():
        - print_all(): Prints all class attributes.
        - write_tmp(): Writes the class attributes in a newly generated tempfile.
    """

    def __init__(self):
        """
        Initializes the ReadUartPi class.
        """
        self.ser = serial.Serial(
            port='/dev/ttyAMA0',
            baudrate = 9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )

        # Create time variables
        self.second = 0
        self.minute = 0
        self.hour = 0

        self.day = 0
        self.month = 0
        self.year = 0

        # Create sensore lists
        self.temperature_list = []
        self.pressure_list = []
        self.altitude_list = []

        # Flag to indicate the start of a message block
        self.start_flag = False
        self.messageNr = -1
        print('--- UART Connection opened ---')
   
    def readLoop(self):
        # https://tools.piex.at/ascii-tabelle/
        while True:
            x = self.ser.read()	#.decode('utf-8')
            print(x)
            hex_data = ' '.join(hex(byte)[2:] for byte in x)
            #print(hex_data)
            sleep(0.5)
        
    def processLoop(self):
        while True:
            x = self.ser.read() #b'('
            hex_data = ' '.join(hex(byte)[2:] for byte in x)

            if self.start_flag:
                    # Update variables
                    if self.messageNr == 0:
                        self.second = hex_data
                        print('--- Second: {} ---'.format(hex_data))
                    elif self.messageNr == 1:
                        self.minute = hex_data
                        print('--- Minute: {} ---'.format(hex_data))
                    elif self.messageNr == 2:
                        self.hour = hex_data
                        print('--- Hour: {} ---'.format(hex_data))
                    elif self.messageNr == 3:
                        self.day = hex_data
                        print('--- Day: {} ---'.format(hex_data))
                    elif self.messageNr == 4:
                        self.month = hex_data
                        print('--- Month: {} ---'.format(hex_data))
                    elif self.messageNr == 5:
                        self.year = hex_data
                        print('--- Year: {} ---'.format(hex_data))
                    elif self.messageNr <= 9:      # messageNr = 6, 7, 8, 9
                        self.temperature_list.append(hex_data)
                        print("--- Temperature List: ", self.temperature_list)
                    elif self.messageNr <= 13:     # messageNr = 10, 11, 12, 13
                        self.pressure_list.append(hex_data)
                        print("--- Pressure List: ", self.pressure_list)
                    elif self.messageNr <= 17:     # messageNr = 14, 15, 16, 17
                        self.altitude_list.append(hex_data)
                        print("--- Altitude List: ", self.altitude_list)
                        if self.messageNr == 17:
                            print('--- All values recieved (18) ---')
                            self.write_ack(0)
                            self.write_tmp()
                            break  # End the script after processing the message block
                    self.messageNr = self.messageNr + 1

            if not hex_data == '0' and not hex_data == '':
                if not (self.start_flag):   
                    print(f'--- A message which is not nothing is found: {hex_data} ---')
                    if hex_data == 'ff':
                        self.start_flag = True
                        self.messageNr = 0
                        print(f'--- Start of message block found: {hex_data} ---')
        sleep(0.5)

    def print_all(self):
        print('################################################################')
        print('Received data:')
        print(f'Day: {self.day}-{self.month}-{self.year}')
        print(f'Time: {self.second}:{self.minute}:{self.hour}')
        print('Temperature List: ', self.temperature_list)
        print('Pressure List: ', self.pressure_list)
        print('Altitude List: ', self.altitude_list)
        print('################################################################')

    def write_tmp(self):
        if len(sys.argv) > 1:
            name_str = f'{int(self.day, 16)}-{int(self.month, 16)}-20{int(self.year, 16)}_{int(self.hour, 16)}-{int(self.minute, 16)}'
            datetime_str = f'{int(self.day, 16)}.{int(self.month, 16)}.20{int(self.year, 16)} {int(self.hour, 16)}:{int(self.minute, 16)}:{int(self.second, 16)}'
            temperature_str = str( int( "".join(filter(lambda x: x != '0', self.temperature_list)), 16 ) )
            pressure_str = str( int( "".join(filter(lambda x: x != '0', self.pressure_list)), 16 ) )
            altitude_str = str( int( "".join(filter(lambda x: x != '0', self.altitude_list)), 16 ) )

            with open(sys.argv[1], 'r+') as file:
                file.seek(0)
                file.write(f'NAME={name_str}\n')
                file.write(f"DATETIME={datetime_str}\n")
                file.write(f"TEMPERATURE={temperature_str}\n")
                file.write(f"PRESSURE={pressure_str}\n")
                file.write(f"ALTITUDE={altitude_str}\n")
            print('--- Values were successfully saved in a temporary file. ---')
        else:
            print('--- Values were not saved in a temporary file. ---')

    def write_ack(self, ok_value):
        subprocess.Popen(["/usr/bin/python", "/home/pi/TimeLapseBox/TimeLapseBox/src/uart_write.py", ok_value])

        #response_history = []
        #noResponse = True
        #while noResponse:
            #self.ser.write(data)
            #print('--- Sended: ', message, ' ---')
            #sleep(0.5)
            #response = self.ser.read()
            #response_history.append(' '.join(hex(byte)[2:] for byte in response))

            # Durch die Liste laufen, wobei der Schritt 2 ist
            #for i in range(0, len(response_history) - 1, 1):
                # Überprüfen, ob die beiden aufeinanderfolgenden Einträge "a" und 5 sind
                #if f'{response_history[i]}{response_history[i + 1]}' == message:
                    #noResponse = False
                    #print('--- Recieved Response: {} ---'.format(message))

readUartClass = ReadUartPi()
readUartClass.processLoop()