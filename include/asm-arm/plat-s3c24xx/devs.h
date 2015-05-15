/* linux/include/asm-arm/plat-s3c24xx/devs.h
 *
 * Copyright (c) 2004 Simtec Electronics
 * Ben Dooks <ben@simtec.co.uk>
 *
 * Header file for s3c2410 standard platform devices
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/platform_device.h>

struct s3c24xx_uart_resources {
	struct resource		*resources;
	unsigned long		 nr_resources;
};

extern struct s3c24xx_uart_resources s3c2410_uart_resources[];

extern struct platform_device *s3c24xx_uart_devs[];
extern struct platform_device *s3c24xx_uart_src[];

extern struct platform_device s3c_device_usb;
extern struct platform_device s3c_device_lcd;
extern struct platform_device s3c_device_wdt;
extern struct platform_device s3c_device_i2c;
extern struct platform_device s3c_device_iis;
extern struct platform_device s3c_device_rtc;
extern struct platform_device s3c_device_adc;
extern struct platform_device s3c_device_sdi;
extern struct platform_device s3c_device_hsmmc;

extern struct platform_device s3c_device_spi0;
extern struct platform_device s3c_device_spi1;

extern struct platform_device s3c_device_nand;

extern struct platform_device s3c_device_timer0;
extern struct platform_device s3c_device_timer1;
extern struct platform_device s3c_device_timer2;
extern struct platform_device s3c_device_timer3;

extern struct platform_device s3c_device_usbgadget;

/* s3c2440 specific devices */

#ifdef CONFIG_CPU_S3C2440

extern struct platform_device s3c_device_camif;

#endif

/* 新增加的 */
#if defined(CONFIG_CPU_S3C2443)||defined(CONFIG_CPU_S3C2450)||defined(CONFIG_CPU_S3C2416)||defined(CONFIG_CPU_S3C6400)||defined(CONFIG_CPU_S3C6410) ||defined(CONFIG_CPU_S3C2416)
extern struct s3c_nand_mtd_info nand_mtd_info;
extern struct flash_platform_data s3c_onenand_data;
extern struct platform_device s3c_device_camif;
#endif

#if defined(CONFIG_CPU_S3C2443)
extern struct platform_device s3c_device_hsmmc;
#elif defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416)
extern struct platform_device s3c_device_hsmmc0;
extern struct platform_device s3c_device_hsmmc1;
extern struct platform_device s3c_device_ide;
#endif

#if defined(CONFIG_CPU_S3C6410) || defined(CONFIG_CPU_S3C2450)|| defined(CONFIG_CPU_S3C2416)
extern struct platform_device s3c_device_smc911x;
#endif
/* 新增加 结束 */

