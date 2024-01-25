# Import libraries
import sys
from datetime import datetime
from subprocess import check_output
from time import sleep

# Import regular expressions
import re

# Import libraries for image processing
import cv2
import numpy
from picamera import PiCamera


class CaptureImagePi:
	"""
    The class captures images using the Raspberry Pi camera, checks the image quality based on sharpness and brightness, 
	and saves the image with additional information if it meets the quality criteria.

	Attributes:
    - attempts (int): The number of attempts to take a picture in succession.

	Methods:
    - internet_connected(): Checks if the device has an active internet connection.
    - capture_image(): Captures an image and saves it to a specified location.
	- check_image(imgPath): Checks the quality of the given image and saves it with additional information based on the evaluation.
	- calculate_brightness(image): Calculates the average brightness of an image.
	- try_again(): Retries the image capture process after a failed attempt.
	- processing_data(image, brightness, sharpness): Add annotations to brightness and sharpness information to an image.
    """

	def __init__(self):
		"""
    	Constructor that initializes the 'attempts' attribute to 0.
    	"""
		self.attempts = 0
		self.brightness_adjustment = 0

	def internet_connected(self):
		"""
		Checks whether the device has an active internet connection. By trying to ping the Google server to check the Internet connection.
		
		Returns:
		- True: If the device has a successful connection to the internet.
		- False: If the device does not have a successful connection to the internet or encounters an exception during the ping attempt.
		"""
		try:
			# Delay to account for potential network latency
			sleep(15)

			# Attempt to ping Google's server with a timeout of 1 second
			check_output('ping -c 1 google.com', shell=True)
			return True
		except:
			return False

	def capture_image(self):
		"""
		Captures an image using the Raspberry Pi Camera and saves it to a specified location.

		This method initializes the PiCamera, sets the resolution, framerate and rotation,
		captures an image after a 3-second delay and closes the camera.
		
		The captured image is saved at the specified location and checked further using the check_image method.

		Note: Ensure the PiCamera module is installed before using this method.
		"""
		# Initialize the PiCamera
		camera = PiCamera()

		# Set camera parameters
		camera.resolution = (2592, 1944)    # max resolution
		# camera.resolution = (2560, 1440)  # QHD
		camera.framerate = 15
		camera.rotation = 180

		try:
			# Start camera preview with a translucent overlay
			camera.start_preview(alpha=240)
			sleep(3)

			# Capture the image and save it to the specified file path
			camera.capture('/home/pi/TimeLapseBox/bootData/image.jpg')
			camera.stop_preview()
			camera.close()
		except Exception as e:
			print(f'Error when capturing the image: {e}')
			camera.close()

		self.check_image('/home/pi/TimeLapseBox/bootData/image')

	def check_image(self, imgPath):
		"""
        Checks the quality of the given image and saves it with additional information based on the evaluation.

		The method reads the image, calculates its sharpness and brightness, and compares them with predefined thresholds.
        If both sharpness and brightness are above the thresholds, the image is considered of acceptable quality,
        and additional information is added to the image. If not, appropriate messages are printed, 
		and the process is restarted for a new attempt if appropriate.

        Parameters:
        - imgPath (str): The path to the image file without the file extension.
        """
		# Read the image
		image = cv2.imread(imgPath + '.jpg', -1)

		# Adjusts the brightness of the image using NumPy by adding a specified value.
		image = numpy.clip(image + self.brightness_adjustment, 0, 255)

		# Calculate sharpness
		sharpness = cv2.Laplacian(image, cv2.CV_64F).var()
		if not sharpness.is_integer():
			sharpness = round(sharpness, 3)

		# Calculate brightness using a helper method
		brightness = self.calculate_brightness(image)

		# Specify the threshold value for sharpness and brightness.
		threshold_sharpness = 57
		threshold_brightness = 25
		print()

		# Evaluate image quality based on sharpness and brightness:
		# Check if both sharpness and brightness meet the specified thresholds.
		if sharpness >= threshold_sharpness and brightness >= threshold_brightness:
			# Print a message indicating that the image quality is acceptable.
			print(f'Image quality is OK ({threshold_sharpness}/{threshold_brightness})')
			print('   Sharpness: {}'.format(sharpness))
			print('   Brightness: {}'.format(brightness))

			# Initialize variables for environmental sensor data with default placeholders.
			date_time = '-'
			temperature = '-'
			pressure = '-'
			altitude = '-'

			# Read sensor values from an external environment file specified as a command line argument.
			if len(sys.argv) > 1:
				with open(sys.argv[1], 'r') as file:
					content = file.read()

					# Extract relevant information using regular expressions.
					datetime_match = re.search(r'DATETIME=(.*)', content)
					temperature_match = re.search(r'TEMPERATURE=(.*)', content)
					pressure_match = re.search(r'PRESSURE=(.*)', content)
					altitude_match = re.search(r'ALTITUDE=(.*)', content)

					# Update variables with sensor data if matches are found in the file content.
					if datetime_match:
						date_time = datetime_match.group(1)
					if temperature_match:
						temperature = temperature_match.group(1)
					if pressure_match:
						pressure = pressure_match.group(1)
					if altitude_match:	
						altitude = altitude_match.group(1)				

			# Checks whether there is an Internet connection in order to have a second source for date_time in the event of a problem.
			if self.internet_connected() and date_time == '-':
				date_time = datetime.now().strftime('%d.%m.%Y %H:%M')

			# Set font properties for image annotations.
			font = cv2.FONT_HERSHEY_DUPLEX
			font_size = 2

			# Add label annotations to the image overlay.
			image = cv2.putText(image, date_time, (20, image.shape[0] - 170), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)
			image = cv2.putText(image, 'Temperature: ' + temperature + ' Grad Celsius', (20, image.shape[0] - 120), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)
			image = cv2.putText(image, 'Pressure: ' + pressure + ' kPa', (20, image.shape[0] - 70), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)
			image = cv2.putText(image, 'Altidude: ' + altitude + ' m', (20, image.shape[0] - 20), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)

			# Checks whether all values have a valid value.
			if '-' in (date_time, temperature, pressure, altitude):
				image = cv2.putText(image, 'CONNECTION ERROR', (20, image.shape[0] - 250), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)

			# Print some processing data.
			#self.processing_data(image, brightness, sharpness)

			# Display and save the processed image.
			cv2.waitKey(0)
			cv2.destroyAllWindows()
			cv2.imwrite(imgPath + '-OpenCV.jpg', image)

		# Check if image sharpness and brightness is below the thresholds.
		elif sharpness < threshold_sharpness and brightness < threshold_brightness:
			print(f'Image sharpness and brightness are insufficient ({threshold_sharpness}/{threshold_brightness})')
			print('   Sharpness: {}'.format(sharpness))
			print('   Brightness: {}'.format(brightness))

			# Increment the brightness adjustment by 15.
			self.brightness_adjustment += 15

			# Restart the image capture process with the updated brightness adjustment.
			self.try_again()

		# Check if image sharpness is below the threshold
		elif sharpness < threshold_sharpness:
			print(f'Image sharpness is insufficient ({threshold_sharpness})')
			print('   Sharpness: {}'.format(sharpness))

			# Restart the image capture process to obtain a better image.
			self.try_again()
	
		else:
			print(f'Image brightness is insufficient ({threshold_brightness})')
			print('   Brightness: {}'.format(brightness))

			# Increment the brightness adjustment by 15.
			self.brightness_adjustment += 15

			# Restart the image capture process with the updated brightness adjustment.
			self.try_again()

	def calculate_brightness(self, image):
		"""
		Calculates the average brightness of an image.

		This method converts the input image to grayscale and then computes the
		average pixel intensity, representing the overall brightness of the image.

		Parameters:
		- image (numpy.ndarray): The input image in BGR format (OpenCV image).

		Returns:
		int: The average brightness of the image, represented as an integer value.
		"""
		# Convert the image to grayscale
		gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

		# Calculate the average pixel intensity
		return int(gray.mean())
	
	def try_again(self):
		"""
		Retries the image capture process after a failed attempt.

		This method increments the 'attempts' attribute by 1 and checks if the number
		of attempts is less than 5. If so, it prints a message to try again, sleeps for
		10 seconds, and then calls the 'capture_image' method recursively. If the number
		of attempts reaches 5 or more, it prints a message indicating that five failed
		attempts have occurred, and the image capture process is canceled.
		"""
		self.attempts = self.attempts + 1
		if self.attempts < 5:
			print('Repeat image capture ...')
			print()
			sleep(10)
			self.capture_image()
		else:
			print('Five failed attempts, image capture is canceled')
			exit(1)

	def processing_data(self, image, brightness, sharpness):
		"""
		Add annotations to brightness and sharpness information to an image.

		This method takes an image and two numerical values representing brightness and sharpness.
		It then annotates the input image with text displaying the brightness and sharpness values.

		Parameters:
		- image (numpy.ndarray): The input image to be processed.
		- brightness (float): The brightness value to be displayed on the image.
		- sharpness (float): The sharpness value to be displayed on the image.
		"""
		# Set font properties for image annotations.
		font = cv2.FONT_HERSHEY_DUPLEX
		font_size = 2

		# Compare brightness and sharpness values for consistent annotation.
		y = max(str(brightness), str(sharpness))

    	# Add annotation for brightness.
		brightness_text = 'Helligkeit: '
		text_width_b, text_height_b = cv2.getTextSize(brightness_text + y, font, 1.5, font_size)[0]
		brightness_text += str(brightness)

		# Determine position for brightness annotation
		brightness_y = image.shape[1] - (text_width_b + 10)
		brightness_x = image.shape[0] - 70

		# Put brightness annotation on the image
		image = cv2.putText(image, brightness_text, (brightness_y, brightness_x), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)

		# Add annotation for sharpness
		sharpness_text = 'Bildschaerfe: '
		text_width_s, text_height_s = cv2.getTextSize(sharpness_text  + y, font, 1.5, font_size)[0]
		sharpness_text += str(sharpness)

	 	# Determine position for sharpness annotation
		sharpness_y = image.shape[1] - (text_width_s + 10)
		sharpness_x = image.shape[0] - 20

		# Put sharpness annotation on the image
		image = cv2.putText(image, sharpness_text, (sharpness_y, sharpness_x), font, 1.5, (255,255,255), font_size, cv2.LINE_AA)

# Instantiate an instance of the class 'CaptureImagePi'.
captureImageClass = CaptureImagePi()
captureImageClass.capture_image()
print()	# Output for better visual clarity.
exit(0)