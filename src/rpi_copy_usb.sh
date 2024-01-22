#!/bin/sh -e
# Script for copying images to a USB stick (part of the TimeLapseBox routine).

tmp_file_path=$1
image_quality=$2
uart_file=$3

# Path to the target directory.
image_path="/mnt/usb/Pictures"
csv_path="/mnt/usb/TimeLapseData.csv"

# Execute if the USB stick is not recognized automatically.
if ! grep -qs '/mnt/usb' /proc/mounts; then
    sudo mount /dev/sda1 /mnt/usb
    echo "USB stick has been successfully mounted: /mnt/usb"
fi

# Überprüfen, ob der Dateipfad vorhanden ist
if [ -f "$tmp_file_path" ] && [ "$uart_file" -eq 0 ]; then
    # Werte aus der Datei lesen und als Umgebungsvariablen setzen
    while IFS= read -r line; do
        export "$line"
    done < "$tmp_file_path"

    image_name="${NAME}.jpg"
    image_raw_name="${NAME}.jpg"

    # Überprüfe, ob die CSV-Datei existiert, falls nicht, lege sie an
    if [ ! -e "$csv_path" ]; then
        printf "Time,Temperature,Presssure,Altitude\n" > "$csv_path"
    fi
    printf "%s,%s,%s,%s\n" "$DATETIME" "$TEMPERATURE" "$PRESSURE" "$ALTITUDE" >> "$csv_path"
    echo "The CSV file has been successfully updated."
else
    echo "The file does not exist: $tmp_file_path"

    # Get the number of existing images, for alternative image_name.
    count_images=$(find "$image_path" -maxdepth 1 -type f -name '*.jpg' | wc -l)
    image_name="${count_images}.jpg"

    count_images=$(find "$image_path/raw" -maxdepth 1 -type f -name '*.jpg' | wc -l)
    image_raw_name="${count_images}.jpg"
fi

if [ "$image_quality" -eq 0 ]; then
    # Copy the new image and name it accordingly.
    if cp "/home/pi/TimeLapseBox/bootData/image-OpenCV.jpg" "$image_path/$image_name"; then
        cp "/home/pi/TimeLapseBox/bootData/image.jpg" "$image_path/raw/$image_name"
        echo "Image was successfully saved and named as \"$image_name\"."
    else
        echo "Error copying the image file!"
    fi
else
    if cp "/home/pi/TimeLapseBox/bootData/image.jpg" "$image_path/raw/$image_name"; then
        echo "Raw image was successfully saved and named as \"$image_name\"."
    else
        echo "Error copying the raw image file!"
    fi
fi

exit 0