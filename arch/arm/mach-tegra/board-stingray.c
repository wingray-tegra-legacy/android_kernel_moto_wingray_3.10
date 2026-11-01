#include <linux/clk.h>
#include <linux/clk/tegra.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/sizes.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include "board.h"
#include "irq.h"
#include "iomap.h"
#include "pmc.h"

#define STINGRAY_UARTB_IRQ 69

static struct plat_serial8250_port stingray_uart_ports[] = {
	{
		.mapbase = TEGRA_UARTB_BASE,
		.irq = STINGRAY_UARTB_IRQ,
		.uartclk = 26000000,
		.regshift = 2,
		.iotype = UPIO_MEM32,
		.flags = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_IOREMAP,
		.type = PORT_TEGRA,
	},
	{},
};

static struct platform_device stingray_uart_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = stingray_uart_ports,
	},
};

static int __init stingray_remove_top_mem(phys_addr_t size)
{
	phys_addr_t end = memblock_end_of_DRAM();

	if (end < size)
		return -EINVAL;

	return memblock_remove(end - size, size);
}

static void __init stingray_reserve(void)
{
	if (memblock_reserve(0, SZ_4K))
		pr_warn("stingray: unable to reserve first page\n");

	if (memblock_remove(SZ_512M - SZ_2M, SZ_2M))
		pr_warn("stingray: unable to remove legacy console area\n");

	if (stingray_remove_top_mem(SZ_256M)) {
		pr_err("stingray: unable to reserve top memory carveout\n");
		return;
	}
	if (stingray_remove_top_mem(SZ_16M)) {
		pr_err("stingray: unable to reserve secondary framebuffer\n");
		return;
	}
	if (stingray_remove_top_mem(SZ_8M))
		pr_err("stingray: unable to reserve framebuffer\n");
}

static void __init stingray_init_irq(void)
{
	tegra_clocks_init();
	tegra20_clock_init_legacy();
	tegra_pmc_init();
	tegra_init_irq();
	tegra_legacy_irq_syscore_init();
}

static void __init stingray_init_machine(void)
{
	struct clk *uartb;
	struct clk *clkm;
	int err;

	tegra_clocks_apply_init_table();
	uartb = clk_get_sys("tegra_uart.1", NULL);
	if (IS_ERR(uartb)) {
		pr_err("stingray: UARTB clock unavailable\n");
	} else {
		clkm = clk_get_sys(NULL, "clk_m");
		if (IS_ERR(clkm))
			err = PTR_ERR(clkm);
		else {
			err = clk_set_parent(uartb, clkm);
			clk_put(clkm);
		}
		if (!err)
			err = clk_set_rate(uartb, 26000000);
		if (!err)
			err = clk_prepare_enable(uartb);
		if (err)
			pr_err("stingray: UARTB clock setup failed: %d\n", err);
		else
			pr_info("stingray: legacy machine ID %d, UARTB enabled\n",
				MACH_TYPE_STINGRAY);
		clk_put(uartb);
	}

	err = platform_device_register(&stingray_uart_device);
	if (err)
		pr_err("stingray: UARTB registration failed: %d\n", err);

	stingray_storage_init();
}

MACHINE_START(STINGRAY, "Motorola Xoom Wingray/Stingray")
	.atag_offset	= 0x00000100,
	.map_io		= tegra_map_common_io,
	.reserve	= stingray_reserve,
	.init_early	= tegra_init_early,
	.init_irq	= stingray_init_irq,
	.init_time	= tegra20_timer_init_legacy,
	.init_machine	= stingray_init_machine,
	.restart	= tegra_assert_system_reset,
MACHINE_END
