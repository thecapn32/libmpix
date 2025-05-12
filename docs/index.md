Introduction
============

Principles:

- Chain multiple operations without large intermediate buffers
- Front-end API easy to use, accelerators easy to implement.
- A portable API for all hardware, with software fallback for everything.

Features:

- Simple zero-copy and pipelined engine, with low runtime overhead,
- Reduces memory overhead (can process 1 MB of data with only 5 kB of RAM)
- Native Zephyr RTOS module

Upcoming:

- Hardware acceleration ([SIMD](https://www.arm.com/technologies/helium), 2.5D GPUs)
- [MicroPython](https://micropython.org/) support
- [Lua](https://lua.org/) support
- Linux/POSIX support

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

The main user API is @ref mpix_image

This is still an early pre-alpha stage, get-started instructions to come.
