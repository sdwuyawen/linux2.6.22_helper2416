/* linux/arch/arm/plat-s3c24xx/s3c244x.c
 *
 * Copyright (c) 2004-2006 Simtec Electronics
 *   Ben Dooks <ben@simtec.co.uk>
 *
 * Samsung S3C2440 and S3C2442 Mobile CPU support (not S3C2443)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/clk.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/arch/regs-clock.h>
#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/regs-dsc.h>

#include <asm/plat-s3c24xx/s3c2410.h>
#include <asm/plat-s3c24xx/s3c2440.h>
#include "s3c244x.h"
#include <asm/plat-s3c24xx/clock.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>
#include <asm/plat-s3c24xx/pm.h>

static struct map_desc s3c244x_iodesc[] __initdata = {
	IODESC_ENT(CLKPWR),
	IODESC_ENT(TIMER),
	IODESC_ENT(WATCHDOG),
	IODESC_ENT(LCD),
};

/* uart initialisation */

void __init s3c244x_init_uarts(struct s3c2410_uartcfg *cfg, int no)
{
	s3c24xx_init_uartdevs("s3c2440-uart", s3c2410_uart_resources, cfg, no);
}

void __init s3c244x_map_io(struct map_desc *mach_desc, int size)
{
	/* register our io-tables */

	iotable_init(s3c244x_iodesc, ARRAY_SIZE(s3c244x_iodesc));	/* ���������ַӳ�� */
	iotable_init(mach_desc, size);

	/* rename any peripherals used differing from the s3c2410 */

	s3c_device_i2c.name  = "s3c2440-i2c";
	s3c_device_nand.name = "s3c2440-nand";
	s3c_device_usbgadget.name = "s3c2440-usbgadget";
}

void __init s3c244x_init_clocks(int xtal)
{
	unsigned long clkdiv;
	unsigned long camdiv;
	unsigned long hclk, fclk, pclk;
	int hdiv = 1;

	/* now we've got our machine bits initialised, work out what
	 * clocks we've got */

	fclk = s3c2410_get_pll(__raw_readl(S3C2410_MPLLCON), xtal) * 2;

	clkdiv = __raw_readl(S3C2410_CLKDIVN);
	camdiv = __raw_readl(S3C2440_CAMDIVN);

	/* work out clock scalings */

	switch (clkdiv & S3C2440_CLKDIVN_HDIVN_MASK) {
	case S3C2440_CLKDIVN_HDIVN_1:
		hdiv = 1;
		break;

	case S3C2440_CLKDIVN_HDIVN_2:
		hdiv = 2;
		break;

	case S3C2440_CLKDIVN_HDIVN_4_8:
		hdiv = (camdiv & S3C2440_CAMDIVN_HCLK4_HALF) ? 8 : 4;
		break;

	case S3C2440_CLKDIVN_HDIVN_3_6:
		hdiv = (camdiv & S3C2440_CAMDIVN_HCLK3_HALF) ? 6 : 3;
		break;
	}

	hclk = fclk / hdiv;
	pclk = hclk / ((clkdiv & S3C2440_CLKDIVN_PDIVN)? 2:1);

	/* print brief summary of clocks, etc */

	printk("S3C244X: core %ld.%03ld MHz, memory %ld.%03ld MHz, peripheral %ld.%03ld MHz\n",
	       print_mhz(fclk), print_mhz(hclk), print_mhz(pclk));

	/* initialise the clocks here, to allow other things like the
	 * console to use them, and to add new ones after the initialisation
	 */

	s3c24xx_setup_clocks(xtal, fclk, hclk, pclk);
	s3c2410_baseclk_add();
}

#ifdef CONFIG_PM

static struct sleep_save s3c244x_sleep[] = {
	SAVE_ITEM(S3C2440_DSC0),
	SAVE_ITEM(S3C2440_DSC1),
	SAVE_ITEM(S3C2440_GPJDAT),
	SAVE_ITEM(S3C2440_GPJCON),
	SAVE_ITEM(S3C2440_GPJUP)
};

static int s3c244x_suspend(struct sys_device *dev, pm_message_t state)
{
	s3c2410_pm_do_save(s3c244x_sleep, ARRAY_SIZE(s3c244x_sleep));
	return 0;
}

static int s3c244x_resume(struct sys_device *dev)
{
	s3c2410_pm_do_restore(s3c244x_sleep, ARRAY_SIZE(s3c244x_sleep));
	return 0;
}

#else
#define s3c244x_suspend NULL
#define s3c244x_resume  NULL
#endif

/* Since the S3C2442 and S3C2440 share  items, put both sysclasses here */

struct sysdev_class s3c2440_sysclass = {
	set_kset_name("s3c2440-core"),
	.suspend	= s3c244x_suspend,
	.resume		= s3c244x_resume
};

struct sysdev_class s3c2442_sysclass = {
	set_kset_name("s3c2442-core"),
	.suspend	= s3c244x_suspend,
	.resume		= s3c244x_resume
};

/* need to register class before we actually register the device, and
 * we also need to ensure that it has been initialised before any of the
 * drivers even try to use it (even if not on an s3c2440 based system)
 * as a driver which may support both 2410 and 2440 may try and use it.
*/

static int __init s3c2440_core_init(void)
{
	return sysdev_class_register(&s3c2440_sysclass);
}

core_initcall(s3c2440_core_init);

static int __init s3c2442_core_init(void)
{
	return sysdev_class_register(&s3c2442_sysclass);
}

core_initcall(s3c2442_core_init);


#if 0
/* �����ӵ�S3C2416�Ĳ��֣�����ƶ���arch/arm/mach-s3c2416���� */

void __init s3c2416_init_clocks(int xtal)
{
	unsigned long epllcon = __raw_readl(S3C2443_EPLLCON);
	unsigned long mpllcon = __raw_readl(S3C2443_MPLLCON);
	unsigned long clkdiv0 = __raw_readl(S3C2443_CLKDIV0);
	unsigned long pll;
	unsigned long fclk;
	unsigned long hclk;
	unsigned long pclk;
	struct clk *clkp;
	int ret;
	int ptr;

	pll = s3c2443_get_mpll(mpllcon, xtal);

	fclk = pll / s3c2443_fclk_div(clkdiv0);
	/* bug fix */
	/* hclk = fclk / s3c2443_get_prediv(clkdiv0); */
	hclk = pll / s3c2443_get_prediv(clkdiv0);
	hclk = hclk / ((clkdiv0 & S3C2443_CLKDIV0_HALF_HCLK) ? 2 : 1);
 	pclk = hclk / ((clkdiv0 & S3C2443_CLKDIV0_HALF_PCLK) ? 2 : 1);

	s3c24xx_setup_clocks(xtal, fclk, hclk, pclk);

	printk("S3C2416: mpll %s %ld.%03ld MHz, cpu %ld.%03ld MHz, mem %ld.%03ld MHz, pclk %ld.%03ld MHz\n",
	       (mpllcon & S3C2443_PLLCON_OFF) ? "off":"on",
	       print_mhz(pll), print_mhz(fclk),
	       print_mhz(hclk), print_mhz(pclk));

	s3c2443_clk_initparents();

	for (ptr = 0; ptr < ARRAY_SIZE(clks); ptr++) {
		clkp = clks[ptr];

		ret = s3c24xx_register_clock(clkp);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
			       clkp->name, ret);
		}
	}

	clk_epll.rate = s3c2443_get_epll(epllcon, xtal);

	clk_usb_bus.parent = &clk_usb_bus_host;

	/* ensure usb bus clock is within correct rate of 48MHz */

	if (clk_get_rate(&clk_usb_bus_host) != (48 * 1000 * 1000)) {
		printk(KERN_INFO "Warning: USB host bus not at 48MHz\n");
		clk_set_rate(&clk_usb_bus_host, 48*1000*1000);
	}

	printk("S3C2416: epll %s %ld.%03ld MHz, usb-bus %ld.%03ld MHz\n",
	       (epllcon & S3C2443_PLLCON_OFF) ? "off":"on",
	       print_mhz(clk_get_rate(&clk_epll)),
	       print_mhz(clk_get_rate(&clk_usb_bus)));

	/* register clocks from clock array */

	clkp = init_clocks;
	for (ptr = 0; ptr < ARRAY_SIZE(init_clocks); ptr++, clkp++) {
		ret = s3c24xx_register_clock(clkp);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
			       clkp->name, ret);
		}
	}

	/* We must be careful disabling the clocks we are not intending to
	 * be using at boot time, as subsytems such as the LCD which do
	 * their own DMA requests to the bus can cause the system to lockup
	 * if they where in the middle of requesting bus access.
	 *
	 * Disabling the LCD clock if the LCD is active is very dangerous,
	 * and therefore the bootloader should be careful to not enable
	 * the LCD clock if it is not needed.
	*/

	/* install (and disable) the clocks we do not need immediately */

	clkp = init_clocks_disable;
	for (ptr = 0; ptr < ARRAY_SIZE(init_clocks_disable); ptr++, clkp++) {

		ret = s3c24xx_register_clock(clkp);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
			       clkp->name, ret);
		}

		(clkp->enable)(clkp, 0);
	}
}

/* �����ӵĽ��� */
#endif
