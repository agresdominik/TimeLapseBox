#!/bin/sh -e
#
# rpi_main_script.sh
#
# This script is executed by the 'rc.local' script, located in the directory '/etc/rc.local'. 
# 'rc.local' serves as a superuser startup script, designed to run commands during system boot. 
# It is considered obsolete but is retained for compatibility with SystemV systems.
#
# The "systemctl status rc-local.service" command shows any error messages or logs associated with its execution. 

# Starts a C++ file that sets up an I2C slave on the Raspberry Pi, allowing it to catch data via the I2C bus.
#g++ /home/pi/TimeLapseBox/TimeLapseBox/src/pi_slave_I2C.cpp -lpthread -lpigpio -o /tmp/slaveInstance
#sudo ./tmp/slaveInstance &

# Starts the pi_camera.py skript.
sudo /usr/bin/python "/home/pi/TimeLapseBox/TimeLapseBox/src/pi_camera.py"

# Checks whether a USB is connected, if so, the bash script for copying an image file to the USB stick is executed
if lsblk | grep -q "sda"; then
    sudo /usr/bin/bash /home/pi/TimeLapseBox/TimeLapseBox/src/rpi_copy_usb.sh
else
    echo "USB stick is not connected. Please connect the USB stick and try again."
fi

# Initiates a system shutdown after the routine has been completed.
# sudo shutdown -h now
exit 0