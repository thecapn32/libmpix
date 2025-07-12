@page supported_operations Supported operations
@brief List of image processing operations supported by libmpix

@note At the moment, the libmpix library only supports C implementation for each operation.
SIMD integration is [in progress](https://github.com/libmpix/libmpix/issues/3).

## Format definitions

@see mpix_formats_h with a list of all pixel formats defined by libmpix.

Note that a presence on that list does not mean that the format can be converted in all direction.
Refer to the other lists below to see what is supported.

You may extend this list directly inside the applicatoin via @ref mpix_bits_per_pixel_cb.

## Pixel format conversion

libmpix can perform pixel format conversion to support multiple input and output formats.

@see mpix_op_convert_h
@dotfile dot/op_convert.dot

## Raw Bayer format conversion

libmpix can perform debayer operations to use
[raw image](https://en.wikipedia.org/wiki/Bayer_filter) as input.

The various options are:

- 3x3, a higher quality but more compute intensive variant
- 2x2, a lower quality but faster to process variant
- 1x1, which lets the bayer pattern unchanged, useful for debug purpose

@see mpix_op_debayer_h
@dotfile dot/op_debayer.dot

## Palette format conversion

libmpix can generate and read data in [indexed colors](https://en.wikipedia.org/wiki/Indexed_color).

The performance quickly goes down as approaching 256 colors (`PALETTE8`),
and the image loose most of the information as approaching 2 colors (`PALETTE1`).
Intermediate palette sizes are a trade-off of performance vs quality.

@see mpix_op_palettize_h
@dotfile dot/op_palettize.dot

## QOI format conversion

libmpix can convert files to the lossless [QOI format](https://qoiformat.org/) which is slightly
less efficient than PNG but lighter in CPU, memory and flash resources.

The output is byte-per-byte identical to the reference implementation.

@see mpix_op_qoi_h
@dotfile dot/op_qoi.dot

## Image correction

libmpix can perform image correction operations as found in most ISP pipelines:

src/op_correction.c

@see mpix_op_correction_h
@dotfile dot/op_correction.dot

## Image kernel processing

libmpix can perform several
[kernel image processing](https://en.wikipedia.org/wiki/Kernel_(image_processing))
operations to work on adjacent pixel affecting the sharpness, noise, blurriness...

@see mpix_op_kernel_h
@dotfile dot/op_kernel.dot

## Image resizing operations

libmpix can resize images using various strategies with a different trade-off between quality,
flexibility and performance.

@see mpix_op_resize_h
@dotfile dot/op_resize.dot
