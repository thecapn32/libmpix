@page how_to_correct How to correct images
@brief Uses libmpix to apply image correction to improve contrasts and colors

Raw images coming from a sensor are typically too dark or over-exposed, with green colors and other
visible defects.

Some of the defects cannot be corrected (too much noise, over-exposure), and some are expected and
corrected as part of any classical image correction pipeline.

![](docs/img/snapshot_raw_imx219_zoom_in.png)

If the image arrives without such defects, this suggests that the correction is included inside
the image sensor or the video acquisition hardware.

The input format is always specified when opening the image and does not need to be specified.
What remains to do is to call @ref mpix_image_convert and specify the output pixel format.

## How to do image correction on libmpix

First load a buffer into an image struct, specifying the pixel format:

```c
struct mpix_image img;

mpix_image_from_buf(&img, buf, sizeof(buf), 640, 480, MPIX_FMT_RGB24);
```

All the possible correction tuning is stored inside a single @ref mpix_correction struct.
To control the correction level, you may adjust each parameter to fit a particular camera and
light condition manually:

- `.black_level` is the value to subtract to every pixel to set the black back to 0.
  This can be set to the minimum value you observe in an image and rarely needs to be updated.

- `.red_level` and `.blue_level` are defined as a proportion to the green channel, with 1.0 mapped
  to 1024. Let's try to apply x2 to both blue and red to correct the green tint: `1024 * 2 = 2048`.

- `.gamma_level` gamma correction to non-linearly increase the brightness: dark/bright colors
  remain dark/bright but intermediate tones become brighter to reflect eye's natural light
  sensitivity. Contrasts will appear more natural and accurate.

```c
struct mpix_correction corr = {
        .black_level = 0x0f,
        .red_level = 2048,
        .blue_level = 2048,
	uint8_t.gamma_level = 10,
};
```

Then, apply each step of the correction you wish to the image:

```c
mpix_image_correction(&img, MPIX_CORRECTION_BLACK_LEVEL, &corr);
mpix_image_correction(&img, MPIX_CORRECTION_WHITE_BALANCE, &corr);
mpix_image_correction(&img, MPIX_CORRECTION_GAMMA, &corr);
```

The image will now be corrected using each of the steps specified.

@see supported_operations for a list of all supported image correction operations.
