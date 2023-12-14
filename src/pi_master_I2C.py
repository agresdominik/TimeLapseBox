import smbus

bus = smbus.SMBus(1)

class PiMaster:
    """
    Initializes a Raspberry Pi as the master for I2C communication. 
    The class features methods for sending data to a specified slave device, 
    resetting the communication counter, and includes a constructor for initializing the class attributes.

	Attributes:
    - counter (int): Shows the progress of a conversation, if reset then a new conversation has started.

	Methods:
    - sendData(slaveAddress, data): Sends data via the I2C bus to the device with the slave address.
    - resetCounter(): Resets the 'counter' attribute, to initiate a new conversation.
    """

    def __init__(self):
        """
    	Constructor that initializes the 'counter' attribute to 0 and 
        prints a status log indicating the successful opening of a test I2C connection.
        It also provides an example usage of the sendData method.
    	"""
        self.counter = 0

        print('Test I2C connection opened as master\n')
        print('Example: sendData(0x09, \'Hello World\')')

    def sendData(self, slaveAddress, data):
        """
        Used for communication via the I2C bus between a Raspberry Pi (master) and an I2C-capable slave device. 
        The code enables data to be sent via the I2C bus to a target device with a specific slave address.
        
        Parameters:
            - slaveAddress (int): The I2C slave address of the target device.
            - data (str): Data to be sent as a string.
        """

        data = str(self.counter) + data
        self.counter = self.counter + 1;

        # Converts each character in the string 'data' to its ASCII value and saves the results.
        intsOfData = list(map(ord, data))

        # Sends the data via the I2C bus to the device with the specified slave address.
        bus.write_i2c_block_data(slaveAddress, intsOfData[0], intsOfData[1:])

    def resetCounter(self):
        """
        Resets the 'counter' attribute to initiate a new conversation.
        """
        self.counter = 0;

def sendData(slaveAddress, data):
    """
    Used for communication via the I2C bus between a Raspberry Pi (master) and an I2C-capable slave device. 
    The code enables data to be sent via the I2C bus to a target device with a specific slave address.
    
    This method only calls the PiMaster class method of the same name, 
    with the only benefit that the user does not have to prefix the statement "master." in the terminal.
    """
    master.sendData(slaveAddress, data)

def resetCounter():
    """
    Resets the 'counter' attribute to initiate a new conversation. 
    
    This method only calls the PiMaster class method of the same name, 
    with the only benefit that the user does not have to prefix the statement "master." in the terminal.
    """
    master.resetCounter()

# Initializes an instance of the class 'PiMaster'
master = PiMaster()