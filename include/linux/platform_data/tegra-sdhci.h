#ifndef _LINUX_PLATFORM_DATA_TEGRA_SDHCI_H
#define _LINUX_PLATFORM_DATA_TEGRA_SDHCI_H

struct tegra_sdhci_platform_data {
	int cd_gpio;
	unsigned int bus_width;
	unsigned int nonremovable:1;
};

#endif
