#if 0
/* linux/arch/arm/mach-s3c2416/dma.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * S3C2443 DMA selection
 *
 * http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>

#include <asm/dma.h>
#include <asm/arch/dma.h>
#include <asm/io.h>

#include <asm/plat-s3c24xx/dma.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-ac97.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-sdi.h>
#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-spi.h>

#define MAP(x) { \
		[0]	= (x) | DMA_CH_VALID,	\
		[1]	= (x) | DMA_CH_VALID,	\
		[2]	= (x) | DMA_CH_VALID,	\
		[3]	= (x) | DMA_CH_VALID,	\
		[4]	= (x) | DMA_CH_VALID,	\
		[5]     = (x) | DMA_CH_VALID,	\
		[6]	= (x) | DMA_CH_VALID,	\
		[7]     = (x) | DMA_CH_VALID,	\
	}

static struct s3c24xx_dma_map __initdata s3c2416_dma_mappings[] = {
	[DMACH_XD0] = {
		.name		= "xdreq0",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ0),
	},
	[DMACH_XD1] = {
		.name		= "xdreq1",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ1),
	},
	[DMACH_SDI] = {
		.name		= "sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_SDI),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_SPI_TX] = {
		.name		= "spi-tx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.from	= S3C_PA_SPI_0 + S3C_SPI_TX_DATA,
	},
	[DMACH_SPI_RX] = {
		.name		= "spi-rx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0RX),
		.hw_addr.to	= S3C_PA_SPI_0 + S3C_SPI_RX_DATA,
	},
	[DMACH_SPI0] = {
		.name		= "spi0",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.to	= S3C2410_PA_SPI + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + S3C2410_SPRDAT,
	},
	[DMACH_SPI1] = {
		.name		= "spi1",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI1TX),
		.hw_addr.to	= S3C2410_PA_SPI + 0x20 + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + 0x20 + S3C2410_SPRDAT,
	},
	[DMACH_UART0] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_0),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_0),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_0),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_0),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_UART0_SRC2] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_1),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1_SRC2] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_1),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2_SRC2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_1),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3_SRC2] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_1),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_TIMER] = {
		.name		= "timer",
		.channels	= MAP(S3C2443_DMAREQSEL_TIMER),
	},
	[DMACH_I2S_IN] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_I2SRX),
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2443_DMAREQSEL_I2STX),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_I2S_IN_1] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2450_DMAREQSEL_I2SRX_1),
		.hw_addr.from	= S3C2450_PA_IIS_1 + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT_1] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2450_DMAREQSEL_I2STX_1),
		.hw_addr.to	= S3C2450_PA_IIS_1 + S3C2410_IISFIFO,
	},
	[DMACH_PCM_IN] = {
		.name		= "pcm-in",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_PCM_OUT] = {
		.name		= "pcm-out",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMOUT),
		.hw_addr.to	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_MIC_IN] = {
		.name		= "mic-in",
		.channels	= MAP(S3C2443_DMAREQSEL_MICIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_MIC_DATA,
	},
};

static void s3c2416_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c24xx_dma_map *map)
{
	writel(map->channels[0] | S3C2443_DMAREQSEL_HW,
	       chan->regs + S3C2443_DMA_DMAREQSEL);
}

static struct s3c24xx_dma_selection __initdata s3c2416_dma_sel = {
	.select		= s3c2416_dma_select,
	.dcon_mask	= 0,
	.map		= s3c2416_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c2416_dma_mappings),
};

static int __init s3c2416_dma_add(struct sys_device *sysdev)
{
	s3c24xx_dma_init(8, IRQ_S3C2443_DMA0, 0x100);
	return s3c24xx_dma_init_map(&s3c2416_dma_sel);
}

static struct sysdev_driver s3c2416_dma_driver __initdata = {
	.add	= s3c2416_dma_add,
};

static int __init s3c2416_dma_init(void)
{
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2416_dma_driver);
}

//arch_initcall(s3c2416_dma_init);

#endif 


#if 0
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

static int s3c2416_test_add(struct sys_device *sysdev)
{
	return 0;
}



static int s3c2416_test_suspend(struct sys_device *dev, pm_message_t state)
{
	return 0;
}

static int s3c2416_test_resume(struct sys_device *dev)
{
	return 0;
}

static struct sysdev_driver s3c2416_test_driver = {
	.add		= s3c2416_test_add,
	.suspend	= s3c2416_test_suspend,
	.resume		= s3c2416_test_resume,
};

static __init int s3c2416_test_init(void)
{
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2416_test_driver);
}

arch_initcall(s3c2416_test_init);

#endif


#if 0
/* linux/arch/arm/mach-s3c2416/dma.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * S3C2443 DMA selection
 *
 * http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>

#include <asm/dma.h>
#include <asm/arch/dma.h>
#include <asm/io.h>

#include <asm/plat-s3c24xx/dma.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-ac97.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-sdi.h>
#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-spi.h>


#define MAP(x) { \
		[0]	= (x) | DMA_CH_VALID,	\
		[1]	= (x) | DMA_CH_VALID,	\
		[2]	= (x) | DMA_CH_VALID,	\
		[3]	= (x) | DMA_CH_VALID,	\
		[4]	= (x) | DMA_CH_VALID,	\
		[5]     = (x) | DMA_CH_VALID,	\
		[6]	= (x) | DMA_CH_VALID,	\
		[7]     = (x) | DMA_CH_VALID,	\
	}

static struct s3c24xx_dma_map __initdata s3c2416_dma_mappings[] = {
	[DMACH_XD0] = {
		.name		= "xdreq0",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ0),
	},
	[DMACH_XD1] = {
		.name		= "xdreq1",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ1),
	},
	[DMACH_SDI] = {
		.name		= "sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_SDI),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_SPI_TX] = {
		.name		= "spi-tx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.from	= S3C_PA_SPI_0 + S3C_SPI_TX_DATA,
	},
	[DMACH_SPI_RX] = {
		.name		= "spi-rx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0RX),
		.hw_addr.to	= S3C_PA_SPI_0 + S3C_SPI_RX_DATA,
	},
	[DMACH_SPI0] = {
		.name		= "spi0",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.to	= S3C2410_PA_SPI + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + S3C2410_SPRDAT,
	},
	[DMACH_SPI1] = {
		.name		= "spi1",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI1TX),
		.hw_addr.to	= S3C2410_PA_SPI + 0x20 + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + 0x20 + S3C2410_SPRDAT,
	},
	[DMACH_UART0] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_0),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_0),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_0),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_0),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_UART0_SRC2] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_1),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1_SRC2] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_1),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2_SRC2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_1),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3_SRC2] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_1),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_TIMER] = {
		.name		= "timer",
		.channels	= MAP(S3C2443_DMAREQSEL_TIMER),
	},
	[DMACH_I2S_IN] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_I2SRX),
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2443_DMAREQSEL_I2STX),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_I2S_IN_1] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2450_DMAREQSEL_I2SRX_1),
		.hw_addr.from	= S3C2450_PA_IIS_1 + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT_1] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2450_DMAREQSEL_I2STX_1),
		.hw_addr.to	= S3C2450_PA_IIS_1 + S3C2410_IISFIFO,
	},
	[DMACH_PCM_IN] = {
		.name		= "pcm-in",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_PCM_OUT] = {
		.name		= "pcm-out",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMOUT),
		.hw_addr.to	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_MIC_IN] = {
		.name		= "mic-in",
		.channels	= MAP(S3C2443_DMAREQSEL_MICIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_MIC_DATA,
	},
};

static void s3c2416_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c24xx_dma_map *map)
{
	writel(map->channels[0] | S3C2443_DMAREQSEL_HW,
	       chan->regs + S3C2443_DMA_DMAREQSEL);
}


static struct s3c24xx_dma_selection __initdata s3c2416_dma_sel = {
	.select		= s3c2416_dma_select,
	.dcon_mask	= 0,
	.map		= s3c2416_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c2416_dma_mappings),
};


static int s3c2416_test_add(struct sys_device *sysdev)
{
	s3c24xx_dma_init(8, IRQ_S3C2443_DMA0, 0x100);
	return s3c24xx_dma_init_map(&s3c2416_dma_sel);
}


static int s3c2416_test_suspend(struct sys_device *dev, pm_message_t state)
{
	return 0;
}

static int s3c2416_test_resume(struct sys_device *dev)
{
	return 0;
}

static struct sysdev_driver s3c2416_test_driver = {
	.add		= s3c2416_test_add,
	.suspend	= s3c2416_test_suspend,
	.resume		= s3c2416_test_resume,
};

static __init int s3c2416_dma_init(void)
{
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2416_test_driver);
}

arch_initcall(s3c2416_dma_init);

#endif



/* linux/arch/arm/mach-s3c2416/dma.c
 *
 * Copyright (c) 2007 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 * S3C2443 DMA selection
 *
 * http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>

#include <asm/dma.h>
#include <asm/arch/dma.h>
#include <asm/io.h>

#include <asm/plat-s3c24xx/dma.h>
#include <asm/plat-s3c24xx/cpu.h>

#include <asm/arch/regs-serial.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-ac97.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-sdi.h>
#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-spi.h>

#define MAP(x) { \
		[0]	= (x) | DMA_CH_VALID,	\
		[1]	= (x) | DMA_CH_VALID,	\
		[2]	= (x) | DMA_CH_VALID,	\
		[3]	= (x) | DMA_CH_VALID,	\
		[4]	= (x) | DMA_CH_VALID,	\
		[5]     = (x) | DMA_CH_VALID,	\
		[6]	= (x) | DMA_CH_VALID,	\
		[7]     = (x) | DMA_CH_VALID,	\
	}

static struct s3c24xx_dma_map __initdata s3c2416_dma_mappings[] = {
	[DMACH_XD0] = {
		.name		= "xdreq0",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ0),
	},
	[DMACH_XD1] = {
		.name		= "xdreq1",
		.channels	= MAP(S3C2443_DMAREQSEL_XDREQ1),
	},
	[DMACH_SDI] = {
		.name		= "sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_SDI),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_SPI_TX] = {
		.name		= "spi-tx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.from	= S3C_PA_SPI_0 + S3C_SPI_TX_DATA,
	},
	[DMACH_SPI_RX] = {
		.name		= "spi-rx",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0RX),
		.hw_addr.to	= S3C_PA_SPI_0 + S3C_SPI_RX_DATA,
	},
	[DMACH_SPI0] = {
		.name		= "spi0",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI0TX),
		.hw_addr.to	= S3C2410_PA_SPI + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + S3C2410_SPRDAT,
	},
	[DMACH_SPI1] = {
		.name		= "spi1",
		.channels	= MAP(S3C2443_DMAREQSEL_SPI1TX),
		.hw_addr.to	= S3C2410_PA_SPI + 0x20 + S3C2410_SPTDAT,
		.hw_addr.from	= S3C2410_PA_SPI + 0x20 + S3C2410_SPRDAT,
	},
	[DMACH_UART0] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_0),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_0),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_0),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_0),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_UART0_SRC2] = {
		.name		= "uart0",
		.channels	= MAP(S3C2443_DMAREQSEL_UART0_1),
		.hw_addr.to	= S3C2410_PA_UART0 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART0 + S3C2410_URXH,
	},
	[DMACH_UART1_SRC2] = {
		.name		= "uart1",
		.channels	= MAP(S3C2443_DMAREQSEL_UART1_1),
		.hw_addr.to	= S3C2410_PA_UART1 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART1 + S3C2410_URXH,
	},
      	[DMACH_UART2_SRC2] = {
		.name		= "uart2",
		.channels	= MAP(S3C2443_DMAREQSEL_UART2_1),
		.hw_addr.to	= S3C2410_PA_UART2 + S3C2410_UTXH,
		.hw_addr.from	= S3C2410_PA_UART2 + S3C2410_URXH,
	},
      	[DMACH_UART3_SRC2] = {
		.name		= "uart3",
		.channels	= MAP(S3C2443_DMAREQSEL_UART3_1),
		.hw_addr.to	= S3C2443_PA_UART3 + S3C2410_UTXH,
		.hw_addr.from	= S3C2443_PA_UART3 + S3C2410_URXH,
	},
	[DMACH_TIMER] = {
		.name		= "timer",
		.channels	= MAP(S3C2443_DMAREQSEL_TIMER),
	},
	[DMACH_I2S_IN] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2443_DMAREQSEL_I2SRX),
		.hw_addr.from	= S3C2410_PA_IIS + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2443_DMAREQSEL_I2STX),
		.hw_addr.to	= S3C2410_PA_IIS + S3C2410_IISFIFO,
	},
	[DMACH_I2S_IN_1] = {
		.name		= "i2s-sdi",
		.channels	= MAP(S3C2450_DMAREQSEL_I2SRX_1),
		.hw_addr.from	= S3C2450_PA_IIS_1 + S3C2410_IISFIFORX,
	},
	[DMACH_I2S_OUT_1] = {
		.name		= "i2s-sdo",
		.channels	= MAP(S3C2450_DMAREQSEL_I2STX_1),
		.hw_addr.to	= S3C2450_PA_IIS_1 + S3C2410_IISFIFO,
	},
	[DMACH_PCM_IN] = {
		.name		= "pcm-in",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_PCM_OUT] = {
		.name		= "pcm-out",
		.channels	= MAP(S3C2443_DMAREQSEL_PCMOUT),
		.hw_addr.to	= S3C2440_PA_AC97 + S3C_AC97_PCM_DATA,
	},
	[DMACH_MIC_IN] = {
		.name		= "mic-in",
		.channels	= MAP(S3C2443_DMAREQSEL_MICIN),
		.hw_addr.from	= S3C2440_PA_AC97 + S3C_AC97_MIC_DATA,
	},
};

static void s3c2416_dma_select(struct s3c2410_dma_chan *chan,
			       struct s3c24xx_dma_map *map)
{
	writel(map->channels[0] | S3C2443_DMAREQSEL_HW,
	       chan->regs + S3C2443_DMA_DMAREQSEL);
}

static struct s3c24xx_dma_selection __initdata s3c2416_dma_sel = {
	.select		= s3c2416_dma_select,
	.dcon_mask	= 0,
	.map		= s3c2416_dma_mappings,
	.map_size	= ARRAY_SIZE(s3c2416_dma_mappings),
};

static int __init s3c2416_dma_add(struct sys_device *sysdev)
{
	s3c24xx_dma_init(8, IRQ_S3C2443_DMA0, 0x100);
	return s3c24xx_dma_init_map(&s3c2416_dma_sel);
}

/* Linux系统调用reboot出现错误的原因: 
 __initdata属性的内存会在初始化结束后被释放掉，而s3c2416_dma_driver是链接在
    s3c2416_sysclass的链表上的，s3c2416_dma_driver内存的释放会造成该链表的错误*/
    
/*static struct sysdev_driver s3c2416_dma_driver __initdata = {
	.add	= s3c2416_dma_add,
};*/

/*
struct sysdev_driver {
	struct list_head	entry;
	int	(*add)(struct sys_device *);
	int	(*remove)(struct sys_device *);
	int	(*shutdown)(struct sys_device *);
	int	(*suspend)(struct sys_device *, pm_message_t state);
	int	(*resume)(struct sys_device *);
};*/

static struct sysdev_driver s3c2416_dma_driver= {
	.add	= s3c2416_dma_add,
};

static int __init s3c2416_dma_init(void)
{
	/* 新增加的 */
	printk("s3c2416_dma_init\n");
	
	printk(" s3c2416_dma_driver.add=0x%08x\n", s3c2416_dma_driver.add);
	printk(" s3c2416_dma_driver.remove=0x%08x\n", s3c2416_dma_driver.remove);
	printk(" s3c2416_dma_driver.shutdown=0x%08x\n", s3c2416_dma_driver.shutdown);
	printk(" s3c2416_dma_driver.suspend=0x%08x\n", s3c2416_dma_driver.suspend);
	printk(" s3c2416_dma_driver.resume=0x%08x\n", s3c2416_dma_driver.resume);
	/* 新增加 结束 */
	return sysdev_driver_register(&s3c2416_sysclass, &s3c2416_dma_driver);
}

arch_initcall(s3c2416_dma_init);


