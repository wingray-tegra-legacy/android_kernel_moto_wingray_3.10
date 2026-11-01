#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/machine.h>
#include <linux/platform_data/tegra-sdhci.h>
#include <linux/platform_data/tegra-gpio.h>
#include <linux/platform_data/tegra-pinctrl.h>
#include <linux/sizes.h>

#include "board.h"
#include "gpio-names.h"
#include "iomap.h"

#define TEGRA_GIC_IRQ(n) ((n) + 32)
#define STINGRAY_PINCTRL_NAME "tegra20-pinctrl"

static struct resource stingray_gpio_resources[] = {
	{ .start = 0x6000d000, .end = 0x6000dfff, .flags = IORESOURCE_MEM },
	{ .start = TEGRA_GIC_IRQ(32), .end = TEGRA_GIC_IRQ(32),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(33), .end = TEGRA_GIC_IRQ(33),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(34), .end = TEGRA_GIC_IRQ(34),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(35), .end = TEGRA_GIC_IRQ(35),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(55), .end = TEGRA_GIC_IRQ(55),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(87), .end = TEGRA_GIC_IRQ(87),
	  .flags = IORESOURCE_IRQ },
	{ .start = TEGRA_GIC_IRQ(89), .end = TEGRA_GIC_IRQ(89),
	  .flags = IORESOURCE_IRQ },
};

static const struct tegra_gpio_platform_data stingray_gpio_data = {
	.bank_stride = 0x80,
	.upper_offset = 0x800,
};

static struct platform_device stingray_gpio_device = {
	.name = "tegra-gpio",
	.id = -1,
	.num_resources = ARRAY_SIZE(stingray_gpio_resources),
	.resource = stingray_gpio_resources,
	.dev.platform_data = (void *)&stingray_gpio_data,
};

static struct resource stingray_pinctrl_resources[] = {
	{ .start = TEGRA_APB_MISC_BASE + 0x14,
	  .end = TEGRA_APB_MISC_BASE + 0x7f,
	  .flags = IORESOURCE_MEM },
	{ .start = TEGRA_APB_MISC_BASE + 0x80,
	  .end = TEGRA_APB_MISC_BASE + 0x9f, .flags = IORESOURCE_MEM },
	{ .start = TEGRA_APB_MISC_BASE + 0xa0,
	  .end = TEGRA_APB_MISC_BASE + 0xbf, .flags = IORESOURCE_MEM },
	{ .start = TEGRA_APB_MISC_BASE + 0x868,
	  .end = TEGRA_APB_MISC_BASE + 0x8ff, .flags = IORESOURCE_MEM },
};

static struct platform_device stingray_pinctrl_device = {
	.name = STINGRAY_PINCTRL_NAME,
	.id = -1,
	.num_resources = ARRAY_SIZE(stingray_pinctrl_resources),
	.resource = stingray_pinctrl_resources,
};

static unsigned long stingray_pin_pullup_driven[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_PULL,
			   TEGRA_PINCONFIG_PULL_UP),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_TRISTATE,
			   TEGRA_PINCONFIG_DRIVEN),
};

static unsigned long stingray_pin_normal_driven[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_PULL,
			   TEGRA_PINCONFIG_PULL_NONE),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_TRISTATE,
			   TEGRA_PINCONFIG_DRIVEN),
};

static unsigned long stingray_pin_driven[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_TRISTATE,
			   TEGRA_PINCONFIG_DRIVEN),
};

static unsigned long stingray_sdmmc2_drive[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_HIGH_SPEED_MODE, 0),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SCHMITT, 1),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_LOW_POWER_MODE, 3),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_DRIVE_DOWN_STRENGTH, 31),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_DRIVE_UP_STRENGTH, 31),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SLEW_RATE_RISING, 3),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SLEW_RATE_FALLING, 3),
};

static unsigned long stingray_sdmmc3_drive[] = {
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_HIGH_SPEED_MODE, 0),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SCHMITT, 1),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_LOW_POWER_MODE, 3),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_DRIVE_DOWN_STRENGTH, 20),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_DRIVE_UP_STRENGTH, 16),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SLEW_RATE_RISING, 3),
	TEGRA_PINCONF_PACK(TEGRA_PINCONF_PARAM_SLEW_RATE_FALLING, 3),
};

static const struct pinctrl_map stingray_storage_pinmux[] = {
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.1", STINGRAY_PINCTRL_NAME,
				  "kbcb", "sdio2"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.1", STINGRAY_PINCTRL_NAME,
				       "kbcb", stingray_pin_pullup_driven),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.1", STINGRAY_PINCTRL_NAME,
				  "kbcd", "sdio2"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.1", STINGRAY_PINCTRL_NAME,
				       "kbcd", stingray_pin_pullup_driven),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.1", STINGRAY_PINCTRL_NAME,
				       "drive_sdmmc2", stingray_sdmmc2_drive),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				  "sdb", "sdio3"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				       "sdb", stingray_pin_driven),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				  "sdc", "sdio3"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				       "sdc", stingray_pin_driven),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				  "sdd", "sdio3"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				       "sdd", stingray_pin_driven),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.2", STINGRAY_PINCTRL_NAME,
				       "drive_sdmmc3", stingray_sdmmc3_drive),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				  "atb", "sdio4"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				       "atb", stingray_pin_pullup_driven),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				  "gma", "sdio4"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				       "gma", stingray_pin_normal_driven),
	PIN_MAP_MUX_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				  "gme", "sdio4"),
	PIN_MAP_CONFIGS_GROUP_DEFAULT("sdhci-tegra.3", STINGRAY_PINCTRL_NAME,
				       "gme", stingray_pin_normal_driven),
};

static const struct tegra_sdhci_platform_data stingray_sdmmc2_data = {
	.cd_gpio = -1,
	.bus_width = 4,
	.nonremovable = 1,
};

static const struct tegra_sdhci_platform_data stingray_sdmmc3_data = {
	.cd_gpio = TEGRA_GPIO_PI5,
	.bus_width = 4,
};

static const struct tegra_sdhci_platform_data stingray_sdmmc4_data = {
	.cd_gpio = -1,
	.bus_width = 8,
	.nonremovable = 1,
};

#define STINGRAY_SDMMC_DEVICE(_id, _base, _irq, _data) \
	{ \
		.name = "sdhci-tegra", \
		.id = (_id), \
		.num_resources = 2, \
		.resource = (struct resource[]) { \
			{ .start = (_base), .end = (_base) + SZ_512 - 1, \
			  .flags = IORESOURCE_MEM }, \
			{ .start = (_irq), .end = (_irq), \
			  .flags = IORESOURCE_IRQ }, \
		}, \
		.dev.platform_data = (void *)(_data), \
	}

static struct platform_device stingray_sdmmc2 =
	STINGRAY_SDMMC_DEVICE(1, TEGRA_SDMMC2_BASE,
			      TEGRA_GIC_IRQ(15), &stingray_sdmmc2_data);
static struct platform_device stingray_sdmmc3 =
	STINGRAY_SDMMC_DEVICE(2, TEGRA_SDMMC3_BASE,
			      TEGRA_GIC_IRQ(19), &stingray_sdmmc3_data);
static struct platform_device stingray_sdmmc4 =
	STINGRAY_SDMMC_DEVICE(3, TEGRA_SDMMC4_BASE,
			      TEGRA_GIC_IRQ(31), &stingray_sdmmc4_data);

void __init stingray_storage_init(void)
{
	int err;

	err = platform_device_register(&stingray_gpio_device);
	if (err) {
		pr_err("stingray: GPIO registration failed: %d\n", err);
		return;
	}

	err = pinctrl_register_mappings(stingray_storage_pinmux,
					ARRAY_SIZE(stingray_storage_pinmux));
	if (err) {
		pr_err("stingray: storage pinmux map registration failed: %d\n",
		       err);
		return;
	}

	err = platform_device_register(&stingray_pinctrl_device);
	if (err) {
		pr_err("stingray: pinctrl registration failed: %d\n", err);
		return;
	}

	err = platform_device_register(&stingray_sdmmc2);
	if (err)
		pr_err("stingray: SDMMC2 registration failed: %d\n", err);

	err = platform_device_register(&stingray_sdmmc3);
	if (err)
		pr_err("stingray: SDMMC3 registration failed: %d\n", err);

	err = platform_device_register(&stingray_sdmmc4);
	if (err)
		pr_err("stingray: SDMMC4 registration failed: %d\n", err);
}
