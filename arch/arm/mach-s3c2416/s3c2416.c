/* linux/arch/arm/mach-s3c2416/s3c2416.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * Samsung S3C2416 Mobile CPU support
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
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/clk.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/proc-fns.h>
#include <asm/arch/idle.h>

#include <asm/arch/regs-s3c2416-clock.h>
#include <asm/arch/reset.h>

#include <asm/plat-s3c24xx/s3c2416.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>
#include <asm/arch/regs-gpio.h>

#undef  DVS_IDLE

static struct map_desc s3c2416_iodesc[] __initdata = {
	IODESC_ENT(WATCHDOG),
	IODESC_ENT(CLKPWR),
	IODESC_ENT(TIMER),
	IODESC_ENT(LCD),
	IODESC_ENT(USBDEV),
	IODESC_ENT(CAMIF),
	IODESC_ENT(EBI),			/* 对应S3C24XX_VA_EBI, S3C24XX_PA_EBI, S3C24XX_SZ_EBI */
	IODESC_ENT(SROMC),
};

struct sysdev_class s3c2416_sysclass = {
	set_kset_name("s3c2416-core"),
	.suspend	= NULL,
	.resume	= NULL,
};

static struct sys_device s3c2416_sysdev = {
	.cls		= &s3c2416_sysclass,
};

static void s3c2416_hard_reset(void)
{
	printk("s3c2416_hard_reset\n");
	__raw_writel(S3C2443_SWRST_RESET, S3C2443_SWRST);
}

int __init s3c2416_init(void)
{
	printk("S3C2416: Initialising architecture\n");

	s3c24xx_reset_hook = s3c2416_hard_reset;

	/* 修改的内容 */
//	s3c_device_nand.name = "s3c2412-nand";

	// For S3C nand
//	s3c_device_nand.name = "s3c-nand";


	return sysdev_register(&s3c2416_sysdev);
}

#define   IDLE_PROBE
static void s3c2416_idle(void)
{
	unsigned long tmp;

	/* ensure our idle mode is to go to idle */
/*if you want to reduce CPU clock with idle */
	#ifdef DVS_IDLE
	tmp = __raw_readl(S3C2443_CLKDIV0);
	tmp &= ~(0x1<<13);
	tmp |= (0x1<<13);
	__raw_writel(tmp, S3C2443_CLKDIV0);
	#else
	tmp = __raw_readl(S3C2443_PWRMODE);
	tmp &= ~(0x1<<17);
	tmp |= (0x1<<17);
	__raw_writel(tmp, S3C2443_PWRMODE);
	#endif
	/* in SMDK2416 you can probe the idle status through the TP11(GPB2) by Laputa*/
#ifdef IDLE_PROBE
	tmp = __raw_readl(S3C2410_GPBDAT);
	tmp |= (0x1<<2);
	__raw_writel(tmp, S3C2410_GPBDAT);
	tmp &= ~(0x1<<2);
	__raw_writel(tmp, S3C2410_GPBDAT);
#endif

	cpu_do_idle();
}

void __init s3c2416_init_uarts(struct s3c2410_uartcfg *cfg, int no)
{
	s3c24xx_init_uartdevs("s3c2440-uart", s3c2410_uart_resources, cfg, no);

#if defined (CONFIG_S3C_SIR)
	s3c24xx_uart_src[3]->name = "s3c-irda";
#endif
	/* rename devices that are s3c2413/s3c2443/s3c6400 specific */
        s3c_device_lcd.name  = "s3c-lcd";
}

/* s3c2443_map_io
 *
 * register the standard cpu IO areas, and any passed in from the
 * machine specific initialisation.
 */

void __init s3c2416_map_io(struct map_desc *mach_desc, int mach_size)
{
	iotable_init(s3c2416_iodesc, ARRAY_SIZE(s3c2416_iodesc));
	iotable_init(mach_desc, mach_size);

	s3c24xx_idle = s3c2416_idle;

	/* 新增加的 */
	/* rename any peripherals used differing from the s3c2410 */

	s3c_device_i2c.name  = "s3c2440-i2c";
//	s3c_device_nand.name = "s3c2440-nand";	/* 选择要用的驱动?? */
	s3c_device_nand.name = "s3c-nand";	/* common-smdk.c中调用platform_add_devices注册平台设备 */
	s3c_device_usbgadget.name = "s3c2440-usbgadget";
	/* 新增加 结束 */
}

/* need to register class before we actually register the device, and
 * we also need to ensure that it has been initialised before any of the
 * drivers even try to use it (even if not on an s3c2443 based system)
 * as a driver which may support both 2443 and 2440 may try and use it.
*/

static int __init s3c2416_core_init(void)
{
	printk("s3c2416_core_init\n");
	return sysdev_class_register(&s3c2416_sysclass);
}

core_initcall(s3c2416_core_init);


#define CAMDIV_val     26

int s3c_camif_set_clock (unsigned int camclk)
{
	unsigned int camclk_div, val, hclkcon;
	struct clk *src_clk = clk_get(NULL, "hclk");

	if (camclk == 4800000) {
		printk(KERN_INFO "External camera clock is set to 48MHz\n");
	}
	else if (camclk > 48000000) {
		printk(KERN_ERR "Invalid camera clock\n");
	}

	writel(readl(S3C2443_CLKSRC) | (1 << 20), S3C2443_CLKSRC);

	camclk_div = clk_get_rate(src_clk) / camclk;
	printk("Parent clock = %ld, CAMDIV = %d\n", clk_get_rate(src_clk), camclk_div);

	// CAMIF HCLK Enable
	hclkcon = __raw_readl(S3C2443_HCLKCON);
	hclkcon |= S3C2443_HCLKCON_CAMIF;
	__raw_writel(hclkcon, S3C2443_HCLKCON);

	/* CAMCLK Enable */
	val = readl(S3C2443_SCLKCON);
	val |= S3C2443_SCLKCON_CAMCLK;
	writel(val, S3C2443_SCLKCON);

	val = readl(S3C2443_CLKDIV1);
	val &= ~(0xf<<CAMDIV_val);
	writel(val, S3C2443_CLKDIV1);

	val |= ((camclk_div -1) << CAMDIV_val);
	writel(val, S3C2443_CLKDIV1);
	val = readl(S3C2443_CLKDIV1);

	return 0;
}

void s3c_camif_disable_clock (void)
{
	unsigned int val;

	val = readl(S3C2443_SCLKCON);
	val &= ~S3C2443_SCLKCON_CAMCLK;
	writel(val, S3C2443_SCLKCON);
}



