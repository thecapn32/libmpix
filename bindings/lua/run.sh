#!/bin/sh

# Debug logs and crash on missing parameter
set -eux

# Run the freshly build binary
build/mpix-lua-demo imx219_640x480_rgb24.raw >imx219_640x480_rgb24.corrected.jpeg
