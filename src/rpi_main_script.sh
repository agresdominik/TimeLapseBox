#!/bin/sh -e
#
# rpi_main_script.sh
#
# This script is executed by the 'rc.local' script, located in the directory '/etc/rc.local'. 
# 'rc.local' serves as a superuser startup script, designed to run commands during system boot. 
# It is considered obsolete but is retained for compatibility with SystemV systems.
#
# The "systemctl status rc-local.service" command shows any error messages or logs associated with its execution. 

# Starts a script that calls the camera.py.
sudo /usr/bin/python "/home/dennis/TimeLapseBox/TimeLapseBox/src/pi_camera.py"

exit 0