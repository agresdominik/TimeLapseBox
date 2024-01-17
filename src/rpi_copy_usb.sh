#!/bin/sh -e
# Script for copying images to a USB stick (part of the TimeLapseBox routine).

# Execute if the USB stick is not recognized automatically.
if ! grep -qs '/mnt/usb' /proc/mounts; then
    sudo mount /dev/sda1 /mnt/usb
    echo "USB stick has been successfully mounted:      /mnt/usb"
fi

# Path to the target directory.
target_path="/mnt/usb/Pictures"

# Get the number of existing images.
count_images=$(ls -l "$target_path" | grep -c -E '\.jpg$|\.png$|\.gif$')

# Copy the new image and name it accordingly.
new_name="${count_images}.jpg"
if cp "/home/pi/TimeLapseBox/bootData/test-OpenCV.jpg" "$target_path/$new_name"; then
    echo "Image was successfully copied and named as \"$new_name\"."
else
    echo "Error copying the image file!"
fi

exit 0