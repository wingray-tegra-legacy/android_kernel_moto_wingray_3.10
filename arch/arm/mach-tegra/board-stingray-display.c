#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/machine.h>
#include <linux/platform_data/lp855x.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/platform_data/tegra-host1x.h>
#include <linux/platform_data/tegra-pinctrl.h>
#include <linux/sizes.h>

#include "iomap.h"
#include "gpio-names.h"

#define STINGRAY_GIC_IRQ(n) ((n) + 32)
#define STINGRAY_PINCTRL_NAME "tegra20-pinctrl"

static struct lp855x_rom_data stingray_backlight_rom[] = {
	{ .addr = 0xa0, .val = 0xa1 },
	{ .addr = 0xa1, .val = 0x67 },
	{ .addr = 0xa2, .val = 0xbf },
	{ .addr = 0xa3, .val = 0x3f },
	{ .addr = 0xa4, .val = 0x08 },
	{ .addr = 0xa5, .val = 0xea },
	{ .addr = 0xa6, .val = 0x64 },
	{ .addr = 0xa7, .val = 0x2a },
};

static struct lp855x_platform_data stingray_backlight_data = {
	.name = "lcd-backlight",
	.device_control = 0x02,
	.initial_brightness = 0x80,
	.brightness_reg = 0xa0,
	.brightness_scale = 726,
	.enable_control = 0x03,
	.disable_control = 0x04,
	.size_program = ARRAY_SIZE(stingray_backlight_rom),
	.rom_data = stingray_backlight_rom,
};

static struct i2c_board_info stingray_backlight_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("lp8550", 0x2c),
		.platform_data = &stingray_backlight_data,
	},
};

static const struct tegra_dc_legacy_mode stingray_panel_mode = {
	.pixelclock = 66400000,
	.h_ref_to_sync = 11,
	.v_ref_to_sync = 1,
	.hsync_len = 26,
	.vsync_len = 6,
	.hback_porch = 12,
	.vback_porch = 3,
	.hactive = 1280,
	.vactive = 800,
	.hfront_porch = 45,
	.vfront_porch = 3,
};

static const struct tegra_dc_platform_data stingray_display_data = {
	.mode = &stingray_panel_mode,
	.width_mm = 217,
	.height_mm = 136,
};

static unsigned long stingray_lcd_driven[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_PULL,
			   TEGRA_PINCONFIG_PULL_DOWN),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_TRISTATE,
			   TEGRA_PINCONFIG_DRIVEN),
};

static unsigned long stingray_lcd_pullup[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_PULL,
			   TEGRA_PINCONFIG_PULL_UP),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_TRISTATE,
			   TEGRA_PINCONFIG_DRIVEN),
};

#define LCD_MUX(pin) PIN_MAP_MUX_GROUP_DEFAULT("tegradc.0", \
	STINGRAY_PINCTRL_NAME, pin, "displaya")
#define LCD_DRIVEN(pin) PIN_MAP_CONFIGS_GROUP_DEFAULT("tegradc.0", \
	STINGRAY_PINCTRL_NAME, pin, stingray_lcd_driven)

static const struct pinctrl_map stingray_display_pinmux[] = {
	PIN_MAP_MUX_GROUP_DEFAULT("tegra-i2c.0", STINGRAY_PINCTRL_NAME,
				  "spdi", "i2c1"),
	LCD_MUX("ld0"), LCD_DRIVEN("ld0"),
	LCD_MUX("ld1"), LCD_DRIVEN("ld1"),
	LCD_MUX("ld2"), LCD_DRIVEN("ld2"),
	LCD_MUX("ld3"), LCD_DRIVEN("ld3"),
	LCD_MUX("ld4"), LCD_DRIVEN("ld4"),
	LCD_MUX("ld5"), LCD_DRIVEN("ld5"),
	LCD_MUX("ld6"), LCD_DRIVEN("ld6"),
	LCD_MUX("ld7"), LCD_DRIVEN("ld7"),
	LCD_MUX("ld8"), LCD_DRIVEN("ld8"),
	LCD_MUX("ld9"), LCD_DRIVEN("ld9"),
	LCD_MUX("ld10"), LCD_DRIVEN("ld10"),
	LCD_MUX("ld11"), LCD_DRIVEN("ld11"),
	LCD_MUX("ld12"), LCD_DRIVEN("ld12"),
	LCD_MUX("ld13"), LCD_DRIVEN("ld13"),
	LCD_MUX("ld14"), LCD_DRIVEN("ld14"),
	LCD_MUX("ld15"), LCD_DRIVEN("ld15"),
	LCD_MUX("ld16"), LCD_DRIVEN("ld16"),
	LCD_MUX("ld17"), LCD_DRIVEN("ld17"),
	LCD_MUX("ldi"), LCD_DRIVEN("ldi"),
	LCD_MUX("lpp"), LCD_DRIVEN("lpp"),
	LCD_MUX("lhp0"), LCD_DRIVEN("lhp0"),
	LCD_MUX("lhp1"), LCD_DRIVEN("lhp1"),
	LCD_MUX("lhp2"), LCD_DRIVEN("lhp2"),
	LCD_MUX("lvp1"), LCD_DRIVEN("lvp1"),
	LCD_MUX("lhs"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("tegradc.0", STINGRAY_PINCTRL_NAME,
				       "lhs", stingray_lcd_pullup),
	LCD_MUX("lvs"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("tegradc.0", STINGRAY_PINCTRL_NAME,
				       "lvs", stingray_lcd_pullup),
	LCD_MUX("lsc0"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("tegradc.0", STINGRAY_PINCTRL_NAME,
				       "lsc0", stingray_lcd_pullup),
};

static struct resource stingray_i2c1_resources[] = {
	{ .start = 0x7000c000, .end = 0x7000c0ff, .flags = IORESOURCE_MEM },
	{ .start = STINGRAY_GIC_IRQ(12), .end = STINGRAY_GIC_IRQ(12),
	  .flags = IORESOURCE_IRQ },
};

static struct platform_device stingray_i2c1_device = {
	.name = "tegra-i2c",
	.id = 0,
	.num_resources = ARRAY_SIZE(stingray_i2c1_resources),
	.resource = stingray_i2c1_resources,
};

static struct regulator_consumer_supply stingray_panel_supply[] = {
	{ .supply = "vdd_panel", .dev_name = "tegradc.0" },
};

static struct regulator_init_data stingray_panel_regulator_init = {
	.constraints = {
		.name = "stingray-panel",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.consumer_supplies = stingray_panel_supply,
	.num_consumer_supplies = ARRAY_SIZE(stingray_panel_supply),
};

static struct fixed_voltage_config stingray_panel_regulator_config = {
	.supply_name = "stingray-panel",
	.microvolts = 5000000,
	.gpio = TEGRA_GPIO_PB2,
	.startup_delay = 200000,
	.enable_high = 1,
	.enabled_at_boot = 1,
	.init_data = &stingray_panel_regulator_init,
};

static struct platform_device stingray_panel_regulator_device = {
	.name = "reg-fixed-voltage",
	.id = 0,
	.dev.platform_data = &stingray_panel_regulator_config,
};

static struct resource stingray_host1x_resources[] = {
	{ .start = TEGRA_HOST1X_BASE,
	  .end = TEGRA_HOST1X_BASE + TEGRA_HOST1X_SIZE - 1,
	  .flags = IORESOURCE_MEM },
	{ .start = STINGRAY_GIC_IRQ(65), .end = STINGRAY_GIC_IRQ(65),
	  .flags = IORESOURCE_IRQ },
	{ .start = STINGRAY_GIC_IRQ(67), .end = STINGRAY_GIC_IRQ(67),
	  .flags = IORESOURCE_IRQ },
};

static u64 stingray_host1x_dma_mask = DMA_BIT_MASK(32);

static struct platform_device stingray_host1x_device = {
	.name = "host1x",
	.id = -1,
	.num_resources = ARRAY_SIZE(stingray_host1x_resources),
	.resource = stingray_host1x_resources,
	.dev = {
		.dma_mask = &stingray_host1x_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static struct resource stingray_display_resources[] = {
	{ .start = TEGRA_DISPLAY_BASE,
	  .end = TEGRA_DISPLAY_BASE + TEGRA_DISPLAY_SIZE - 1,
	  .flags = IORESOURCE_MEM },
	{ .start = STINGRAY_GIC_IRQ(73), .end = STINGRAY_GIC_IRQ(73),
	  .flags = IORESOURCE_IRQ },
};

static struct platform_device stingray_display_device = {
	.name = "tegradc",
	.id = 0,
	.num_resources = ARRAY_SIZE(stingray_display_resources),
	.resource = stingray_display_resources,
	.dev = {
		.parent = &stingray_host1x_device.dev,
		.platform_data = (void *)&stingray_display_data,
	},
};

void __init stingray_display_init(void)
{
	struct clk *pll_c;
	struct clk *disp1;
	int err;

	err = pinctrl_register_mappings(stingray_display_pinmux,
					ARRAY_SIZE(stingray_display_pinmux));
	if (err) {
		pr_err("stingray: display pinmux map registration failed: %d\n",
		       err);
		return;
	}

	pll_c = clk_get_sys(NULL, "pll_c");
	if (IS_ERR(pll_c)) {
		pr_err("stingray: PLLC unavailable for display\n");
		return;
	}
	err = clk_set_rate(pll_c, 598000000);
	if (err) {
		pr_err("stingray: PLLC rate setup failed: %d\n", err);
		clk_put(pll_c);
		return;
	}

	disp1 = clk_get_sys("tegradc.0", NULL);
	if (IS_ERR(disp1)) {
		pr_err("stingray: display clock unavailable\n");
		clk_put(pll_c);
		return;
	}
	err = clk_set_parent(disp1, pll_c);
	clk_put(disp1);
	clk_put(pll_c);
	if (err) {
		pr_err("stingray: display clock parent setup failed: %d\n", err);
		return;
	}

	err = platform_device_register(&stingray_panel_regulator_device);
	if (err) {
		pr_err("stingray: panel regulator registration failed: %d\n", err);
		return;
	}

	err = platform_device_register(&stingray_host1x_device);
	if (err) {
		pr_err("stingray: Host1x registration failed: %d\n", err);
		return;
	}

	err = platform_device_register(&stingray_display_device);
	if (err) {
		pr_err("stingray: display controller registration failed: %d\n",
		       err);
		return;
	}

	i2c_register_board_info(0, stingray_backlight_i2c_info,
				ARRAY_SIZE(stingray_backlight_i2c_info));
	err = platform_device_register(&stingray_i2c1_device);
	if (err)
		pr_err("stingray: I2C1 registration failed: %d\n", err);
}
