/*
 * Copyright (C) 2012 Avionic Design GmbH
 * Copyright (C) 2012 NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/platform_data/tegra-host1x.h>
#include <linux/regulator/consumer.h>

#include "drm.h"
#include "dc.h"

struct tegra_rgb {
	struct tegra_output output;
	struct clk *clk_parent;
	struct clk *clk;
	struct regulator *panel_supply;
	bool panel_enabled;
};

static inline struct tegra_rgb *to_rgb(struct tegra_output *output)
{
	return container_of(output, struct tegra_rgb, output);
}

struct reg_entry {
	unsigned long offset;
	unsigned long value;
};

static const struct reg_entry rgb_enable[] = {
	{ DC_COM_PIN_OUTPUT_ENABLE(0),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_ENABLE(1),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_ENABLE(2),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_ENABLE(3),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(0), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(1), 0x01000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(2), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(3), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_DATA(0),     0x00000000 },
	{ DC_COM_PIN_OUTPUT_DATA(1),     0x00000000 },
	{ DC_COM_PIN_OUTPUT_DATA(2),     0x00000000 },
	{ DC_COM_PIN_OUTPUT_DATA(3),     0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(0),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(1),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(2),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(3),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(4),   0x00210222 },
	{ DC_COM_PIN_OUTPUT_SELECT(5),   0x00002200 },
	{ DC_COM_PIN_OUTPUT_SELECT(6),   0x00020000 },
};

static const struct reg_entry rgb_disable[] = {
	{ DC_COM_PIN_OUTPUT_SELECT(6),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(5),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(4),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(3),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(2),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(1),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_SELECT(0),   0x00000000 },
	{ DC_COM_PIN_OUTPUT_DATA(3),     0xaaaaaaaa },
	{ DC_COM_PIN_OUTPUT_DATA(2),     0xaaaaaaaa },
	{ DC_COM_PIN_OUTPUT_DATA(1),     0xaaaaaaaa },
	{ DC_COM_PIN_OUTPUT_DATA(0),     0xaaaaaaaa },
	{ DC_COM_PIN_OUTPUT_POLARITY(3), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(2), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(1), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_POLARITY(0), 0x00000000 },
	{ DC_COM_PIN_OUTPUT_ENABLE(3),   0x55555555 },
	{ DC_COM_PIN_OUTPUT_ENABLE(2),   0x55555555 },
	{ DC_COM_PIN_OUTPUT_ENABLE(1),   0x55150005 },
	{ DC_COM_PIN_OUTPUT_ENABLE(0),   0x55555555 },
};

static void tegra_dc_write_regs(struct tegra_dc *dc,
				const struct reg_entry *table,
				unsigned int num)
{
	unsigned int i;

	for (i = 0; i < num; i++)
		tegra_dc_writel(dc, table[i].value, table[i].offset);
}

static int tegra_output_rgb_enable(struct tegra_output *output)
{
	struct tegra_rgb *rgb = to_rgb(output);
	struct tegra_dc *dc = to_tegra_dc(output->encoder.crtc);
	int err;

	if (rgb->panel_supply && !rgb->panel_enabled) {
		err = regulator_enable(rgb->panel_supply);
		if (err)
			return err;
		rgb->panel_enabled = true;
	}

	tegra_dc_write_regs(dc, rgb_enable, ARRAY_SIZE(rgb_enable));

	return 0;
}

static int tegra_output_rgb_disable(struct tegra_output *output)
{
	struct tegra_rgb *rgb = to_rgb(output);
	struct tegra_dc *dc = to_tegra_dc(output->encoder.crtc);
	int err = 0;

	tegra_dc_write_regs(dc, rgb_disable, ARRAY_SIZE(rgb_disable));
	if (rgb->panel_supply && rgb->panel_enabled) {
		err = regulator_disable(rgb->panel_supply);
		if (!err)
			rgb->panel_enabled = false;
	}

	return err;
}

static int tegra_output_rgb_setup_clock(struct tegra_output *output,
					struct clk *clk, unsigned long pclk)
{
	struct tegra_rgb *rgb = to_rgb(output);

	if (!rgb->clk_parent)
		return 0;

	return clk_set_parent(clk, rgb->clk_parent);
}

static int tegra_output_rgb_check_mode(struct tegra_output *output,
				       struct drm_display_mode *mode,
				       enum drm_mode_status *status)
{
	/*
	 * FIXME: For now, always assume that the mode is okay. There are
	 * unresolved issues with clk_round_rate(), which doesn't always
	 * reliably report whether a frequency can be set or not.
	 */

	*status = MODE_OK;

	return 0;
}

static const struct tegra_output_ops rgb_ops = {
	.enable = tegra_output_rgb_enable,
	.disable = tegra_output_rgb_disable,
	.setup_clock = tegra_output_rgb_setup_clock,
	.check_mode = tegra_output_rgb_check_mode,
};

int tegra_dc_rgb_probe(struct tegra_dc *dc)
{
	struct device_node *np = NULL;
	const struct tegra_dc_platform_data *pdata = dc->dev->platform_data;
	const struct tegra_dc_legacy_mode *panel_mode;
	struct tegra_rgb *rgb;
	int err;

	if (dc->dev->of_node) {
		np = of_get_child_by_name(dc->dev->of_node, "rgb");
		if (!np || !of_device_is_available(np))
			return -ENODEV;
	} else if (!pdata || !pdata->mode) {
		return -ENODEV;
	}

	rgb = devm_kzalloc(dc->dev, sizeof(*rgb), GFP_KERNEL);
	if (!rgb)
		return -ENOMEM;

	rgb->clk = devm_clk_get(dc->dev, NULL);
	if (IS_ERR(rgb->clk)) {
		dev_err(dc->dev, "failed to get clock\n");
		return PTR_ERR(rgb->clk);
	}

	rgb->output.dev = dc->dev;
	rgb->output.hpd_gpio = -EINVAL;
	if (dc->dev->of_node) {
		rgb->clk_parent = devm_clk_get(dc->dev, "parent");
		if (IS_ERR(rgb->clk_parent)) {
			dev_err(dc->dev, "failed to get parent clock\n");
			return PTR_ERR(rgb->clk_parent);
		}

		err = clk_set_parent(rgb->clk, rgb->clk_parent);
		if (err < 0) {
			dev_err(dc->dev, "failed to set parent clock: %d\n", err);
			return err;
		}

		rgb->output.of_node = np;
		err = tegra_output_parse_dt(&rgb->output);
		if (err < 0)
			return err;
	} else {
		panel_mode = pdata->mode;
		rgb->panel_supply = devm_regulator_get(dc->dev, "vdd_panel");
		if (IS_ERR(rgb->panel_supply)) {
			dev_err(dc->dev, "failed to get panel regulator\n");
			return PTR_ERR(rgb->panel_supply);
		}
		rgb->output.has_fixed_mode = true;
		rgb->output.width_mm = pdata->width_mm;
		rgb->output.height_mm = pdata->height_mm;
		rgb->output.fixed_mode.clock = panel_mode->pixelclock / 1000;
		rgb->output.fixed_mode.hdisplay = panel_mode->hactive;
		rgb->output.fixed_mode.hsync_start = panel_mode->hactive +
			panel_mode->hfront_porch;
		rgb->output.fixed_mode.hsync_end =
			rgb->output.fixed_mode.hsync_start + panel_mode->hsync_len;
		rgb->output.fixed_mode.htotal =
			rgb->output.fixed_mode.hsync_end + panel_mode->hback_porch;
		rgb->output.fixed_mode.vdisplay = panel_mode->vactive;
		rgb->output.fixed_mode.vsync_start = panel_mode->vactive +
			panel_mode->vfront_porch;
		rgb->output.fixed_mode.vsync_end =
			rgb->output.fixed_mode.vsync_start + panel_mode->vsync_len;
		rgb->output.fixed_mode.vtotal =
			rgb->output.fixed_mode.vsync_end + panel_mode->vback_porch;
		rgb->output.fixed_mode.flags = DRM_MODE_FLAG_PHSYNC |
			DRM_MODE_FLAG_PVSYNC;
		drm_mode_set_name(&rgb->output.fixed_mode);
	}

	dc->rgb = &rgb->output;

	return 0;
}

int tegra_dc_rgb_init(struct drm_device *drm, struct tegra_dc *dc)
{
	struct tegra_rgb *rgb;
	int err;

	if (!dc->rgb)
		return -ENODEV;
	rgb = to_rgb(dc->rgb);

	rgb->output.type = TEGRA_OUTPUT_RGB;
	rgb->output.ops = &rgb_ops;

	err = tegra_output_init(dc->base.dev, &rgb->output);
	if (err < 0) {
		dev_err(dc->dev, "output setup failed: %d\n", err);
		return err;
	}

	/*
	 * By default, outputs can be associated with each display controller.
	 * RGB outputs are an exception, so we make sure they can be attached
	 * to only their parent display controller.
	 */
	rgb->output.encoder.possible_crtcs = 1 << dc->pipe;

	return 0;
}

int tegra_dc_rgb_exit(struct tegra_dc *dc)
{
	if (dc->rgb) {
		int err;

		err = tegra_output_disable(dc->rgb);
		if (err < 0) {
			dev_err(dc->dev, "output failed to disable: %d\n", err);
			return err;
		}

		err = tegra_output_exit(dc->rgb);
		if (err < 0) {
			dev_err(dc->dev, "output cleanup failed: %d\n", err);
			return err;
		}

		dc->rgb = NULL;
	}

	return 0;
}
