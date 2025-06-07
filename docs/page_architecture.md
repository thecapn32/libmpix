@page architecture Architecture
@brief Internal structure of libmpix

SIMD brings video abilities to microcontrollers:
[#1](https://www.synopsys.com/designware-ip/technical-bulletin/signal-processing-risc-v-dsp-extensions.html),
[#2](https://www.arm.com/technologies/helium).

**libmpix** mixes SIMD instructions, hardware accelerators and software to process video data as
efficiently as a chip permits, with RAM down to a few hundread kilobytes.


Original idea
-------------

Color image sensors produce [bayer data](https://en.wikipedia.org/wiki/Bayer_filter),
which go x3 in size once converted to RGB.

An MCU with 1 MByte of RAM cannot store both the input and output VGA frame in these conditions:
`640 * 480 * (1 + 3) = 1228800`.

By inserting a ring buffer between line-processing functions, the large intermediate buffers are
eliminated:

```
convert_1line_awb() --(gearbox)--> convert_2lines_debayer_2x2() --(gearbox)--> convert_8lines_jpeg()
```

Only the final buffer with the compact JPEG image is present.

Because vairous steps do not take the same amount of lines, some adaptation between the function
calls must be made:

```
convert_1line_awb() --+--> convert_2lines_debayer_2x2() --+--> convert_8lines_jpeg()
convert_1line_awb() --'                                   |
convert_1line_awb() --+--> convert_2lines_debayer_2x2() --+
convert_1line_awb() --'                                   |
convert_1line_awb() --+--> convert_2lines_debayer_2x2() --+
convert_1line_awb() --'                                   |
convert_1line_awb() --+--> convert_2lines_debayer_2x2() --'
convert_1line_awb() --'
```

This becomes complex to manage manually.


Introduction of libmpix
-----------------------

**libmpix** automates this flow, permitting to define "stream processors" that are independent on
their context:

![](page_architehcture_stream.drawio.png)

- Facilitates writing new line conversion functions, and stitch them into streams
  (**[gstreamer](https://gstreamer.freedesktop.org/)** style).

- This will be extended in the future to cover a complete ISP pipeline for Zephyr
  (**[libcamera](https://libcamera.org/)** style)

- Most image pre-processing algorithms can be implemented on a line-based fashion
  (**[opencv](https://opencv.org/)** style).

- A driver using this to automatically convert data between input and output formats is provided
  (**[ffmpeg](https://ffmpeg.org/)** style)

Performance-wise:

- Having this extra gearbox library between lines conversion functions adds some overhead,
  but this only happens once a full line is converted. For instance, every `640 * 3` bytes
  for VGA resolution, considered low impact.

- This plays well with SIMD instructions that rarely process data one *column* at a time,
  but instead work on contiguous bytes.

- This permits the transfer of the converted image to start as soon as the first line of data is
  converted, without waiting the full conversion.


Detailled example
-----------------

![](page_architehcture_detail.drawio.png)
