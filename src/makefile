#Device Name used by AVRDude
DEVICE = atmega328p
#Programmer used by AVRDude
PROGRAMMER = STK500
#Name of file to be compiled in src directory
FILENAME = main
BUILDDIR = ../build/
PORT = ...
BAUD = 115200
COMPILE = avr-gcc -Wall -Os -mmcu=$(DEVICE)

default: compile upload clean

compile:
	$(COMPILE) -c $(FILENAME).c -o $(BUILDDIR)$(FILENAME).o
	$(COMPILE) -o $(BUILDDIR)$(FILENAME).elf $(BUILDDIR)$(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(BUILDDIR)$(FILENAME).elf $(BUILDDIR)$(FILENAME).hex
	avr-size --format=avr --mcu=$(DEVICE) $(BUILDDIR)$(FILENAME).elf

upload:
	avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$(BUILDDIR)$(FILENAME).hex:i

clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
	rm $(FILENAME).hex