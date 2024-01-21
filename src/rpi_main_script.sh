#!/bin/sh -e
#
# rpi_main_script.sh
#
# This script is executed by the 'rc.local' script, located in the directory '/etc/rc.local'. 
# 'rc.local' serves as a superuser startup script, designed to run commands during system boot. 
# It is considered obsolete but is retained for compatibility with SystemV systems.
#
# The "systemctl status rc-local.service" command shows any error messages or logs associated with its execution. 

file_path='/tmp/timeLapseData'
sudo touch $file_path

# Starts the uart_read.py script and sets up a UART connection to receive messages that are stored in a temporary file.
sudo /usr/bin/python /home/pi/TimeLapseBox/TimeLapseBox/src/uart_read.py "$file_path"

# Starts the pi_camera.py script and capture its exit code. The exit code shows whether a good picture was taken.
/usr/bin/python /home/pi/TimeLapseBox/TimeLapseBox/src/pi_camera.py "$file_path"
pi_camera_exit=$?

# Checks whether a USB is connected, if so, the bash script for copying an image file to the USB stick is executed
if [ "$pi_camera_exit" -eq 0 ] && [ -f "$file_path" ]; then
    if lsblk | grep -q "sda"; then
        sudo /usr/bin/bash /home/pi/TimeLapseBox/TimeLapseBox/src/rpi_copy_usb.sh "$file_path"
    else
        echo "USB stick is not connected. Please connect the USB stick and try again."
    fi
fi

sudo rm -v "$file_path"

# Initiates a system shutdown after the routine has been completed.
#sudo shutdown -h now
exit 0