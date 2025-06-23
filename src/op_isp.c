/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <errno.h>

#include <mpix/genlist.h>
#include <mpix/image.h>
#include <mpix/op_isp.h>
#include <mpix/utils.h>

static const struct mpix_isp_op **mpix_isp_op_list;

int mpix_image_signal_processing(struct mpix_image *img, uint32_t type, struct mpix_isp *isp)
{
	const struct mpix_isp_op *op = NULL;
	struct mpix_isp_op *new;
	int ret;

	for (size_t i = 0; mpix_isp_op_list[i] != NULL; i++) {
		const struct mpix_isp_op *tmp = mpix_isp_op_list[i];

		if (tmp->base.fourcc_src == img->fourcc &&
		    tmp->type == type) {
			op = tmp;
			break;
		}
	}

	if (op == NULL) {
		MPIX_ERR("ISP operation %u on %s data not found",
			 type, MPIX_FOURCC_TO_STR(img->fourcc));
		return mpix_image_error(img, -ENOSYS);
	}

	ret = mpix_image_append_uncompressed_op(img, &op->base, sizeof(*op));
	if (ret != 0) {
		return ret;
	}

	new = (struct mpix_isp_op *)img->ops.last;
	new->isp = isp;

	return 0;
}

void mpix_isp_op(struct mpix_base_op *base)
{
	struct mpix_isp_op *op = (void *)base;
	const uint8_t *src = mpix_op_get_input_line(base);
	uint8_t *dst = mpix_op_get_output_line(base);

	op->isp_fn(src, dst, base->width, op->isp);
	mpix_op_done(base);
}

void mpix_isp_black_level_raw8(const uint8_t *src, uint8_t *dst, uint16_t width,
			       struct mpix_isp *isp)
{
	uint8_t level = isp->black_level;

	for (size_t w = 0; w < width; w++, src++, dst++) {
		*dst = MAX(0, *src - level);
	}
}
MPIX_REGISTER_ISP_OP(isp_blc_bggr8, mpix_isp_black_level_raw8, BLACK_LEVEL, BGGR8); /* Zephyr 3.6 */
MPIX_REGISTER_ISP_OP(isp_blc_sbggr8, mpix_isp_black_level_raw8, BLACK_LEVEL, SBGGR8);
MPIX_REGISTER_ISP_OP(isp_blc_srggb8, mpix_isp_black_level_raw8, BLACK_LEVEL, SRGGB8);
MPIX_REGISTER_ISP_OP(isp_blc_sgrbg8, mpix_isp_black_level_raw8, BLACK_LEVEL, SGRBG8);
MPIX_REGISTER_ISP_OP(isp_blc_sgbrg8, mpix_isp_black_level_raw8, BLACK_LEVEL, SGBRG8);
MPIX_REGISTER_ISP_OP(isp_blc_grey, mpix_isp_black_level_raw8, BLACK_LEVEL, GREY);

void mpix_isp_black_level_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				struct mpix_isp *isp)
{
	uint8_t level = isp->black_level;

	for (size_t w = 0; w < width; w++, src += 3, dst += 3) {
		dst[0] = MAX(0, src[0] - level);
		dst[1] = MAX(0, src[1] - level);
		dst[2] = MAX(0, src[2] - level);
	}
}
MPIX_REGISTER_ISP_OP(isp_blc, mpix_isp_black_level_rgb24, BLACK_LEVEL, RGB24);

void mpix_isp_white_balance_rgb24(const uint8_t *src, uint8_t *dst, uint16_t width,
				  struct mpix_isp *isp)
{
	uint16_t red_level = isp->red_level;
	uint16_t blue_level = isp->blue_level;

	for (size_t w = 0; w < width; w++, src += 3, dst += 3) {
		dst[0] = CLAMP(src[0] * (256 + red_level) / 256, 0x00, 0xff);
		dst[1] = src[1];
		dst[2] = CLAMP(src[2] * (256 + blue_level) / 256, 0x00, 0xff);
	}
}
MPIX_REGISTER_ISP_OP(isp_awb_rgb24, mpix_isp_white_balance_rgb24, WHITE_BALANCE, RGB24);

static const struct mpix_isp_op **mpix_isp_op_list =
	(const struct mpix_isp_op *[]){MPIX_LIST_ISP_OP};
