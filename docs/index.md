Introduction
============

The **libmpix** is a library to work with image data on microcontrollers: pixel format conversion,
debayer, blur, sharpen, color correction, resizing...

It pipelines multiple operations together, eliminating the intermediate buffers.
This allows larger resolution to fit in constrained systems, without compromising performance.

It provides a C implementation of the classical operations of an imaging pipeline, and allows
ports or the application alike to implement hardware-optimized operations such as
[ARM Helium](https://www.arm.com/technologies/helium) SIMD instructions.

Features:

- Simple zero-copy and pipelined engine, with low runtime overhead,
- Reduces memory overhead (can process 1 MB of data with only 5 kB of RAM)
- Linux/POSIX support

Upcoming:

- Hardware acceleration ([SIMD](https://www.arm.com/technologies/helium), 2.5D GPUs)
- [MicroPython](https://micropython.org/) support
- [Lua](https://lua.org/) support
- Native Zephyr RTOS module

C API:

```c
#include <mpix/image.h>

struct mpix_image img;

mpix_image_from_buf(&img, buf_in, sizeof(buf_in), MPIX_FORMAT_RGB24);
mpix_image_kernel(&img, MPIX_KERNEL_DENOISE, 5);
mpix_image_kernel(&img, MPIX_KERNEL_SHARPEN, 3);
mpix_image_convert(&img, MPIX_FORMAT_YUYV);
mpix_image_to_buf(&img, buf_out, sizeof(buf_out));

return img.err;
```

This is still an early pre-alpha stage, get-started instructions to come.
