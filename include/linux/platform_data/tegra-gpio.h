#ifndef _LINUX_PLATFORM_DATA_TEGRA_GPIO_H
#define _LINUX_PLATFORM_DATA_TEGRA_GPIO_H

#include <linux/types.h>

struct tegra_gpio_platform_data {
	u32 bank_stride;
	u32 upper_offset;
};

#endif
