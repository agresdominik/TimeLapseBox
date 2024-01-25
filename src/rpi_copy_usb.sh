#!/bin/sh -e
#
# rpi_copy_usb.sh
#
# Script for copying images and sensor data to a USB stick as part of the TimeLapseBox routine.
#
# Usage: bash copy_images.sh <tmp_file_path> <image_quality> <uart_exit>
# Parameters:
# - <tmp_file_path>: Path to a temporary file containing sensor data.
# - <image_quality>: Quality flag, indicates whether the image capture of an acceptable image was successful.
# - <uart_exit>: UART connection flag, indicates whether the data transfer via the UART connection was successful.

# Assign command line parameters to variables.
tmp_file_path="$1"
image_quality="$2"
uart_exit="$3"

# Path to the target directories.
image_path="/mnt/usb/Pictures"
csv_path="/mnt/usb/TimeLapseData.csv"

# Function to ensure the USB stick is mounted.
ensure_usb_mounted() {
    if ! grep -qs '/mnt/usb' /proc/mounts; then
        sudo mount /dev/sda1 /mnt/usb
        echo "USB stick has been successfully mounted: /mnt/usb"
    fi
}

# Function to create or update CSV file with sensor data.
update_csv_file() {
    # Check if the CSV file exists, if not, create it.
    if [ ! -e "$csv_path" ]; then
        printf "Time;Temperature;Pressure;Altitude\n" > "$csv_path"
    fi

    # Append sensor data to the CSV file.
    printf "%s;%s;%s;%s\n" "$DATETIME" "$TEMPERATURE" "$PRESSURE" "$ALTITUDE" >> "$csv_path"
    echo "The CSV file has been successfully updated."
}

# Check if the temporary file exists and UART exit code is 0.
if [ -f "$tmp_file_path" ] && [ "$uart_exit" -eq 0 ]; then
    # Read values from the file and set them as environment variables.
    while IFS= read -r line; do
        export "$line"
    done < "$tmp_file_path"

    # Set image names based on sensor data.
    image_name="${NAME}.jpg"
    image_raw_name="${NAME}.jpg"

    update_csv_file
else
    if [ -f "$tmp_file_path" ]; then
        echo "No sensor values found in: $tmp_file_path"
    else
        echo "The file does not exist: $tmp_file_path"
    fi

    # Get the number of existing images for alternative image names.
    count_images=$(find "$image_path" -maxdepth 1 -type f -name '*.jpg' | wc -l)
    image_name="${count_images}.jpg"

    count_images=$(find "$image_path/raw" -maxdepth 1 -type f -name '*.jpg' | wc -l)
    image_raw_name="${count_images}.jpg"
fi

ensure_usb_mounted

# Copy images based on the image quality flag.
if [ "$image_quality" -eq 0 ]; then
    # Copy the new image and raw image and name it accordingly
    if cp "/home/pi/TimeLapseBox/bootData/image-OpenCV.jpg" "$image_path/$image_name" &&
       cp "/home/pi/TimeLapseBox/bootData/image.jpg" "$image_path/raw/$image_name"; then
        echo "Image was successfully saved and named as \"$image_name\"."
    else
        echo "Error copying the image file!"
    fi
else
    # Copy only the raw image and name it accordingly
    if cp "/home/pi/TimeLapseBox/bootData/image.jpg" "$image_path/raw/$image_name"; then
        echo "Raw image was successfully saved and named as \"$image_name\"."
    else
        echo "Error copying the raw image file!"
    fi
fi
exit 0