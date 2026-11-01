#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/keyreset.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <asm/system_info.h>

#include "gpio-names.h"

#define STINGRAY_TOUCH_RESET_GPIO TEGRA_GPIO_PR4
#define STINGRAY_TOUCH_IRQ_GPIO_P1_P3 TEGRA_GPIO_PV2
#define STINGRAY_TOUCH_IRQ_GPIO_P2    TEGRA_GPIO_PZ2
#define STINGRAY_TOUCH_WAKE_GPIO  TEGRA_GPIO_PJ2
#define STINGRAY_REVISION_P2     (0x8200U << 16)

static bool stingray_touch_is_p2;

static int __init stingray_revision_setup(char *revision)
{
	stingray_touch_is_p2 = !strcmp(revision, "p2");
	return 1;
}
__setup("hw_rev=", stingray_revision_setup);

static struct mxt_platform_data stingray_touch_data = {
	.name = "qtouch-touchscreen",
	.reset_gpio = STINGRAY_TOUCH_RESET_GPIO,
	.wake_gpio = STINGRAY_TOUCH_WAKE_GPIO,
	.x_line = 0x21,
	.y_line = 0x2a,
	.x_size = 0x065f,
	.y_size = 0x0a15,
	.blen = 0x20,
	.threshold = 0x2d,
	.orient = MXT_DIAGONAL,
	.irqflags = IRQF_TRIGGER_LOW,
};

static struct i2c_board_info stingray_touch_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x5b),
		.platform_data = &stingray_touch_data,
	},
};

static struct gpio_keys_button stingray_keypad_buttons[] = {
	{
		.code = KEY_VOLUMEUP,
		.gpio = TEGRA_GPIO_PR0,
		.active_low = 1,
		.desc = "volume-up",
		.type = EV_KEY,
		.debounce_interval = 10,
	},
	{
		.code = KEY_VOLUMEDOWN,
		.gpio = TEGRA_GPIO_PR1,
		.active_low = 1,
		.desc = "volume-down",
		.type = EV_KEY,
		.debounce_interval = 10,
	},
};

static struct gpio_keys_platform_data stingray_keypad_data = {
	.buttons = stingray_keypad_buttons,
	.nbuttons = ARRAY_SIZE(stingray_keypad_buttons),
	.name = "stingray-keypad",
};

static struct platform_device stingray_keypad_device = {
	.name = "gpio-keys",
	.id = -1,
	.dev.platform_data = &stingray_keypad_data,
};

static int stingray_log_reset(void)
{
	pr_warn("stingray: hard reset buttons pushed\n");
	return 0;
}

static struct keyreset_platform_data stingray_reset_keys_data = {
	.reset_fn = stingray_log_reset,
	.keys_down = { KEY_END, KEY_VOLUMEUP, 0 },
};

static struct platform_device stingray_keyreset_device = {
	.name = KEYRESET_NAME,
	.id = -1,
	.dev.platform_data = &stingray_reset_keys_data,
};

void __init stingray_input_init(void)
{
	int err, irq;
	int touch_irq_gpio = stingray_touch_is_p2 ||
		(system_rev == STINGRAY_REVISION_P2) ?
		STINGRAY_TOUCH_IRQ_GPIO_P2 : STINGRAY_TOUCH_IRQ_GPIO_P1_P3;

	err = gpio_request_one(TEGRA_GPIO_PQ0, GPIOF_OUT_INIT_LOW,
			       "keypad-col");
	if (err)
		pr_err("stingray: keypad column GPIO request failed: %d\n", err);

	err = platform_device_register(&stingray_keyreset_device);
	if (err)
		pr_err("stingray: key reset registration failed: %d\n", err);

	err = platform_device_register(&stingray_keypad_device);
	if (err)
		pr_err("stingray: keypad registration failed: %d\n", err);

	err = gpio_request_one(touch_irq_gpio, GPIOF_IN,
			       "touch-irq");
	if (err) {
		pr_err("stingray: touch IRQ GPIO request failed: %d\n", err);
		return;
	}

	irq = gpio_to_irq(touch_irq_gpio);
	if (irq < 0) {
		pr_err("stingray: touch IRQ mapping failed: %d\n", irq);
		gpio_free(touch_irq_gpio);
		return;
	}

	stingray_touch_i2c_info[0].irq = irq;

	/* Register board info before the Tegra I2C controller is created. */
	i2c_register_board_info(0, stingray_touch_i2c_info,
				ARRAY_SIZE(stingray_touch_i2c_info));
}
