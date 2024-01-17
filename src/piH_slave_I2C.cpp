#include <pigpio.h>
#include <iostream>
#include <cctype> // tolower()
#include <iomanip>

using namespace std;

// g++ piH_slave_I2C.cpp -lpthread -lpigpio -o slaveInstance
// sudo ./slaveInstance

void runSlave();
void closeSlave();
int getControlBits(int, bool);
string toLowerCase(string);
string hexToString(string);

const int slaveAddress = 0x09;  // <-- The Pi does not have a fixed I2C slave address, therefore address of choice
bsc_xfer_t xfer;                // Struct to control data flow

/*
typedef struct
{
   uint32_t control;          // Write
   int rxCnt;                 // Read only, the number of received bytes placed in rxBuf
   char rxBuf[BSC_FIFO_SIZE]; // Read only
   int txCnt;                 // Write, the number of bytes to be transmitted
   char txBuf[BSC_FIFO_SIZE]; // Write, the bytes to be trasnmitted
} bsc_xfer_t;
*/

int main() {
    // 'gpioInitialise()' initializes the pigpio library and must be called before most library functions. 
    // Also returns the pigpio version number if successfully executed.
    gpioInitialise();

    runSlave();

    // There is currently no option to close the slave instance after the runtime, so it must be closed manually if necessary.
    //closeSlave();

    return 0;
}

void runSlave() {
    // Close old device (if any)
    xfer.control = getControlBits(slaveAddress, false); // To avoid conflicts when restarting
    bscXfer(&xfer);

    // Set I2C slave Address to 0x0A
    xfer.control = getControlBits(slaveAddress, true);
    //int status = bscXfer(&xfer); // Should now be visible in I2C-Scanners
    
    if (true/*status >= 0*/) {
        // Successfully opened the I2C slave
        cout << "\n" << " Opened slave (Hex)\n" << "\n";
        xfer.rxCnt = 0;

        // Continuous loop to receive data
        while(1) {
            bscXfer(&xfer);
            if(xfer.rxCnt > 0) {
                if(xfer.rxBuf[0] == '0')
                    cout << "\n" << "Transmission received via I2C bus: \n";
                cout << "(" << xfer.rxBuf[0] << ") ";
                cout << "Received " << xfer.rxCnt << " (Hex)  bytes: ";

                string message = "";
                for(int i = 1; i < xfer.rxCnt; i++)
                    message = message + xfer.rxBuf[i];

                message = hexToString(message);
                cout << message << "\n";

                message = toLowerCase(message);
                if(message == "ende" || message == "shutdown") {
                    closeSlave();
                    system("sudo shutdown -h now");
                }
            }
            //if (xfer.rxCnt > 0){
            //    cout << xfer.rxBuf;
            //}
        }
    } else {
        cout << "Failed to open slave!\n";
    }
}

void closeSlave() { 
    // Close old device
    xfer.control = getControlBits(slaveAddress, false);
    bscXfer(&xfer);
    cout << "Closed slave.\n";

    // 'gpioTerminate()' terminates the pigpio library.
    // This function resets the used DMA channels, releases memory, and terminates any running threads.
    gpioTerminate();
}

int getControlBits(int address /* max 127 */, bool open) {
    /*
    Excerpt from http://abyz.me.uk/rpi/pigpio/cif.html#bscXfer regarding the control bits:

    22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    a  a  a  a  a  a  a  -  -  IT HC TF IR RE TE BK EC ES PL PH I2 SP EN

    Bits 0-13 are copied unchanged to the BSC CR register. See pages 163-165 of the Broadcom 
    peripherals document for full details. 
    https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf#page=164

    aaaaaaa defines the I2C slave address (only relevant in I2C mode)
    IT  invert transmit status flags
    HC  enable host control
    TF  enable test FIFO
    IR  invert receive status flags
    RE  enable receive  +
    TE  enable transmit +
    BK  abort operation and clear FIFOs -
    EC  send control register as first I2C byte
    ES  send status register as first I2C byte
    PL  set SPI polarity high
    PH  set SPI phase high
    I2  enable I2C mode
    SP  enable SPI mode
    EN  enable BSC peripheral
    */

    // Flags like this: 0b/*IT:*/0/*HC:*/0/*TF:*/0/*IR:*/0/*RE:*/0/*TE:*/0/*BK:*/0/*EC:*/0/*ES:*/0/*PL:*/0/*PH:*/0/*I2:*/0/*SP:*/0/*EN:*/0;

    int flags;
    if(open)
        flags = /*RE:*/ (1 << 9) | /*TE:*/ (1 << 8) | /*I2:*/ (1 << 2) | /*EN:*/ (1 << 0);
    else // Close/Abort
        flags = /*BK:*/ (1 << 7) | /*I2:*/ (0 << 2) | /*EN:*/ (0 << 0);

    return (address << 16 /*= to the start of significant bits*/) | flags;
}

string toLowerCase(string input) {
    string result;
    for (char ch : input) {
        result += std::tolower(ch);
    }
    return result;
}

string hexToString(string hex) {
    string result;
    istringstream hexStream(hex);

    // Lese zwei Zeichen (ein Byte) gleichzeitig aus dem Hex-String
    for (std::string byte; hexStream >> setw(2) >> byte; ) {
        // Konvertiere das Hex-Byte in einen Dezimalwert und füge es dem Ergebnis hinzu
        char chr = stoi(byte, nullptr, 16);
        result += chr;
    }

    return result;
}
