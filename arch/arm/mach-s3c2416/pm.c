/* linux/arch/arm/mach-s3c2450/pm.c
 *
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * http://armlinux.simtec.co.uk/.
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
#include <linux/sysdev.h>
#include <linux/platform_device.h>

#include <asm/cacheflush.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-power.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-irq.h>
#include <asm/arch/regs-dsc.h>
#include <asm/arch/regs-s3c2416-clock.h>

#include <asm/plat-s3c24xx/cpu.h>
#include <asm/plat-s3c24xx/pm.h>

//#include <asm/plat-s3c24xx/s3c2450.h>

static void s3c2450_cpu_suspend(void)
{
	//unsigned long tmp;

	/* USB Physical power */
	__raw_writel(__raw_readl(S3C2443_PHYPWR) | 0xf, S3C2443_PHYPWR);

	/* USB Suspend mode */
	__raw_writel(__raw_readl(S3C2410_MISCCR)|(1<<12)|(1<<13), S3C2410_MISCCR);

	__raw_writel(0xffffffff, S3C2410_INTMSK);
	__raw_writel(__raw_readl(S3C2410_SRCPND), S3C2410_SRCPND);
	__raw_writel(__raw_readl(S3C2410_INTPND), S3C2410_INTPND);

	__raw_writel(0xffffffff, S3C2410_EINTPEND);
	__raw_writel(0xffffffff, S3C2410_EINTMASK);
	__raw_writel(0xffffffff, S3C2410_SRCPND);
	__raw_writel(0xffffffff, S3C2410_INTPND);

	__raw_writel(0xff80, S3C2443_RSTCON);
	__raw_writel(0xffff, S3C2443_OSCSET);
	/* ack any outstanding external interrupts before we go to sleep */

	__raw_writel((0xffffffff & ~((1<<0) | (1<<3) | (1<<30))) , S3C2410_INTMSK);

	__raw_writel(__raw_readl(S3C2410_EINTPEND), S3C2410_EINTPEND);
	__raw_writel(__raw_readl(S3C2410_INTPND), S3C2410_INTPND);
	__raw_writel(__raw_readl(S3C2410_SRCPND), S3C2410_SRCPND);

	__raw_writel( (1<<15), S3C2443_PWRCFG);

	/* set our standby method to sleep */
	__raw_writel(0x2BED, S3C2443_PWRMODE);
}

static void s3c2450_pm_prepare(void)
{
	int ret;

	/* set flag to wake up */
	__raw_writel(0x2BED, S3C2443_INFORM0);

        /* memory auto-detect configuration for u-boot side wake-up */
        ret = __raw_readl(S3C2410_BWSCON) & S3C2410_MEMCFG_MASK;

        if(ret ==  S3C2410_MEMCFG_MDDR)
                __raw_writel(0xDD, S3C2443_INFORM2);  /* mDDR */
	else if(ret == S3C2410_MEMCFG_MSDR)
                __raw_writel(0xDA, S3C2443_INFORM2);  /* mSDRAM */
	else if(ret == S3C2410_MEMCFG_DDR2)
		panic("DDR2 is not supported at u-boot");
        else
		panic("memory setup is not correct?");
		

}

static int s3c2450_pm_add(struct sys_device *sysdev)
{
	pm_cpu_prep = s3c2450_pm_prepare;
	pm_cpu_sleep = s3c2450_cpu_suspend;

	return 0;
}

static struct sleep_save s3c2450_sleep[] = {
//	SAVE_ITEM(S3C2450_DSC0),
//	SAVE_ITEM(S3C2450_DSC1),
//	SAVE_ITEM(S3C2413_GPJDAT),
//	SAVE_ITEM(S2C2413_GPJCON),
//	SAVE_ITEM(S3C2413_GPJUP),

	/* save the PWRCFG to get back to original sleep method */

//	SAVE_ITEM(S3C2443_PWRMODE),

	/* save the sleep configuration anyway, just in case these
	 * get damaged during wakeup */

//	SAVE_ITEM(S3C2450_GPBSLPCON),
//	SAVE_ITEM(S3C2450_GPCSLPCON),
//	SAVE_ITEM(S3C2450_GPDSLPCON),
//	SAVE_ITEM(S3C2450_GPESLPCON),
//	SAVE_ITEM(S3C2450_GPFSLPCON),
//	SAVE_ITEM(S3C2450_GPGSLPCON),
//	SAVE_ITEM(S3C2450_GPHSLPCON),
//	SAVE_ITEM(S3C2413_GPJSLPCON),
};

static int s3c2450_pm_suspend(struct sys_device *dev, pm_message_t state)
{
	s3c2410_pm_do_save(s3c2450_sleep, ARRAY_SIZE(s3c2450_sleep));
	return 0;
}

static int s3c2450_pm_resume(struct sys_device *dev)
{
//	unsigned long tmp;

//	tmp = __raw_readl(S3C2443_RSTCON);
//	tmp |= (1<<16);
//	__raw_writel(tmp, S3C2443_RSTCON);
//	__raw_writel(0, S3C2443_PWRMODE);

	__raw_writel(0x0, S3C2443_INFORM0);
	__raw_writel(0x0, S3C2443_INFORM2); 
	s3c2410_pm_do_restore(s3c2450_sleep, ARRAY_SIZE(s3c2450_sleep));
	return 0;
}

static struct sysdev_driver s3c2450_pm_driver = {
	.add		= s3c2450_pm_add,
	.suspend	= s3c2450_pm_suspend,
	.resume		= s3c2450_pm_resume,
};

static __init int s3c2450_pm_init(void)
{
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2450_pm_driver);
}

arch_initcall(s3c2450_pm_init);
