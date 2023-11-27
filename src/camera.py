from picamera import PiCamera
from time import sleep
from datetime import datetime
import os
import subprocess

# Funktion zum Aufnehmen und Speichern der täglichen Bilder
def capture_image():
	camera = PiCamera()
	camera.resolution = (2592, 1944)
	camera.framerate = 15
	camera.rotation = 180

	current_datetime = datetime.now().strftime("%d.%m.%Y %H:%M")
	notes = current_datetime

	#camera.annotate_text = notes
	camera.annotate_text_size = 20

	camera.start_preview(alpha = 240)
	sleep(3)
	camera.capture('/home/dennis/TimeLapseBox/bootImages/{:%Y-%m-%d_%H-%M}.jpg'.format(datetime.now()))
	camera.stop_preview()
	camera.close()

# Funktion zum Prüfen der Internetverbindung
def internet_connected():
	try:
		sleep(20)
		subprocess.check_output("ping -c 1 google.com", shell=True)
		return True
	except:
		return False

if True:
	print("Debugging mode")
else:
	print("Capture mode")
capture_image()
#os.system("sudo shutdown -h now")
