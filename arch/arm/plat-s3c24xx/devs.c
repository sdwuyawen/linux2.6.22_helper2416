/* linux/arch/arm/plat-s3c24xx/devs.c
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Base S3C24XX platform device definitions
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
#include <asm/arch/fb.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/udc.h>

#include <asm/plat-s3c24xx/devs.h>
#include <asm/plat-s3c24xx/cpu.h>
#include <asm/arch/regs-spi.h>

#include <asm/arch/spi.h>
#include <asm/arch/spi-gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <asm/arch/regs-gpio.h>
#include <linux/dm9000.h>

/* Serial port registrations */

static struct resource s3c2410_uart0_resource[] = {
	[0] = {
		.start = S3C2410_PA_UART0,
		.end   = S3C2410_PA_UART0 + 0x3fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3CUART_RX0,
		.end   = IRQ_S3CUART_ERR0,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c2410_uart1_resource[] = {
	[0] = {
		.start = S3C2410_PA_UART1,
		.end   = S3C2410_PA_UART1 + 0x3fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3CUART_RX1,
		.end   = IRQ_S3CUART_ERR1,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c2410_uart2_resource[] = {
	[0] = {
		.start = S3C2410_PA_UART2,
		.end   = S3C2410_PA_UART2 + 0x3fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3CUART_RX2,
		.end   = IRQ_S3CUART_ERR2,
		.flags = IORESOURCE_IRQ,
	}
};

struct s3c24xx_uart_resources s3c2410_uart_resources[] __initdata = {
	[0] = {
		.resources	= s3c2410_uart0_resource,
		.nr_resources	= ARRAY_SIZE(s3c2410_uart0_resource),
	},
	[1] = {
		.resources	= s3c2410_uart1_resource,
		.nr_resources	= ARRAY_SIZE(s3c2410_uart1_resource),
	},
	[2] = {
		.resources	= s3c2410_uart2_resource,
		.nr_resources	= ARRAY_SIZE(s3c2410_uart2_resource),
	},
};

/* yart devices */

static struct platform_device s3c24xx_uart_device0 = {
	.id		= 0,
};

static struct platform_device s3c24xx_uart_device1 = {
	.id		= 1,
};

static struct platform_device s3c24xx_uart_device2 = {
	.id		= 2,
};

struct platform_device *s3c24xx_uart_src[3] = {
	&s3c24xx_uart_device0,
	&s3c24xx_uart_device1,
	&s3c24xx_uart_device2,
};

struct platform_device *s3c24xx_uart_devs[3] = {
};

/* USB Host Controller */

static struct resource s3c_usb_resource[] = {
	[0] = {
		.start = S3C24XX_PA_USBHOST,
		.end   = S3C24XX_PA_USBHOST + S3C24XX_SZ_USBHOST - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_USBH,
		.end   = IRQ_USBH,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_usb_dmamask = 0xffffffffUL;

struct platform_device s3c_device_usb = {
	.name		  = "s3c2410-ohci",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_usb_resource),
	.resource	  = s3c_usb_resource,
	.dev              = {
		.dma_mask = &s3c_device_usb_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_usb);

/* LCD Controller */

static struct resource s3c_lcd_resource[] = {
	[0] = {
		.start = S3C24XX_PA_LCD,
		.end   = S3C24XX_PA_LCD + S3C24XX_SZ_LCD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD,
		.end   = IRQ_LCD,
		.flags = IORESOURCE_IRQ,
	}

};

static u64 s3c_device_lcd_dmamask = 0xffffffffUL;

struct platform_device s3c_device_lcd = {
	.name		  = "s3c2410-lcd",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_lcd_resource),
	.resource	  = s3c_lcd_resource,
	.dev              = {
		.dma_mask		= &s3c_device_lcd_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_lcd);

void __init s3c24xx_fb_set_platdata(struct s3c2410fb_mach_info *pd)
{
	struct s3c2410fb_mach_info *npd;

	npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	if (npd) {
		memcpy(npd, pd, sizeof(*npd));
		s3c_device_lcd.dev.platform_data = npd;
	} else {
		printk(KERN_ERR "no memory for LCD platform data\n");
	}
}

/* NAND Controller */

static struct resource s3c_nand_resource[] = {
	[0] = {
		.start = S3C2410_PA_NAND,
		.end   = S3C2410_PA_NAND + S3C24XX_SZ_NAND - 1,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_nand = {
	.name		  = "s3c2410-nand",	/* 这个名字用于和drivers/mtd/nand/s3c2410.c中platform_driver结构中的名字匹配 */
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_nand_resource),
	.resource	  = s3c_nand_resource,
};

EXPORT_SYMBOL(s3c_device_nand);

/* USB Device (Gadget)*/

static struct resource s3c_usbgadget_resource[] = {
	[0] = {
		.start = S3C24XX_PA_USBDEV,
		.end   = S3C24XX_PA_USBDEV + S3C24XX_SZ_USBDEV - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_USBD,
		.end   = IRQ_USBD,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_usbgadget = {
	.name		  = "s3c2410-usbgadget",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_usbgadget_resource),
	.resource	  = s3c_usbgadget_resource,
};

EXPORT_SYMBOL(s3c_device_usbgadget);

void __init s3c24xx_udc_set_platdata(struct s3c2410_udc_mach_info *pd)
{
	struct s3c2410_udc_mach_info *npd;

	npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	if (npd) {
		memcpy(npd, pd, sizeof(*npd));
		s3c_device_usbgadget.dev.platform_data = npd;
	} else {
		printk(KERN_ERR "no memory for udc platform data\n");
	}
}


/* Watchdog */

static struct resource s3c_wdt_resource[] = {
	[0] = {
		.start = S3C24XX_PA_WATCHDOG,
		.end   = S3C24XX_PA_WATCHDOG + S3C24XX_SZ_WATCHDOG - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_WDT,
		.end   = IRQ_WDT,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_wdt = {
	.name		  = "s3c2410-wdt",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_wdt_resource),
	.resource	  = s3c_wdt_resource,
};

EXPORT_SYMBOL(s3c_device_wdt);

/* I2C */

static struct resource s3c_i2c_resource[] = {
	[0] = {
		.start = S3C24XX_PA_IIC,
		.end   = S3C24XX_PA_IIC + S3C24XX_SZ_IIC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IIC,
		.end   = IRQ_IIC,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_i2c = {
	.name		  = "s3c2410-i2c",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_i2c_resource),
	.resource	  = s3c_i2c_resource,
};

EXPORT_SYMBOL(s3c_device_i2c);

/* IIS */

static struct resource s3c_iis_resource[] = {
	[0] = {
		.start = S3C24XX_PA_IIS,
		.end   = S3C24XX_PA_IIS + S3C24XX_SZ_IIS -1,
		.flags = IORESOURCE_MEM,
	}
};

static u64 s3c_device_iis_dmamask = 0xffffffffUL;

struct platform_device s3c_device_iis = {
	.name		  = "s3c2410-iis",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_iis_resource),
	.resource	  = s3c_iis_resource,
	.dev              = {
		.dma_mask = &s3c_device_iis_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_iis);

/* RTC */

static struct resource s3c_rtc_resource[] = {
	[0] = {
		.start = S3C24XX_PA_RTC,
		.end   = S3C24XX_PA_RTC + 0xff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_RTC,
		.end   = IRQ_RTC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_TICK,
		.end   = IRQ_TICK,
		.flags = IORESOURCE_IRQ
	}
};

struct platform_device s3c_device_rtc = {
	.name		  = "s3c2410-rtc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_rtc_resource),
	.resource	  = s3c_rtc_resource,
};

EXPORT_SYMBOL(s3c_device_rtc);

/* ADC */

static struct resource s3c_adc_resource[] = {
	[0] = {
		.start = S3C24XX_PA_ADC,
		.end   = S3C24XX_PA_ADC + S3C24XX_SZ_ADC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TC,
		.end   = IRQ_TC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_ADC,
		.end   = IRQ_ADC,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_adc = {
	.name		  = "s3c2410-adc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_adc_resource),
	.resource	  = s3c_adc_resource,
};

/* SDI */

static struct resource s3c_sdi_resource[] = {
	[0] = {
		.start = S3C2410_PA_SDI,
		.end   = S3C2410_PA_SDI + S3C24XX_SZ_SDI - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI,
		.end   = IRQ_SDI,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_sdi = {
	.name		  = "s3c2410-sdi",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_sdi_resource),
	.resource	  = s3c_sdi_resource,
};

EXPORT_SYMBOL(s3c_device_sdi);

/* High-speed MMC/SD */
#if 0
static struct resource s3c_hsmmc_resource[] = {
	[0] = {
		.start = S3C2443_PA_HSMMC,
		.end   = S3C2443_PA_HSMMC + S3C2443_SZ_HSMMC - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3C2443_HSMMC,
		.end   = IRQ_S3C2443_HSMMC,
		.flags = IORESOURCE_IRQ,
	}
};
#endif	/* 0 */

/* 新增加的 */
static struct resource s3c_hsmmc_resource[] = {
	[0] = {
		.start = S3C_PA_HSMMC,
		.end   = S3C_PA_HSMMC+ S3C_SZ_HSMMC,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI_1,
		.end   = IRQ_SDI_1,
		.flags = IORESOURCE_IRQ,
	}
};
/* 新增加的 结束 */

static u64 s3c_device_hsmmc_dmamask = 0xffffffffUL;

struct platform_device s3c_device_hsmmc = {
	.name		  = "s3c-sdhci",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_hsmmc_resource),
	.resource	  = s3c_hsmmc_resource,
	.dev              = {
		.dma_mask = &s3c_device_hsmmc_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};



/* SPI (0) */

static struct resource s3c_spi0_resource[] = {
	[0] = {
		.start = S3C24XX_PA_SPI,
		.end   = S3C24XX_PA_SPI + 0x1f,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	}

};

static u64 s3c_device_spi0_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi0 = {
	.name		  = "s3c2410-spi",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_spi0_resource),
	.resource	  = s3c_spi0_resource,
        .dev              = {
                .dma_mask = &s3c_device_spi0_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_spi0);

/* SPI (1) */

static struct resource s3c_spi1_resource[] = {
	[0] = {
		.start = S3C24XX_PA_SPI + S3C2410_SPI1,
		.end   = S3C24XX_PA_SPI + S3C2410_SPI1 + 0x1f,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	}

};

static u64 s3c_device_spi1_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi1 = {
	.name		  = "s3c2410-spi",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(s3c_spi1_resource),
	.resource	  = s3c_spi1_resource,
        .dev              = {
                .dma_mask = &s3c_device_spi1_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_spi1);


/* GPIO SPI */

static void spi_gpio_cs(struct s3c2410_spigpio_info *spi, int pin, int cs)
{
	switch (cs) {
	case BITBANG_CS_ACTIVE:
		s3c2410_gpio_setpin(pin, 0);
		break;
	case BITBANG_CS_INACTIVE:
		s3c2410_gpio_setpin(pin, 1);
		break;
	}
}

/* spi_device的信息 */
static struct spi_board_info spigpio_info_helper2416[] = 
{
	{
	    	 .modalias = "spigpio_flash",  						/* 对应的spi_driver名字也是"spigpio_tft" */
	    	 .max_speed_hz = 8000000,						/* max spi clock (SCK) speed in HZ */
	    	 .bus_num = 0,    						 		/* jz2440里OLED接在SPI CONTROLLER 1 */
	    	 .mode    = SPI_MODE_0,
//	    	 .chip_select   = S3C2410_GPH10, 				/* oled_cs, 它的含义由spi_master确定 */
		 .chip_select   = S3C2410_GPH10, 			/* s3c2410_spigpio_probe()
		 											->spi_new_device() 
													{
														snprintf(proxy->dev.bus_id, sizeof proxy->dev.bus_id,
															"%s.%u", master->cdev.class_id,
															chip->chip_select);
													}	
												会用到chip_select。所以一个spi_master下的两个不同的spi_device的
												chip_select必须不同，否则会注册失败。
																
		 											*/
	    	 .platform_data = NULL , 						/* 它在spi_driver里使用 */    	 
	},
	{
	    	 .modalias = "spigpio_tft",  						/* 对应的spi_driver名字也是"spigpio_tft" */
	    	 .max_speed_hz = 8000000,						/* max spi clock (SCK) speed in HZ */
	    	 .bus_num = 0,    						 		/* jz2440里OLED接在SPI CONTROLLER 1 */
	    	 .mode    = SPI_MODE_0,
	    	 .chip_select   = 1001, 				/* oled_cs, 它的含义由spi_master确定 */
	    	 .platform_data = (const void *)S3C2410_GPF7 , 	/* oled_dc, 它在spi_driver里使用 */    	 
	 },
	 
};

#if 0
/* spi tft用的接线 */
static struct s3c2410_spigpio_info spi_gpio_cfg = {
	.pin_clk	= S3C2410_GPF5,
	.pin_mosi	= S3C2410_GPG3,
	.pin_miso	= S3C2410_GPG1,
	.chip_select	= &spi_gpio_cs,
	.board_size = 2,
	.board_info = spigpio_info_helper2416,
};
#endif

/* spi flash用的接线 */
static struct s3c2410_spigpio_info spi_gpio_cfg = {
	.pin_clk	= S3C2410_GPF5,
	.pin_mosi	= S3C2410_GPG3,
	.pin_miso	= S3C2410_GPF7,
	.chip_select	= &spi_gpio_cs,
	.board_size = 2,
	.board_info = spigpio_info_helper2416,
};


struct platform_device helper2416_gpiospi = {
	.name		  = "s3c24xx-spi-gpio",					/* 平台设备的名字和平台驱动的名字一致 */
	.id		  = 1,
	.dev = {
		.platform_data = &spi_gpio_cfg,
	},
};

EXPORT_SYMBOL(helper2416_gpiospi);


/* HSPI */

/* spi_device的信息 */
static struct spi_board_info hspi_info_helper2416[] = 
{
	{
	    	 .modalias = "hspi_mcp2515",  						/* 对应的spi_driver名字也是"spigpio_tft" */
	    	 .max_speed_hz = 8000000,						/* max spi clock (SCK) speed in HZ */
	    	 .bus_num = 0,    						 		/* jz2440里OLED接在SPI CONTROLLER 1 */
	    	 .mode    = SPI_MODE_0,
												/* oled_cs, 它的含义由spi_master确定 */
//		 .chip_select   = S3C2410_GPL13, 	
		 .chip_select   = 0, 						/* master查表转换 */
		 										/* s3c2410_spigpio_probe()
		 											->spi_new_device() 
													{
														snprintf(proxy->dev.bus_id, sizeof proxy->dev.bus_id,
															"%s.%u", master->cdev.class_id,
															chip->chip_select);
													}	
												会用到chip_select。所以一个spi_master下的两个不同的spi_device的
												chip_select必须不同，否则会注册失败。
																
		 											*/
	    	 .platform_data = (void *)S3C2410_GPH9 , 						/* 它在spi_driver里使用 */    	 
	},
	{
	    	 .modalias = "hspi_flash",  						/* 对应的spi_driver名字也是"spigpio_tft" */
	    	 .max_speed_hz = 10000000,						/* max spi clock (SCK) speed in HZ */
	    	 .bus_num = 0,    						 		/* jz2440里OLED接在SPI CONTROLLER 1 */
	    	 .mode    = SPI_MODE_0,
												
		 .chip_select   = 1, 						/* master查表转换 */
		 										/* s3c2410_spigpio_probe()
		 											->spi_new_device() 
													{
														snprintf(proxy->dev.bus_id, sizeof proxy->dev.bus_id,
															"%s.%u", master->cdev.class_id,
															chip->chip_select);
													}	
												会用到chip_select。所以一个spi_master下的两个不同的spi_device的
												chip_select必须不同，否则会注册失败。
																
		 											*/
	    	 .platform_data = 0 , 						/* 它在spi_driver里使用 */    	 
	},
	{
		.modalias = "hspi_stm32",  						/* 对应的spi_driver名字也是"hspi_stm32" */
		.max_speed_hz = 500000,						/* max spi clock (SCK) speed in HZ */
		.bus_num = 0,    						 		/* jz2440里OLED接在SPI CONTROLLER 1 */
		.mode    = SPI_MODE_0,

		.chip_select   = 2, 						/* master查表转换 */
												/* s3c2410_spigpio_probe()
												   ->spi_new_device() 
												   {
													   snprintf(proxy->dev.bus_id, sizeof proxy->dev.bus_id,
													   "%s.%u", master->cdev.class_id,
													   chip->chip_select);
												   }	
												   会用到chip_select。所以一个spi_master下的两个不同的spi_device的
												   chip_select必须不同，否则会注册失败。

												*/
		.platform_data = 0 , 						/* 它在spi_driver里使用 */    	 
	},
};


static struct s3c2416_hspi_info hspi_cfg = {
	.board_size = 3,
	.board_info = hspi_info_helper2416,
	.ss_talbes = {
		S3C2410_GPL13,
		S3C2410_GPH10,
		S3C2410_GPL13,
		0,
		0,
		0
	},
};

static struct resource s3c_hspi0_resource[] = {
	[0] = {
		.start = S3C24XX_PA_SPI,
		.end   = S3C24XX_PA_SPI + 0x1f,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	}

};

static u64 s3c_device_hspi0_dmamask = 0xffffffffUL;

struct platform_device s3c_device_hspi0 = {
	.name		  = "s3c2416-hspi",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_hspi0_resource),
	.resource	  = s3c_hspi0_resource,
        .dev              = {
                .dma_mask = &s3c_device_hspi0_dmamask,
                .coherent_dma_mask = 0xffffffffUL,
                .platform_data = &hspi_cfg,
        }
};
EXPORT_SYMBOL(s3c_device_hspi0);

/* pwm timer blocks */

static struct resource s3c_timer0_resource[] = {
	[0] = {
		.start = S3C24XX_PA_TIMER + 0x0C,
		.end   = S3C24XX_PA_TIMER + 0x0C + 0xB,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TIMER0,
		.end   = IRQ_TIMER0,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_timer0 = {
	.name		  = "s3c2410-timer",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_timer0_resource),
	.resource	  = s3c_timer0_resource,
};

EXPORT_SYMBOL(s3c_device_timer0);

/* timer 1 */

static struct resource s3c_timer1_resource[] = {
	[0] = {
		.start = S3C24XX_PA_TIMER + 0x18,
		.end   = S3C24XX_PA_TIMER + 0x23,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TIMER1,
		.end   = IRQ_TIMER1,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_timer1 = {
	.name		  = "s3c2410-timer",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(s3c_timer1_resource),
	.resource	  = s3c_timer1_resource,
};

EXPORT_SYMBOL(s3c_device_timer1);

/* timer 2 */

static struct resource s3c_timer2_resource[] = {
	[0] = {
		.start = S3C24XX_PA_TIMER + 0x24,
		.end   = S3C24XX_PA_TIMER + 0x2F,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TIMER2,
		.end   = IRQ_TIMER2,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_timer2 = {
	.name		  = "s3c2410-timer",
	.id		  = 2,
	.num_resources	  = ARRAY_SIZE(s3c_timer2_resource),
	.resource	  = s3c_timer2_resource,
};

EXPORT_SYMBOL(s3c_device_timer2);

/* timer 3 */

static struct resource s3c_timer3_resource[] = {
	[0] = {
		.start = S3C24XX_PA_TIMER + 0x30,
		.end   = S3C24XX_PA_TIMER + 0x3B,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TIMER3,
		.end   = IRQ_TIMER3,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_timer3 = {
	.name		  = "s3c2410-timer",
	.id		  = 3,
	.num_resources	  = ARRAY_SIZE(s3c_timer3_resource),
	.resource	  = s3c_timer3_resource,
};

EXPORT_SYMBOL(s3c_device_timer3);

/* 新删除的 */
#if 0
#ifdef CONFIG_CPU_S3C2440

/* Camif Controller */

static struct resource s3c_camif_resource[] = {
	[0] = {
		.start = S3C2440_PA_CAMIF,
		.end   = S3C2440_PA_CAMIF + S3C2440_SZ_CAMIF - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CAM,
		.end   = IRQ_CAM,
		.flags = IORESOURCE_IRQ,
	}

};

static u64 s3c_device_camif_dmamask = 0xffffffffUL;

struct platform_device s3c_device_camif = {
	.name		  = "s3c2440-camif",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_camif_resource),
	.resource	  = s3c_camif_resource,
	.dev              = {
		.dma_mask = &s3c_device_camif_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_camif);

#endif // CONFIG_CPU_S32440

#endif 

/* 新增加的 */

//#if !defined(CONFIG_CPU_S3C2440) && (defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)  || defined(CONFIG_CPU_S3C2416))
#if defined(CONFIG_CPU_S3C2443) || defined(CONFIG_CPU_S3C2450)  || defined(CONFIG_CPU_S3C2416)
/* Camif Controller */

static struct resource s3c_camif_resource[] = {
	[0] = {
		.start = S3C2443_PA_CAMIF,
		.end   = S3C2443_PA_CAMIF + S3C2443_SZ_CAMIF - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_S3C2440_CAM_C,
		.end   = IRQ_S3C2440_CAM_C,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_S3C2440_CAM_P,
		.end   = IRQ_S3C2440_CAM_P,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_camif_dmamask = 0xffffffffUL;

struct platform_device s3c_device_camif = {
	.name		  = "s3c-camif",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_camif_resource),
	.resource	  = s3c_camif_resource,
	.dev              = {
		.dma_mask = &s3c_device_camif_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_camif);


static struct resource s3c_ide_resource[] = {
	[0] = {
		.start = S3C_PA_CFATA,
		.end   = S3C_PA_CFATA+ S3C_SZ_CFATA,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CFCON,
		.end   = IRQ_CFCON,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_ide = {
	.name		  = "s3c-ide",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_ide_resource),
	.resource	  = s3c_ide_resource,
};

EXPORT_SYMBOL(s3c_device_ide);

#if defined(CONFIG_CPU_S3C2450) || defined(CONFIG_CPU_S3C2416) 
/* HS-MMC Controller */
extern struct s3c_hsmmc_cfg s3c_hsmmc0_platform;
extern struct s3c_hsmmc_cfg s3c_hsmmc1_platform;

static struct resource s3c_hsmmc0_resource[] = {
	[0] = {
		.start = S3C_PA_HSMMC+0x400000,
		.end   = S3C_PA_HSMMC+0x400000+S3C_SZ_HSMMC,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI_0,
		.end   = IRQ_SDI_0,
		.flags = IORESOURCE_IRQ,
	},
	/* To detect a card inserted, use an external interrupt */
	[2] = {
		.start = IRQ_EINT1,
		.end   = IRQ_EINT1,
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource s3c_hsmmc1_resource[] = {
	[0] = {
		.start = S3C_PA_HSMMC,
		.end   = S3C_PA_HSMMC+S3C_SZ_HSMMC,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI_1,
		.end   = IRQ_SDI_1,
		.flags = IORESOURCE_IRQ,
	},
	/* To detect a card inserted, use an external interrupt */ // YX_MODI add for card detect interrupt
	[2] = {
		.start = IRQ_EINT8,
		.end   = IRQ_EINT8,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_hsmmc0 = {
	.name		  = "s3c-hsmmc",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_hsmmc0_resource),
	.resource	  = s3c_hsmmc0_resource,
	.dev		= {
		.platform_data = &s3c_hsmmc0_platform,
	}
};


struct platform_device s3c_device_hsmmc1 = {
	.name		  = "s3c-hsmmc",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(s3c_hsmmc1_resource),
	.resource	  = s3c_hsmmc1_resource,
	.dev		= {
		.platform_data = &s3c_hsmmc1_platform,
	}
};

EXPORT_SYMBOL(s3c_device_hsmmc0);
EXPORT_SYMBOL(s3c_device_hsmmc1);

#else 
/* HS-MMC controller */
extern struct s3c_hsmmc_cfg s3c_hsmmc_platform;

static struct resource s3c_hsmmc_resource[] = {
	[0] = {
		.start = S3C_PA_HSMMC,
		.end   = S3C_PA_HSMMC+ S3C_SZ_HSMMC,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDI_1,
		.end   = IRQ_SDI_1,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_hsmmc = {
	.name		  = "s3c-hsmmc",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c_hsmmc_resource),
	.resource	  = s3c_hsmmc_resource,
	.dev		= {
		.platform_data = &s3c_hsmmc_platform,
	}
};

EXPORT_SYMBOL(s3c_device_hsmmc);
#endif

#endif // CONFIG_CPU_S32443


#if defined (CONFIG_CPU_S3C6410) || defined (CONFIG_CPU_S3C2450)|| defined (CONFIG_CPU_S3C2416)
static struct resource s3c_smc911x_resources[] = {
      [0] = {
              .start  = S3C_PA_SMC9115,
              .end    = S3C_PA_SMC9115 + 0x02000000,
              .flags  = IORESOURCE_MEM,
      },
      [1] = {
#if defined(CONFIG_CPU_S3C6410)
              .start = IRQ_EINT10,
              .end   = IRQ_EINT10,
#elif defined(CONFIG_CPU_S3C2450)|| defined (CONFIG_CPU_S3C2416)
              .start = IRQ_EINT15,
              .end   = IRQ_EINT15,
#endif
              .flags = IORESOURCE_IRQ,
      },
};

struct platform_device s3c_device_smc911x = {
      .name           = "smc911x",
      .id             =  -1,
      .num_resources  = ARRAY_SIZE(s3c_smc911x_resources),
      .resource       = s3c_smc911x_resources,
}; 

EXPORT_SYMBOL(s3c_device_smc911x);
#endif

/* 新增加结束 */

/* 新增加DM9000 */
static struct resource dm9000_resources[] = {  
    [0] = {  
        .start  = S3C24XX_PA_DM9000,  
        .end    = S3C24XX_PA_DM9000+0x00,  
        .flags  = IORESOURCE_MEM,  
    },  
    [1] = {  
        .start  = S3C24XX_PA_DM9000+0x01,  
        .end    = S3C24XX_PA_DM9000+S3C24XX_SZ_DM9000-1,  
        .flags  = IORESOURCE_MEM,  
    },  
    [2] = {  
        .start  = IRQ_EINT11,  
        .end    = IRQ_EINT11,  
//      .flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,   
        .flags  = IORESOURCE_IRQ | IRQF_TRIGGER_LOW,  
    },  
};  

static struct dm9000_plat_data dm9000_setup = {  
    .flags          = DM9000_PLATF_8BITONLY  
};  

struct platform_device s3c_device_dm9000 = {  
    .name           = "dm9000",  
    .id             = 0,  
    .num_resources  = ARRAY_SIZE(dm9000_resources),  
    .resource       = dm9000_resources,  
    .dev            =   
    {  
        .platform_data = &dm9000_setup,  
    }  
};  
EXPORT_SYMBOL(s3c_device_dm9000);  

/* 新增加DM9000结束 */

