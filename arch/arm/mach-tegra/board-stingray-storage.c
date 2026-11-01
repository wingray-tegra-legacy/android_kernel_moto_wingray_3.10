#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/platform_data/tegra-sdhci.h>
#include <linux/sizes.h>

#include "board.h"
#include "gpio-names.h"
#include "iomap.h"

#define TEGRA_GIC_IRQ(n) ((n) + 32)

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
