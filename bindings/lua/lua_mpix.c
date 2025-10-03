/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

#include <mpix/genlist.h>
#include <mpix/lua.h>
#include <mpix/image.h>
#include <mpix/operation.h>
#include <mpix/print.h>

static struct mpix_image *lua_mpix_image;
static struct mpix_palette lua_mpix_palette;

static int lua_mpix_op(lua_State *L, enum mpix_op_type type, const char *name)
{
	int32_t params[32];
	size_t nb = 0;
	int err;

	/* p*: fill all into a c array */
	for (nb = 0; nb < ARRAY_SIZE(params) && nb + 1 <= (size_t)lua_gettop(L); nb++) {
		params[nb] = lua_tointeger(L, nb + 1);
	}
	lua_pop(L, nb);

	/* add the element to the pipeline */
	err = mpix_pipeline_add(lua_mpix_image, type, params, nb);
	if (err == -EBADMSG) luaL_error(L, "invalid number of arguments for %s operation", name);
	if (err) luaL_error(L, "failed to add %s to the pipeline: %s", name, strerror(-err));

	return 0;
}

/* one wrapper function for every operation */
#define LUA_MPIX_IMAGE_FN(X, x) \
static int lua_mpix_##x(lua_State *L) { return lua_mpix_op(L, MPIX_OP_##X, #X); }
MPIX_FOR_EACH_OP(LUA_MPIX_IMAGE_FN)

static int lua_mpix_dump(lua_State *L)
{
	mpix_print_pipeline(lua_mpix_image->first_op);
	mpix_print_ctrls(lua_mpix_image->ctrls);
	return 0;
}

static int lua_mpix_format(lua_State *L)
{
	struct mpix_image *img = lua_mpix_image;

	mpix_image_format(lua_mpix_image);

	/* R1: Table containing the width/height/format fields */
	lua_newtable(L);

	lua_pushinteger(L, img->fmt.width);
	lua_setfield(L, -2, "width");

	lua_pushinteger(L, img->fmt.height);
	lua_setfield(L, -2, "height");

	lua_pushinteger(L, img->fmt.fourcc);
	lua_setfield(L, -2, "pixel_format");

	lua_pushinteger(L, mpix_format_pitch(&img->fmt));
	lua_setfield(L, -2, "bytes_per_line");

	return 1;
}

static int lua_mpix_ctrl(lua_State *L)
{
	struct mpix_image *img = lua_mpix_image;
	int err;

	/* P1: control id */
	int cid = lua_tointeger(L, 1);

	/* P2: control value */
	int32_t value = lua_tointeger(L, 2);

	/* Apply the control to the pipeline */
	err = mpix_image_ctrl_value(img, cid, value);
	if (err) luaL_error(L, "'%s' while applying control to the pipeline", strerror(-err));

	return 0;
}

static int lua_mpix_optimize_palette(lua_State *L)
{
	struct mpix_image *img = lua_mpix_image;
	struct mpix_palette *palette = &lua_mpix_palette;
	int err;

	/* P1: number of samples for optimizing the palette */
	lua_Integer num_samples = lua_tointeger(L, 1);

	/* Optimize the palette as many samples as requested */
	err = mpix_image_optimize_palette(img, palette, num_samples);
	if (err) luaL_error(L, "failed to optimize the color palette (format=%s)",
			    MPIX_FOURCC_TO_STR(palette->fourcc));

	return 0;
}

static void lua_mpix_palette_hooks(lua_State *L)
{
	struct mpix_image *img = lua_mpix_image;
	struct mpix_palette *palette = &lua_mpix_palette;
	struct mpix_image palette_img = {};
	int err;

	/* Try to find a palette through the image to get the fourcc */
	err = mpix_pipeline_get_palette_fourcc(img->first_op, palette);
	if (err) return; /* nothing to do */

	/* Turn the color palette into an image to apply the correction on it */
	mpix_image_from_palette(&palette_img, palette);

	/* Apply the color corretction to the palette */
	for (struct mpix_base_op *op = img->first_op; op != NULL; op = op->next) {
		switch (op->type) {
		case MPIX_OP_CORRECT_BLACK_LEVEL:
		case MPIX_OP_CORRECT_COLOR_MATRIX:
		case MPIX_OP_CORRECT_GAMMA:
		case MPIX_OP_CORRECT_WHITE_BALANCE:
			err = mpix_pipeline_add(&palette_img, op->type, NULL, 0);
			if (err) luaL_error(L, "%s at line %u", __func__, __LINE__);
			break;
		default:
			break;
		}
	}

	/* Transfer all controls present on both */
	for (int cid = 0; cid < MPIX_NB_CID; cid++) {
		if (img->ctrls[cid] != NULL && palette_img.ctrls[cid] != NULL) {
			size_t size =  mpix_image_ctrl_size(cid) * sizeof(*img->ctrls);
			memcpy(palette_img.ctrls[cid], img->ctrls[cid], size);
		}
	}

	/* Apply the image correction to the color palette to get accurate colors */
	err = mpix_image_to_palette(&palette_img, palette);
	if (err) luaL_error(L, "%s at line %u", __func__, __LINE__);

	/* Apply it to all palette operations */
	err = mpix_pipeline_set_palette(img->first_op, palette);
	if (err) luaL_error(L, "%s at line %u", __func__, __LINE__);
}

int lua_mpix_hooks(lua_State *L)
{
	lua_mpix_palette_hooks(L);
	return 0;
}

static const struct luaL_Reg lua_mpix_reg[] = {
	{ "optimize_palette", lua_mpix_optimize_palette },
	{ "dump", lua_mpix_dump },
	{ "ctrl", lua_mpix_ctrl },
	{ "format", lua_mpix_format },
	{ NULL, NULL },
};

static const struct luaL_Reg lua_mpix_op_reg[] = {
#define MPIX_IMAGE_REG(X, x) \
	{ #x, lua_mpix_##x },
MPIX_FOR_EACH_OP(MPIX_IMAGE_REG)
	{ NULL, NULL },
};

void lua_mpix_set_image(struct mpix_image *img)
{
	lua_mpix_image = img;
}

int luaopen_mpix(lua_State *L)
{
	/* Table for the module */
	lua_newtable(L);

	/* Add top-level functions */
	luaL_setfuncs(L, lua_mpix_reg, 0);

	/* Add table of formats */
	lua_newtable(L);
	for (int i = 0; mpix_str_fmt[i].name != NULL; i++) {
		lua_pushinteger(L, mpix_str_fmt[i].value);
		lua_setfield(L, -2, mpix_str_fmt[i].name);
	}
	lua_setfield(L, -2, "fmt");

	/* Add table of control IDs */
	lua_newtable(L);
	for (int i = 0; i < MPIX_NB_CID; i++) {
		lua_pushinteger(L, i);
		lua_setfield(L, -2, mpix_str_cid[i]);
	}
	lua_setfield(L, -2, "cid");

	/* Add table of kernel convolution operations */
	lua_newtable(L);
	for (int i = 0; i < MPIX_NB_KERNEL; i++) {
		lua_pushinteger(L, i);
		lua_setfield(L, -2, mpix_str_kernel[i]);
	}
	lua_setfield(L, -2, "kernel");

	/* Add table of operation functions */
	lua_newtable(L);
	luaL_setfuncs(L, lua_mpix_op_reg, 0);
	lua_setfield(L, -2, "op");

	/* Return the module to i.e. store a as global */
	return 1;
}
