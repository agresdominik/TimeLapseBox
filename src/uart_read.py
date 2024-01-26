import subprocess
import sys
import time
import serial

class ReadUartPi:
    """
    A class for initializing and managing a UART connection to receive sensor data.

    Attributes:
        - ser (serial.Serial): Serial object for UART communication with specified parameters.
        - second, minute, hour (int): Time variables for timestamping sensor data.
        - day, month, year (int): Date variables for timestamping sensor data.
        - temperature_list, pressure_list, altitude_list (List[str]): Lists to store sensor data.
        - start_flag (bool): Flag to indicate the start of a message block.
        - messageNr (int): Variable to keep track of the received message number.
        - timeout_seconds (int): Timeout duration for waiting for UART messages.

    Methods:
        - processLoop(): Main loop for processing incoming UART messages.
        - write_tmp(): Writes the class attributes in a generated tempfile.
        - toStr(value: str) -> str: Converts hex value to a formatted string.
        - write_response(ok_value: int, exit_nr: int): Writes a response to the UART communication.
    """

    def __init__(self):
        """
        Initializes the ReadUartPi class.
        """
        # Initialize UART communication.
        self.ser = serial.Serial(
            port='/dev/ttyAMA0',
            baudrate = 9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )

        # Initialize time variables.
        self.second = []
        self.minute = []
        self.hour = []

        self.day = []
        self.month = []
        self.year = []

        # Initialize sensor data lists.
        self.temperature_list = []
        self.pressure_list = []
        self.altitude_list = []

        # Initialize Flag to indicate the start of a message block
        self.start_flag = False

        # Initialize counters
        self.messageNr = 0
        self.timeout_seconds = 45

        print('--- UART Connection opened ---')
        
    def processLoop(self):
        """
        Main loop for processing incoming UART messages.
        """
        # Defining a mapping between 'self.messageNr' and their corresponding variable names.
        # Each key is associated with a tuple containing the variable name and its display name.
        value_mapping = {
            1: ('second', 'Second'),
            2: ('minute', 'Minute'),
            3: ('hour', 'Hour'),
            4: ('day', 'Day'),
            5: ('month', 'Month'),
            6: ('year', 'Year'),
            7: ('temperature_list', 'Temperature List'),
            8: ('temperature_list', 'Temperature List'),
            9: ('temperature_list', 'Temperature List'),
            10: ('temperature_list', 'Temperature List'),
            11: ('pressure_list', 'Pressure List'),
            12: ('pressure_list', 'Pressure List'),
            13: ('pressure_list', 'Pressure List'),
            14: ('pressure_list', 'Pressure List'),
            15: ('altitude_list', 'Altitude List'),
            16: ('altitude_list', 'Altitude List'),
            17: ('altitude_list', 'Altitude List'),
            18: ('altitude_list', 'Altitude List'),
        }

        # Send a message via UART indicating readiness to receive data.
        self.write_response(0, 0)

        # Set the start time for the next timeout.
        start_time = time.time()

        while True:
            x = self.ser.read() # Read a byte from the UART.
            hex_data = ' '.join(hex(byte)[2:] for byte in x)    # Convert the byte to hex format.

            # Check if a message block has started.
            if self.start_flag:
                # Reset the timeout counter.
                start_time = time.time()

                # Check if the current message number is in the value mapping.
                if self.messageNr in value_mapping:
                    # Retrieve the variable name and display name from the value mapping.
                    value_name, display_name = value_mapping[self.messageNr]

                    # Access the attribute with the retrieved variable name and append the hex_data.
                    getattr(self, value_name).append(hex_data)

                    # Print a message indicating the update of the variable with its value.
                    print('--- {}: {} ---'.format(display_name, hex_data))

                    if self.messageNr == 18:
                        print('--- All values received (18) ---')
                        print('--- UART Connection closed ---')

                        # For a nicer output, because of the subprocess in write_response().
                        time.sleep(1)

                        # Saves the sensor values a temporary file.
                        self.write_tmp()

                        # End the script after processing the message block.
                        exit(0)
                self.messageNr += 1

            # Handle non-empty messages that are not part of a message block.
            if hex_data and hex_data not in ('0',''):
                if not self.start_flag:   
                    print(f'--- A message which is not nothing is found: {hex_data} ---')

                    # Checks if the message announces a message block.
                    if hex_data == 'ff':
                        self.start_flag = True
                        self.messageNr = 0
                        print(f'--- Start of message block found: {hex_data} ---')

            # Check for timeout.
            if time.time() - start_time > self.timeout_seconds:
                print('--- Timeout reached, no message received via UART! ---')
                print('--- UART Connection closed ---')

                # Send an error message over UART.
                self.write_response(1, 0)
                
                # For a nicer output, because of the subprocess in write_response().
                time.sleep(1)

                # End the script
                exit(1)

    def write_tmp(self):
        """
        Writes the sensor values in a generated tempfile.
        """
        if len(sys.argv) > 1:
            str_day = self.toStr(self.day)
            str_month = self.toStr(self.month)
            str_year = self.toStr(self.year)
            str_second = self.toStr(self.second)
            str_minute = self.toStr(self.minute)
            str_hour = self.toStr(self.hour)

            name_str = f'{str_day}-{str_month}-20{str_year}_{str_hour}-{str_minute}-{str_second}'
            datetime_str = f'{str_day}.{str_month}.20{str_year} {str_hour}:{str_minute}:{str_second}'
            temperature_str = str( int( ''.join(filter(lambda x: x != '0', self.temperature_list)), 16 ) )
            pressure_str = str( int( ''.join(filter(lambda x: x != '0', self.pressure_list)), 16 ) )
            altitude_str = str( int( ''.join(filter(lambda x: x != '0', self.altitude_list)), 16 ) )

            with open(sys.argv[1], 'r+') as file:
                file.seek(0)
                file.write(f'NAME={name_str}\n')
                file.write(f'DATETIME={datetime_str}\n')
                file.write(f'TEMPERATURE={temperature_str}\n')
                file.write(f'PRESSURE={pressure_str}\n')
                file.write(f'ALTITUDE={altitude_str}\n')
            print('--- Values were successfully saved in a temporary file. ---')
        else:
            print('--- Values were not saved in a temporary file. ---')

    def toStr(self, value):
        """
        Converts hex value to a formatted string.

        Parameters:
		- value (str): The hex value to be formatted.

		Returns:
		str: The formatted and double-digit string.
        """
        result = str(int(str(value[0]), 16))

        # Check whether the length of the value is less than 2
        if len(result) < 2:
            # Add a leading zero
            return f'0{result}'  
        else:
            # Otherwise return the value unchanged
            return result

    def write_response(self, ok_value, exit_nr):
        """
        Writes a response message using a subprocess.   The outbox number specifies the duration for sending the reply.

        This method starts a subprocess to send a response message for a specified period of time.
        The response message is generated using the '/home/pi/TimeLapseBox/TimeLapseBox/src/uart_write.py' script,
        which takes two arguments: ok_value and exit_nr. These values are converted to strings before being passed
        as command-line arguments to the script.

        Parameters:
            ok_value (int): The value that represents the success or failure of the previous data transfer.
            exit_nr (int): The exit number specifies the duration for sending the response.
            
        Note:
            Ensure that the '/home/pi/TimeLapseBox/TimeLapseBox/src/uart_write.py' script is present at the specified path
            and is executable. Also, make sure that the Python interpreter at '/usr/bin/python' is correct for your system.
        """
        subprocess.Popen(['/usr/bin/python', '/home/pi/TimeLapseBox/TimeLapseBox/src/uart_write.py', str(ok_value), str(exit_nr)])

# Instantiate an instance of the class 'ReadUartPi' and run the class.
readUartClass = ReadUartPi()
readUartClass.processLoop()