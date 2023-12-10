# Import libraries
import os
from subprocess import check_output
from datetime import datetime				# Only temporary, will be replaced
from time import sleep

# Import libraries for image processing
import cv2
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
	- check_image(): Checks the quality of the given image and saves it with additional information based on the evaluation.
	- calculate_brightness(): Calculates the average brightness of an image.
	- try_again(): Retries the image capture process after a failed attempt.
	- shutdown_system(): Initiates a system shutdown.
    """

	def __init__(self):
		"""
    	Constructor that initializes the "attempts" attribute to 0.
    	"""
		self.attempts = 0

	def internet_connected(self):
		"""
		Checks whether the device has an active internet connection. By trying to ping the Google server to check the Internet connection.
		
		Returns:
		- True: If the device has a successful connection to the internet.
		- False: If the device does not have a successful connection to the internet or
		encounters an exception during the ping attempt.
		"""
		try:
			# Delay to account for potential network latency
			sleep(20)

			# Attempt to ping Google's server with a timeout of 1 second
			check_output("ping -c 1 google.com", shell=True)
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
		camera.resolution = (2592, 1944)	# max resolution
		# camera.resolution = (2560, 1440)	# QHD
		camera.framerate = 15
		camera.rotation = 180

 		# Start camera preview with a translucent overlay
		camera.start_preview(alpha = 240)
		sleep(3)

		# Capture the image and save it to the specified file path
		camera.capture('/home/dennis/TimeLapseBox/bootImages/test.jpg')
		camera.stop_preview()
		camera.close()

		self.check_image('/home/dennis/TimeLapseBox/bootImages/test')

	def check_image(self, imgPath):
		"""
        Checks the quality of the given image and saves it with additional information based on the evaluation.

		The method reads the image, calculates its sharpness and brightness, and compares them with predefined thresholds.
        If both sharpness and brightness are above the thresholds, the image is considered of acceptable quality,
        and additional information is added to the image. If not, appropriate messages are printed, and the user may try again.

        Parameters:
        - imgPath (str): The path to the image file without the file extension.
        """
		# Read the image
		image = cv2.imread(imgPath + '.jpg', -1)

		# Calculate sharpness
		sharpness = cv2.Laplacian(image, cv2.CV_64F).var()
		sharpness = round(sharpness, 3)

		# Calculate brightness using a helper method
		brightness = self.calculate_brightness(image)

		# Set experimental threshold values
		threshold_sharpness = 57
		threshold_brightness = 20

		# Evaluate image quality based on sharpness and brightness
		if sharpness >= threshold_sharpness and brightness >= threshold_brightness:
			print("Image quality is OK")
			print("   Sharpness: {}".format(sharpness))
			print("   Brightness: {}".format(brightness))

			# Add labels to the image
			font = cv2.FONT_HERSHEY_SIMPLEX
			image = cv2.putText(image, datetime.now().strftime("%d.%m.%Y %H:%M"), (20, image.shape[0] - 110), font, 1.5, (255,255,255), 5, cv2.LINE_AA)
			image = cv2.putText(image, 'Helligkeit: ' + str(brightness), (20, image.shape[0] - 65), font, 1.5, (255,255,255), 5, cv2.LINE_AA)
			image = cv2.putText(image, 'Bildschaerfe: ' + str(sharpness), (20, image.shape[0] - 20), font, 1.5, (255,255,255), 5, cv2.LINE_AA)

			# Display and save the processed image
			cv2.waitKey(0)
			cv2.destroyAllWindows()
			cv2.imwrite(imgPath + "-OpenSV.jpg", image)

		elif sharpness < threshold_sharpness and brightness < threshold_brightness:
			print("Image sharpness and brightness are insufficient")
			print("   Sharpness: {}".format(sharpness))
			print("   Brightness: {}".format(brightness))
			self.try_again()

		elif sharpness < threshold_sharpness:
			print("Image sharpness is insufficient")
			print("   Sharpness: {}".format(sharpness))
			self.try_again()

		else:
			print("Image brightness is insufficient")
			print("   Brightness: {}".format(brightness))
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
			print("Repeat image capture ...")
			print()
			sleep(10)
			self.capture_image()
		else:
			print("Five failed attempts, image capture is canceled")

	def shutdown_system(self):
		"""
        Initiates a system shutdown.

        This method executes a system command to shut down the Raspberry Pi.
        Note: Ensure the script has sufficient permissions to execute shutdown commands.
        """
		os.system("sudo shutdown -h now")

captureImageClass = CaptureImagePi()
captureImageClass.capture_image()
#captureImageClass.shutdown_system()