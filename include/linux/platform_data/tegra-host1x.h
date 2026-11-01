#ifndef _LINUX_PLATFORM_DATA_TEGRA_HOST1X_H
#define _LINUX_PLATFORM_DATA_TEGRA_HOST1X_H

struct tegra_dc_legacy_mode {
	unsigned long pixelclock;
	unsigned int h_ref_to_sync;
	unsigned int v_ref_to_sync;
	unsigned int hsync_len;
	unsigned int vsync_len;
	unsigned int hback_porch;
	unsigned int vback_porch;
	unsigned int hactive;
	unsigned int vactive;
	unsigned int hfront_porch;
	unsigned int vfront_porch;
};

struct tegra_dc_platform_data {
	const struct tegra_dc_legacy_mode *mode;
	unsigned int width_mm;
	unsigned int height_mm;
};

#endif
