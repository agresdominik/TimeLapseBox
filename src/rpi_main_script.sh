#!/bin/sh -e
#
# rpi_main_script.sh
#
# This script is designed to be executed by the 'rc.local' script during system boot on a Raspberry Pi.
# 'rc.local' serves as a superuser startup script, designed to run commands during system boot. 
# It is considered obsolete but is retained for compatibility with SystemV systems.
#
# The "systemctl status rc-local.service" command shows any error messages or logs associated with its execution. 

# Path for the temporary file used during the execution of the script
file_path='/tmp/timeLapseData'
sudo touch $file_path

# Starts the uart_read.py script to establish a UART connection for receiving messages that will be stored in a temporary file.
sudo /usr/bin/python /home/pi/TimeLapseBox/TimeLapseBox/src/uart_read.py "$file_path"
uart_exit=$?

# Starts the pi_camera.py script and capture its exit codeto determine if a good picture was taken.
/usr/bin/python /home/pi/TimeLapseBox/TimeLapseBox/src/pi_camera.py "$file_path"
pi_camera_exit=$?

# Checks whether a USB is connected, if so, the bash script for copying an image file to the USB stick is executed
if lsblk | grep -q "sda"; then
    sudo /usr/bin/bash /home/pi/TimeLapseBox/TimeLapseBox/src/rpi_copy_usb.sh "$file_path" "$pi_camera_exit" "$uart_exit"
else
    echo "USB stick is not connected. Please connect the USB stick and try again."
fi

# Remove the temporary file used during the script execution
sudo rm -v "$file_path"

# Initiates a system shutdown after the routine has been completed.
#sudo shutdown
exit 0