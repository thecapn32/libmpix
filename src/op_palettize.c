/* SPDX-License-Identifier: Apache-2.0 */

#include <assert.h>
#include <errno.h>

#include <mpix/genlist.h>
#include <mpix/sample.h>
#include <mpix/image.h>
#include <mpix/utils.h>
#include <mpix/op_palettize.h>

static const struct mpix_op **mpix_palettize_op_list;

static inline uint8_t mpix_rgb24_to_palette(const uint8_t rgb[3],
					    const struct mpix_palette *palette);

static int mpix_image_append_palettize(struct mpix_image *img, const struct mpix_op *op,
				     struct mpix_palette *palette)
{
	int ret;

	ret = mpix_image_append_uncompressed(img, op);
	if (ret != 0) {
		return ret;
	}

	img->ops.last->arg1 = palette;

	return 0;
}

int mpix_image_update_palette(struct mpix_image *img, struct mpix_palette *palette,
			      uint16_t num_samples)
{
	uint32_t *sums;
	const size_t sums_sz = palette->colors_nb * sizeof(*sums) * 3;
	uint16_t *nums;
	const size_t nums_sz = palette->colors_nb * sizeof(*nums);
	uint8_t rgb[3];

	uint8_t *colors = palette->colors;

	if (palette->format != MPIX_FMT_RGB24) {
		MPIX_ERR("Only RGB24 supported");
		return -ENOTSUP;
	}

	sums = mpix_port_alloc(sums_sz);
	if (sums == NULL) {
		MPIX_ERR("Failed to allocate the sum array");
		return mpix_image_error(img, -ENOMEM);
	}
	memset(sums, 0x00, sums_sz);

	nums = mpix_port_alloc(nums_sz);
	if (nums == NULL) {
		MPIX_ERR("Failed to allocate the num array");
		mpix_port_free(sums);
		return mpix_image_error(img, -ENOMEM);
	}
	memset(nums, 0x00, nums_sz);

	/* Take samples from the input image, find which palette point they belong to, and
	 * accumulate the colors to get an average color
	 */
	for (uint16_t i = 0; i < num_samples; i++) {
		uint8_t idx;

		mpix_sample_random_rgb(img->buffer, img->width, img->height, img->format, rgb);
		idx = mpix_rgb24_to_palette(rgb, palette);

		sums[idx * 3 + 0] += rgb[0];
		sums[idx * 3 + 1] += rgb[1];
		sums[idx * 3 + 2] += rgb[2];
		nums[idx]++;
	}

	/* Average each sum to generate the new palette, with now slightly adjusted colors to
	 * better fit the input image. Repeating the operation allows to improve the accuracy.
	 */
	for (size_t idx = 0; idx < palette->colors_nb; idx++) {
		if (nums[idx] == 0) {
			/* If no value was detected, tune the colors a bit */
			palette->colors[idx * 3 + 0] += 0x10;
			palette->colors[idx * 3 + 1] += 0x10;
			palette->colors[idx * 3 + 2] += 0x10;
		} else {
			/* If there are matches, re-compute a better value */
			palette->colors[idx * 3 + 0] = sums[idx * 3 + 0] / nums[idx];
			palette->colors[idx * 3 + 1] = sums[idx * 3 + 1] / nums[idx];
			palette->colors[idx * 3 + 2] = sums[idx * 3 + 2] / nums[idx];
		}
	}

	mpix_port_free(sums);
	mpix_port_free(nums);

	return 0;
}

int mpix_image_depalettize(struct mpix_image *img, struct mpix_palette *palette)
{
	const struct mpix_op *op = NULL;

	for (size_t i = 0; mpix_palettize_op_list[i] != NULL; i++) {
		const struct mpix_op *tmp = mpix_palettize_op_list[i];

		if (tmp->format_in == img->format && tmp->format_out == palette->format) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		MPIX_ERR("Conversion operation from %s to %s not found",
			 MPIX_FOURCC_TO_STR(img->format), MPIX_FOURCC_TO_STR(palette->format));
		return mpix_image_error(img, -ENOSYS);
	}

	return mpix_image_append_palettize(img, op, palette);
}

int mpix_image_palettize(struct mpix_image *img, struct mpix_palette *palette)
{
	const struct mpix_op *op = NULL;
	uint32_t new_format = 0;
	int ret;

	ret = mpix_image_convert(img, palette->format);
	if (ret != 0) {
		return ret;
	}

	if (palette->colors_nb == 1 << 1) {
		new_format = MPIX_FMT_PALETTE1;
	} else if (palette->colors_nb == 1 << 2) {
		new_format = MPIX_FMT_PALETTE2;
	} else if (palette->colors_nb == 1 << 4) {
		new_format = MPIX_FMT_PALETTE4;
	} else if (palette->colors_nb <= 1 << 8) {
		new_format = MPIX_FMT_PALETTE8;
	} else {
		__builtin_unreachable();
		return -EINVAL;
	}

	for (size_t i = 0; mpix_palettize_op_list[i] != NULL; i++) {
		const struct mpix_op *tmp = mpix_palettize_op_list[i];

		if (tmp->format_in == img->format && tmp->format_out == new_format) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		MPIX_ERR("Conversion operation from %s to %s not found",
			 MPIX_FOURCC_TO_STR(img->format), MPIX_FOURCC_TO_STR(new_format));
		return mpix_image_error(img, -ENOSYS);
	}

	return mpix_image_append_palettize(img, op, palette);
}

void mpix_palettize_op(struct mpix_op *op)
{
	const uint8_t *line_in = mpix_op_get_input_line(op);
	uint8_t *line_out = mpix_op_get_output_line(op);
	void (*fn)(const uint8_t *s, uint8_t *d, uint16_t w, struct mpix_palette *p) = op->arg0;
	struct mpix_palette *palette = op->arg1;

	assert(fn != NULL);

	fn(line_in, line_out, op->width, palette);
	mpix_op_done(op);
}

/*
 * The 3D (R, G, B) distance between two points is given by:
 *
 * 	sqrt((r1 - r0)^2 + (g1 - g0)^2 + (b1 - b0)^2)
 *
 * But if A > B then sqrt(A) > sqrt(B), so no need to compute the square root.
 * The square of the distances are computed instead.
 */
static inline uint32_t mpix_rgb_square_distance(const uint8_t rgb0[3], const uint8_t rgb1[3])
{
	int16_t r_dist = (int16_t)rgb1[0] - (int16_t)rgb0[0];
	int16_t g_dist = (int16_t)rgb1[1] - (int16_t)rgb0[1];
	int16_t b_dist = (int16_t)rgb1[2] - (int16_t)rgb0[2];

	return r_dist * r_dist + g_dist * g_dist + b_dist + b_dist;
}

static inline uint8_t mpix_rgb24_to_palette(const uint8_t rgb[3],
					    const struct mpix_palette *palette)
{
	uint8_t best_color[3];
	uint32_t best_square_distance = UINT32_MAX;
	uint8_t idx = 0;

	for (size_t i = 0; i < palette->colors_nb; i++) {
		uint8_t *color = &palette->colors[i * 3];
		uint32_t square_distance = mpix_rgb_square_distance(color, rgb);

		if (square_distance < best_square_distance) {
			best_square_distance = square_distance;
			memcpy(best_color, color, sizeof(best_color));
			idx = i;
		}
	}

	return idx;
}

__attribute__ ((weak))
void mpix_convert_rgb24_to_palette4(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette)
{
	assert(width % 2 == 0);
	assert(palette->colors_nb == 1 << 4);

	for (uint16_t w = 0; w < width; w += 2, src += 6, dst += 1) {
		dst[0] = 0;
		dst[0] |= mpix_rgb24_to_palette(&src[0], palette) << 4;
		dst[0] |= mpix_rgb24_to_palette(&src[3], palette) << 0;
	}
}
MPIX_REGISTER_PALETTIZE_OP(rgb24_palette4, mpix_convert_rgb24_to_palette4, RGB24, PALETTE4);

__attribute__ ((weak))
void mpix_convert_palette4_to_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				    const struct mpix_palette *palette)
{
	assert(width % 2 == 0);
	assert(palette->colors_nb == 1 << 4);

	for (uint16_t w = 0; w < width; w += 2, src += 1, dst += 6) {
		uint8_t *color0 = &palette->colors[(src[0] >> 4) * 3];
		uint8_t *color1 = &palette->colors[(src[0] & 0xf) * 3];

		dst[0] = color0[0];
		dst[1] = color0[1];
		dst[2] = color0[2];
		dst[3] = color1[0];
		dst[4] = color1[1];
		dst[5] = color1[2];
	}
}
MPIX_REGISTER_PALETTIZE_OP(palette4_rgb24, mpix_convert_palette4_to_rgb24, PALETTE4, RGB24);

static const struct mpix_op **mpix_palettize_op_list = (const struct mpix_op *[]){
	MPIX_LIST_PALETTIZE_OP
};
