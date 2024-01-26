#Device Name used by AVRDude
DEVICE = atmega328p
#Programmer used by AVRDude
PROGRAMMER = stk500v2
#Name of file to be compiled in src directory
FILENAME = main
#Target build folder
BUILDDIR = ../build/
#Port on witch the device is connected
PORT = /dev/ttyUSB0
#Baud rate for communication between programmer and this device
BAUD = 115200
#Compile Command using avr-gcc -Wall: show all warnings -Os: optimizer for size
COMPILE = avr-gcc -Wall -Os -mmcu=$(DEVICE)

#Execute path
default: compile upload

#Compile Command
#1. Compiles the c file into a o file and saves it in the build folder
#2. Makes a elf file out of the o file in the build order
#3. Copies the elf file into a hex file while only transferring the text and data sections
#4. Checks if the size of the file will function on our microcontroller
compile:
	$(COMPILE) -c $(FILENAME).c -o $(BUILDDIR)$(FILENAME).o
	$(COMPILE) -o $(BUILDDIR)$(FILENAME).elf $(BUILDDIR)$(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(BUILDDIR)$(FILENAME).elf $(BUILDDIR)$(FILENAME).hex
	avr-size --format=avr --mcu=$(DEVICE) $(BUILDDIR)$(FILENAME).elf

#Uploads the hex data onto the device with avrdude
upload:
	avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$(BUILDDIR)$(FILENAME).hex:i

Removes the unneeded files
clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
	rm $(FILENAME).hex
