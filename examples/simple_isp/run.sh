#!/bin/sh

# Debug logs and crash on missing parameter
set -eux

src=./imx219_640x480_rgb24.raw
dst=./imx219_640x480_rgb24.corrected.raw
png=./imx219_640x480_rgb24.corrected.png

# Run the freshly build binary
build/mpix-simple-isp-demo "$src" >"$dst"

# Convert the RGB24 output to PNG
ffmpeg -hide_banner -loglevel error -y -f rawvideo -pix_fmt rgb24 -s 640x480 -i "$dst" "$png"
