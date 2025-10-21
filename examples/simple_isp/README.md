# Simple ISP Example

A simple example for working from input file and storing data to an output file, for demo purpose.

To build:

```
cmake -B build
cmake --build build
```

To run it on the input `.raw` image in this directory:

```
sh run.sh
```

This will produce a matching `.corrected.jpeg` file.

Build for 32-bit ARM hard-float test with qemu on fedora:

Install dependencies:

Fedora:

```
sudo dnf copr enable lantw44/arm-linux-gnueabihf-toolchain -y
sudo dnf install -y arm-linux-gnueabihf-binutils arm-linux-gnueabihf-gcc arm-linux-gnueabihf-g++
sudo dnf install -y gcc-arm-linux-gnu gcc-c++-arm-linux-gnu
```

Setup:
```
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabihf-gcc \
  -DCMAKE_C_FLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=hard"
```

Build:
```
cmake --build build
```
Run it with Qemu:

Install Dependencies:

Fedora:

```
sudo dnf install -y cmake ninja-build qemu-user-static qemu-user-binfmt
```

Run:
```
qemu-arm -L /usr/arm-linux-gnueabihf -L /usr/arm-linux-gnueabihf/sys-root ./build/mpix-simple-isp-demo imx219_640x480_rgb24.raw > imx219_640x480_rgb24.jpeg
```