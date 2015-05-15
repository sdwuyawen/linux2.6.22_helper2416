/* linux/arch/arm/mach-s3c2416/mach-smdk2416.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * http://www.fluff.org/ben/smdk2443/
 *
 * Thanks to Samsung for the loan of an SMDK2416
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/setup.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-mem.h>

#include <asm/arch/idle.h>
#include <asm/arch/fb.h>

#include <asm/plat-s3c24xx/s3c2410.h>
#include <asm/plat-s3c24xx/s3c2440.h>
#include <asm/plat-s3c24xx/clock.h>
#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/plat-s3c24xx/common-smdk.h>

#include <asm/arch/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/hsmmc.h>

static struct map_desc smdk2416_iodesc[] __initdata = {
	//IODESC_ENT(CS8900),
};

#define UCON S3C2410_UCON_DEFAULT | S3C2440_UCON_FCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c24xx_uart_clksrc smdk2416_serial_clocks[] = {
	[0] = {
		.name		= "pclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},
	[1] = {
		.name		= "esysclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	}

};

#if 0	/* helper2416上原来是这个，但是有问题，换成了2.6.22自带的下面的内容 */
static struct s3c2410_uartcfg smdk2416_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		/* Use PCLK */
		.ucon	     = 0x3c5,
		//.ucon	     = 0xfc5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
		.clocks	     = smdk2416_serial_clocks,
		.clocks_size = ARRAY_SIZE(smdk2416_serial_clocks),
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
//		.ucon	     = 0xfc5,
		.ulcon	     = 0x03,
//		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
		.clocks	     = smdk2416_serial_clocks,
		.clocks_size = ARRAY_SIZE(smdk2416_serial_clocks),
	},
	/* IR port */
	[3] = {
		.hwport      = 3,
		.flags	     = 0,
		.ucon	     = 0x3c5,
//		.ucon	     = 0xfc5,
		.ulcon	     = 0x03,
//		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
		.clocks      = smdk2416_serial_clocks,
		.clocks_size = ARRAY_SIZE(smdk2416_serial_clocks),
	}
};
#endif

static struct s3c2410_uartcfg smdk2416_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	}
};


static struct platform_device *smdk2416_devices[] __initdata = {
	
	//&s3c_device_wdt,
	&s3c_device_i2c,
	&s3c_device_lcd,
	&s3c_device_rtc,
	&s3c_device_adc,
	&s3c_device_iis,
	&s3c_device_usbgadget,
	&s3c_device_usb,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_smc911x
};

static struct s3c24xx_board smdk2416_board __initdata = {
	.devices       = smdk2416_devices,
	.devices_count = ARRAY_SIZE(smdk2416_devices)
};

static void __init smdk2416_map_io(void)
{
	/********* add here *********/
	putstring_VA("smdk2416_map_io()\r\n");
	/********* end add *********/
	s3c24xx_init_io(smdk2416_iodesc, ARRAY_SIZE(smdk2416_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(smdk2416_uartcfgs, ARRAY_SIZE(smdk2416_uartcfgs));
	s3c24xx_set_board(&smdk2416_board);
	/********* add here *********/
	putstring_VA("end of smdk2416_map_io()\r\n");
	/********* end add *********/
}


static void smdk2416_cs89x0_set(void)
{
	u32 val;

	/********* add here *********/
	putstring_VA("smdk2416_cs89x0_set()\r\n");
	/********* end add *********/

	/* 设置EBICON，BANK1选择为SROM，而不是NAND */
	val = readl(S3C_BANK_CFG);				
	val &= ~((1<<8)|(1<<9)|(1<<10));
	writel(val, S3C_BANK_CFG);

/*
#define S3C24XX_VA_SROMC	S3C2410_ADDR(0x03700000)
#define S3C2443_PA_SROMC	(0x4F000000)
#define S3C24XX_SZ_SROMC	SZ_1M
*/
	/* Bank1 Idle cycle ctrl. */
	writel(0xf, S3C_SSMC_SMBIDCYR1);

	/* Bank1 Read Wait State cont. = 14 clk          Tacc? */
	/* WSTRD表示从nCS有效到nRD无效总长度，即nRD总长度，包含WSTOEN部分，WSTOEN必须<=WSTRD */
	writel(12, S3C_SSMC_SMBWSTRDR1);

	/* Bank1 Write Wait State ctrl. */
	/* WSTWR表示从nCS有效到nWE无效总长度，即nWR总长度，包含WSTWEN部分，WSTWEN必须<=WSTWR */
	writel(12, S3C_SSMC_SMBWSTWRR1);

	/* Bank1 Output Enable Assertion Delay ctrl.     Tcho? */
	/* nCS有效到nOE有效之间的间隔，和WSTRD配套使用 */
	writel(2, S3C_SSMC_SMBWSTOENR1);

	/* Bank1 Write Enable Assertion Delay ctrl. */
	/* nCS有效到nWE有效之间的间隔，和WSTWR配套使用 */
	writel(2, S3C_SSMC_SMBWSTWENR1);

	/* 在ADDR信号和nCS信号之间插入delay，数值由SMBCR中DELAYnCS确定 */
	val = readl(S3C_SSMC_SMBCR1);

	val |=  ((1<<15)|(1<<7));		/* .bit7 = 1, .bit15 = 1 */
//	val &=  ~((1<<15)|(1<<7));		/* .bit7 = 0, .bit15 = 0 */
	writel(val, S3C_SSMC_SMBCR1);

	/* bit2是使能nWait,=1表示使能
	 * =0时，WSTRD,WSTWR,WSTOEN,WSTWEN有效
	 * =1时，SMBCR.DRnOWE=1时
	 *             nCS到nOE的时间由WSTOEN确定，必须大于1
	 *             nCS到nWE的时间由WSTWEN确定，必须大于1
	 *       SMBCR.DRnCS=1时
	 *             ADDR Signal和nCS之间的时间由SMBCR.DELAYnCS确定
	 *       
	 */
	/* bit1配置nWait的极性，默认0表示低电平有效 */
	/* bit0是RBLE,=1表示在read时nBE[1:0]保持为0
	 * nBE[1:0]在板子上未连接
	 */
	val = readl(S3C_SSMC_SMBCR1);
	val |=  ((1<<2)|(1<<0));	/* .bit2 = 1, .bit0 = 1 */
//	val &=  ~(1<<2);			/* .bit2 = 0 */
//	val |=  (1<<0);			/* .bit0 = 1 */
	writel(val, S3C_SSMC_SMBCR1);

	/* SMAddrValid = always High when Read/Write
	 * RSMAVD保持为高
	 */
	val = readl(S3C_SSMC_SMBCR1);
	val &= ~((3<<20)|(3<<12));
	writel(val, S3C_SSMC_SMBCR1);

	/* 设置位宽,16 */
	val = readl(S3C_SSMC_SMBCR1);
	val &= ~(3<<4);
	writel(val, S3C_SSMC_SMBCR1);

	val = readl(S3C_SSMC_SMBCR1);
	val |= (1<<4);

	writel(val, S3C_SSMC_SMBCR1);

	/********* add here *********/
	putstring_VA("end of smdk2416_cs89x0_set()\r\n");
	/********* end add *********/

}

static void __init smdk2416_machine_init(void)
{
	/********* add here *********/
	putstring_VA("smdk2416_machine_init()\r\n");
	/********* end add *********/
	/* SROM init for NFS */
	smdk2416_cs89x0_set();

	/* 	
		这里可以添加platform_add_devices，用于注册设备
		platform_add_devices(smdk2440_devices, ARRAY_SIZE(smdk2440_devices)); 
	*/
	platform_add_devices(smdk2416_devices, ARRAY_SIZE(smdk2416_devices)); 
	
	smdk_machine_init();

	/********* add here *********/
	putstring_VA("end of smdk2416_machine_init()\r\n");
	/********* end add *********/
}

static void __init smdk2416_fixup (struct machine_desc *desc, struct tag *tags,
	      char **cmdline, struct meminfo *mi)
{
	/*
	 * Bank start addresses are not present in the information
	 * passed in from the boot loader.  We could potentially
	 * detect them, but instead we hard-code them.
	 */
	mi->bank[0].start = 0x30000000;

#if defined(CONFIG_VIDEO_SAMSUNG)
	mi->bank[0].size = 49*1024*1024;
#elif defined(CONFIG_PP_S3C2443)
	mi->bank[0].size = 60*1024*1024;
#else
	mi->bank[0].size = 64*1024*1024;
#endif
	mi->bank[0].node = 0;

	mi->nr_banks = 1;
}


MACHINE_START(SMDK2416, "SMDK2416")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= smdk2416_map_io,	/* 在arch/arm/mm/mmu.c中devicemaps_init()中被调用(setup_arch()->paging_init()->devicemaps_init()->mdesc->map_io()) */
	.fixup      	= smdk2416_fixup,
	.init_machine	= smdk2416_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END


/*
 * HS-MMC GPIO Set function for S3C2416 SMDK board
 */
void hsmmc_set_gpio (uint channel, uint width)
{
	switch (channel) {

	/* can supports 1 and 4 bit bus */
	case 0:
		/* GPIO E : Command, Clock */
		s3c2410_gpio_cfgpin(S3C2410_GPE5,  S3C2450_GPE5_SD0_CLK);
		s3c2410_gpio_cfgpin(S3C2410_GPE6, S3C2450_GPE6_SD0_CMD);

		if (width == 1) {
			/* GPIO E : MMC DATA0[0] */
			s3c2410_gpio_cfgpin(S3C2410_GPE7,  S3C2450_GPE7_SD0_DAT0);
		}
		else if (width == 4) {
			/* GPIO E : MMC DATA0[0:3] */
			s3c2410_gpio_cfgpin(S3C2410_GPE7,  S3C2450_GPE7_SD0_DAT0);
			s3c2410_gpio_cfgpin(S3C2410_GPE8,  S3C2450_GPE8_SD0_DAT1);
			s3c2410_gpio_cfgpin(S3C2410_GPE9,  S3C2450_GPE9_SD0_DAT2);
			s3c2410_gpio_cfgpin(S3C2410_GPE10,  S3C2450_GPE10_SD0_DAT3);
		}
		break;

	/* can supports 1, 4, and 8 bit bus */
	case 1:
		/* GPIO L : Command, Clock */
		s3c2410_gpio_cfgpin(S3C2443_GPL8, S3C2450_GPL8_SD1CMD);
		s3c2410_gpio_cfgpin(S3C2443_GPL9, S3C2450_GPL9_SD1CLK);

		/* GPIO J : Chip detect, LED, Write Protect */
		s3c2410_gpio_cfgpin(S3C2443_GPJ13, S3C2450_GPJ13_SD1LED);
		s3c2410_gpio_cfgpin(S3C2443_GPJ14, S3C2450_GPJ14_nSD1CD);

		s3c2410_gpio_cfgpin(S3C2443_GPJ15, S3C2450_GPJ15_nSD1WP); /* write protect enable */
		s3c2410_gpio_setpin(S3C2443_GPJ15, 1);

		if (width == 1) {
			/* GPIO L : MMC DATA1[0] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
		}
		else if (width == 4) {
			/* GPIO L : MMC DATA1[0:3] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
			s3c2410_gpio_cfgpin(S3C2443_GPL1, S3C2450_GPL1_SD1DAT1);
			s3c2410_gpio_cfgpin(S3C2443_GPL2, S3C2450_GPL2_SD1DAT2);
			s3c2410_gpio_cfgpin(S3C2443_GPL3, S3C2450_GPL3_SD1DAT3);
		}
		else if (width == 8) {
			/* GPIO L : MMC DATA1[0:7] */
			s3c2410_gpio_cfgpin(S3C2443_GPL0, S3C2450_GPL0_SD1DAT0);
			s3c2410_gpio_cfgpin(S3C2443_GPL1, S3C2450_GPL1_SD1DAT1);
			s3c2410_gpio_cfgpin(S3C2443_GPL2, S3C2450_GPL2_SD1DAT2);
			s3c2410_gpio_cfgpin(S3C2443_GPL3, S3C2450_GPL3_SD1DAT3);

			s3c2410_gpio_cfgpin(S3C2443_GPL4, S3C2450_GPL4_SD1DAT4);
			s3c2410_gpio_cfgpin(S3C2443_GPL5, S3C2450_GPL5_SD1DAT5);
			s3c2410_gpio_cfgpin(S3C2443_GPL6, S3C2450_GPL6_SD1DAT6);
			s3c2410_gpio_cfgpin(S3C2443_GPL7, S3C2450_GPL7_SD1DAT7);
		}
		break;

	default:
		break;
	}
}


#define HOST_CAPS (MMC_CAP_4_BIT_DATA | MMC_CAP_MULTIWRITE | \
			MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED)

/* Channel 0 : added HS-MMC channel */
struct s3c_hsmmc_cfg s3c_hsmmc0_platform = {
	.hwport = 0,
	.enabled = 1,
	.host_caps = HOST_CAPS,
	.bus_width = 4,
	.highspeed = 0,

	/* ctrl for mmc */
	.fd_ctrl[MMC_MODE_MMC] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for mmc */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	/* ctrl for sd */
	.fd_ctrl[MMC_MODE_SD] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for sd */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	.clk_name[0] = "hsmmc",				/* 1st clock source */
	.clk_name[1] = "esysclk",			/* 2nd clock source hsmmc-epll by Ben Dooks */
	.clk_name[2] = "hsmmc-ext",			/* 3rd clock source */
};

/* Channel 1 : default HS-MMC channel */
struct s3c_hsmmc_cfg s3c_hsmmc1_platform = {
	.hwport = 1,
	.enabled = 1,
	.host_caps = HOST_CAPS | MMC_CAP_8_BIT_DATA,
	.bus_width = 8,
	.highspeed = 0,

	/* ctrl for mmc */
	.fd_ctrl[MMC_MODE_MMC] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for mmc */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	/* ctrl for sd */
	.fd_ctrl[MMC_MODE_SD] = {
		.ctrl2 = 0xC0000100,			/* ctrl2 for sd */
		.ctrl3[SPEED_NORMAL] = 0,		/* ctrl3 for low speed */
		.ctrl3[SPEED_HIGH]   = 0,		/* ctrl3 for high speed */
		.ctrl4 = 0x3,
	},

	.clk_name[0] = "hsmmc",				/* 1st clock source */
	.clk_name[1] = "esysclk",			/* 2nd clock source hsmmc-epll by Ben Dooks */
	.clk_name[2] = "hsmmc-ext",			/* 3rd clock source */
};

