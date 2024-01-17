#include <pigpio.h>
#include <iostream>
#include <cctype> // tolower()

using namespace std;

// g++ pi_close_slave.cpp -lpthread -lpigpio -o slaveInstance
// sudo ./slaveInstance

void closeSlave();
int getControlBits(int, bool);

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

    // There is currently no option to close the slave instance after the runtime, so it must be closed manually if necessary.
    closeSlave();

    return 0;
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