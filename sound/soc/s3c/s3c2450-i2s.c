/*
 * s3c-i2s.c  --  ALSA Soc Audio Layer
 *
 * (c) 2006 Wolfson Microelectronics PLC.
 * Graeme Gregory graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 * (c) 2004-2005 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *	Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *
 *  Revision history
 *    11th Dec 2006   Merged with Simtec driver
 *    10th Nov 2006   Initial version.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-gpioj.h>
#include <asm/arch/audio.h>
#include <asm/arch/dma.h>

#include <asm/arch/regs-s3c2450-clock.h>

#include "s3c-pcm.h"
#include "s3c-i2s.h"

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

/* used to disable sysclk if external crystal is used */
static int extclk = 0;
module_param(extclk, int, 0);
MODULE_PARM_DESC(extclk, "set to 1 to disable s3c24XX i2s sysclk");

static struct s3c2410_dma_client s3c24xx_dma_client_out = {
	.name = "I2S PCM Stereo out"
};

static struct s3c2410_dma_client s3c24xx_dma_client_in = {
	.name = "I2S PCM Stereo in"
};

static struct s3c24xx_pcm_dma_params s3c24xx_i2s_pcm_stereo_out = {
	.client		= &s3c24xx_dma_client_out,
#if defined CONFIG_SND_SOC_I2S_V32
	.channel	= DMACH_I2S_OUT_1,
	.dma_addr	= S3C2450_PA_IIS_1 + S3C2410_IISFIFO,
#elif defined CONFIG_SND_SOC_I2S_V40
	.channel	= DMACH_I2S_OUT,
	.dma_addr	= S3C2410_PA_IIS + S3C2410_IISFIFO,
#endif
	.dma_size	= 4,
};

static struct s3c24xx_pcm_dma_params s3c24xx_i2s_pcm_stereo_in = {
	.client		= &s3c24xx_dma_client_in,
#if defined CONFIG_SND_SOC_I2S_V32
	.channel	= DMACH_I2S_IN_1,
	.dma_addr	= S3C2450_PA_IIS_1 + S3C2410_IISFIFORX,
#elif defined CONFIG_SND_SOC_I2S_V40
	.channel	= DMACH_I2S_IN,
	.dma_addr	= S3C2410_PA_IIS + S3C2410_IISFIFORX,
#endif
	.dma_size	= 4,
};

struct s3c24xx_i2s_info {
	void __iomem	*regs;
	struct clk	*iis_clk;
	int master;
};
static struct s3c24xx_i2s_info s3c24xx_i2s;

static void s3c24xx_snd_txctrl(int on)
{
	u32 iiscon;

	s3cdbg("Entered %s : on = %d \n", __FUNCTION__, on);

	iiscon  = readl(s3c24xx_i2s.regs + S3C2410_IISCON);

	if (on) {
		
		iiscon |= S3C_IIS0CON_I2SACTIVE;
		writel(iiscon,  s3c24xx_i2s.regs + S3C2410_IISCON);

	} else {
		/* note, we have to disable the FIFOs otherwise bad things
		 * seem to happen when the DMA stops. According to the
		 * Samsung supplied kernel, this should allow the DMA
		 * engine and FIFOs to reset. If this isn't allowed, the
		 * DMA engine will simply freeze randomly.
		 */

		iiscon  &= ~S3C_IIS0CON_I2SACTIVE;

		writel(iiscon,  s3c24xx_i2s.regs + S3C2410_IISCON);
	}
}

static void s3c24xx_snd_rxctrl(int on)
{
	u32 iiscon;

	s3cdbg("Entered %s: on = %d\n", __FUNCTION__, on);

	iiscon  = readl(s3c24xx_i2s.regs + S3C2410_IISCON);

	if (on) {
		iiscon  |= S3C_IIS0CON_I2SACTIVE;
		writel(iiscon,  s3c24xx_i2s.regs + S3C2410_IISCON);
	} else {
		/* note, we have to disable the FIFOs otherwise bad things
		 * seem to happen when the DMA stops. According to the
		 * Samsung supplied kernel, this should allow the DMA
		 * engine and FIFOs to reset. If this isn't allowed, the
		 * DMA engine will simply freeze randomly.
		 */
		iiscon  &= ~S3C_IIS0CON_I2SACTIVE;

		writel(iiscon,  s3c24xx_i2s.regs + S3C2410_IISCON);
	}
}

/*
 * Wait for the LR signal to allow synchronisation to the L/R clock
 * from the codec. May only be needed for slave mode.
 */
static int s3c24xx_snd_lrsync(void)
{
	u32 iiscon;
	unsigned long timeout = jiffies + msecs_to_jiffies(5);

	s3cdbg("Entered %s\n", __FUNCTION__);

	while (1) {
		iiscon = readl(s3c24xx_i2s.regs + S3C2410_IISCON);
		if (iiscon & S3C_IISCON_LRINDEX)
			break;

		if (timeout < jiffies)
			return -ETIMEDOUT;
	}

	return 0;
}

/*
 * Check whether CPU is the master or slave
 */
static inline int s3c24xx_snd_is_clkmaster(void)
{
	s3cdbg("Entered %s\n", __FUNCTION__);

	return (readl(s3c24xx_i2s.regs + S3C2410_IISMOD) & S3C_IIS0MOD_IMS_SLAVE) ? 0:1;
}

/*
 * Set S3C24xx I2S DAI format
 */
static int s3c_i2s_set_fmt(struct snd_soc_cpu_dai *cpu_dai,
		unsigned int fmt)
{
	u32 iismod;

	iismod = readl(s3c24xx_i2s.regs + S3C2410_IISMOD);

	iismod &= ~S3C_IIS0MOD_FS_MASK; 
	iismod |= S3C_IIS0MOD_384FS;

#if defined CONFIG_SND_SOC_I2S_V40
	/* 24bit enable */
//	iismod &= ~S3C_IIS0MOD_BLC_MASK;
//	iismod |= S3C_IIS0MOD_BLC_24BIT;
#endif

	writel(iismod, s3c24xx_i2s.regs + S3C2410_IISMOD);

	return 0;

}

static int s3c_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	unsigned long iiscon;
	unsigned long iismod;
	unsigned long iisfcon;
	s3cdbg("Entered %s\n", __FUNCTION__);

	writel((readl(S3C2410_MISCCR) & ~(7<<8))|(1<<8), S3C2410_MISCCR);

	/*Set I2C port to controll WM8753 codec*/
	s3c2410_gpio_pullup(S3C2410_GPE15, 0);
	s3c2410_gpio_pullup(S3C2410_GPE14, 0);
	s3c2410_gpio_cfgpin(S3C2410_GPE15, S3C2410_GPE15_IICSDA);
	s3c2410_gpio_cfgpin(S3C2410_GPE14, S3C2410_GPE14_IICSCL);

#if defined CONFIG_SND_SOC_I2S_V40
	/* Configure the I2S pins in correct mode */
	writel(0x0, S3C2450_GPESEL);

	s3c2410_gpio_cfgpin(S3C2410_GPE0, S3C2410_GPE0_I2SLRCK);
	s3c2410_gpio_cfgpin(S3C2410_GPE1, S3C2410_GPE1_I2SSCLK);
	s3c2410_gpio_cfgpin(S3C2410_GPE2, S3C2410_GPE2_CDCLK);
	s3c2410_gpio_cfgpin(S3C2410_GPE3, S3C2410_GPE3_I2SSDI);
	s3c2410_gpio_cfgpin(S3C2410_GPE4, S3C2410_GPE4_I2SSDO);

	writel(readl(S3C2410_GPEUP)| 0x3ff, S3C2410_GPEUP);

	writel(readl(S3C2450_GPBSEL)|(0x3<<3), S3C2450_GPBSEL);
	writel(readl(S3C2410_GPBUP)|(0xF<<18), S3C2410_GPBUP);

#elif defined CONFIG_SND_SOC_I2S_V32
	/* Configure the I2S pins in correct mode */
	writel(0x0, S3C2450_GPLSEL);

	s3c2410_gpio_cfgpin(S3C2410_GPL4, S3C2450_GPL4_I2S1_SCLK);
	s3c2410_gpio_cfgpin(S3C2410_GPL5, S3C2450_GPL5_I2S1_CDCLK);
	s3c2410_gpio_cfgpin(S3C2410_GPL6, S3C2450_GPL6_I2S1_SDI);
	s3c2410_gpio_cfgpin(S3C2410_GPL7, S3C2450_GPL7_I2S1_SDO);
	s3c2410_gpio_cfgpin(S3C2443_GPJ13, S3C2450_GPJ13_I2S1_LRCK);

	writel(readl(S3C2410_GPLUP)| (0xf<<4), S3C2410_GPLUP);
	writel(readl(S3C2443_GPJDN)| (0x3<<26), S3C2443_GPJDN);

#else
	printk("Error: S3C2450 I2S configration \n",);
#endif

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rtd->dai->cpu_dai->dma_data = &s3c24xx_i2s_pcm_stereo_out;
	} else {
		rtd->dai->cpu_dai->dma_data = &s3c24xx_i2s_pcm_stereo_in;
	}

	/* Working copies of registers */
	iiscon = readl(s3c24xx_i2s.regs + S3C2410_IISCON);
	iismod = readl(s3c24xx_i2s.regs + S3C2410_IISMOD);
	iisfcon = readl(s3c24xx_i2s.regs + S3C2443_IISFIC);

	iiscon |= S3C_IIS0CON_TXDMACTIVE;
	iiscon |= S3C_IIS0CON_RXDMACTIVE;

	iismod &= ~S3C_IIS0MOD_CLK_MASK;
	iismod |= S3C_IIS0MOD_IMS_EXTERNAL_MASTER| S3C_IIS0MOD_INTERNAL_CLK;
	iismod &= ~S3C_IIS0MOD_MODE_MASK;
	iismod |= S3C_IIS0MOD_TXRXMODE;

	/* Multi channel enable */
	iismod &= ~S3C_IIS0MOD_DCE_MASK;
	switch (params_channels(params)) {
	case 6:
		printk("s3c i2s: 5.1channel\n");
		iismod |= S3C_IIS0MOD_DCE_SD1;
		iismod |= S3C_IIS0MOD_DCE_SD2;
		break;
	case 4:
		printk("s3c i2s: 4 channel\n");
		iismod |= S3C_IIS0MOD_DCE_SD1;
		break;
	case 2:
		printk("s3c i2s: 2 channel\n");
		break;
	default:
		printk(KERN_ERR "s3c-i2s-v40: %d channels unsupported\n",
		       params_channels(params));
		return -EINVAL;
	}

	/* Set the bit rate */
#if 0
	iismod &= ~0x6000;
#endif
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		iismod |= S3C_IIS0MOD_8BIT;
		iismod &= ~S3C_IIS0MOD_BFS_MASK;
		iismod |= S3C_IIS0MOD_32FS;
		iismod &= ~S3C_IIS0MOD_FS_MASK;
		iismod |= S3C_IIS0MOD_384FS;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		iismod &= ~S3C_IIS0MOD_FS_MASK;
		iismod &= ~S3C_IIS0MOD_BFS_MASK;
		iismod |= S3C_IIS0MOD_384FS | S3C_IIS0MOD_32FS;
		iismod &= ~S3C_IIS0MOD_BLC_MASK;
		iismod |= S3C_IIS0MOD_16BIT;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iismod &= ~S3C_IIS0MOD_FS_MASK;
		iismod &= ~S3C_IIS0MOD_BFS_MASK;
		iismod |= S3C_IIS0MOD_384FS | S3C_IIS0MOD_48FS;
		iismod &= ~S3C_IIS0MOD_BLC_MASK;
		iismod |= S3C_IIS0MOD_24BIT;
		break;
	default:
		return -EINVAL;
	}


	iisfcon |= S3C_IIS_TX_FLUSH;
	iisfcon |= S3C_IIS_RX_FLUSH;

	writel(iiscon, s3c24xx_i2s.regs + S3C2410_IISCON);
	iismod &= ~S3C_IIS0MOD_FM_MASK;
	writel(iismod, s3c24xx_i2s.regs + S3C2410_IISMOD);
	writel(iisfcon, s3c24xx_i2s.regs + S3C2443_IISFIC);

	/* Tx, Rx fifo flush bit clear */
	iisfcon  &= ~(S3C_IIS_TX_FLUSH | S3C_IIS_RX_FLUSH);
	writel(iisfcon, s3c24xx_i2s.regs + S3C2443_IISFIC);

	s3cdbg("s3c iis mode: 0x%08x\n", readl(s3c24xx_i2s.regs + S3C2410_IISMOD));
	s3cdbg("s3c: params_channels %d\n", params_channels(params));
	s3cdbg("s3c: params_format %d\n", params_format(params));
	s3cdbg("s3c: params_subformat %d\n", params_subformat(params));
	s3cdbg("s3c: params_period_size %d\n", params_period_size(params));
	s3cdbg("s3c: params_period_bytes %d\n", params_period_bytes(params));
	s3cdbg("s3c: params_periods %d\n", params_periods(params));
	s3cdbg("s3c: params_buffer_size %d\n", params_buffer_size(params));
	s3cdbg("s3c: params_buffer_bytes %d\n", params_buffer_bytes(params));
	s3cdbg("s3c: params_tick_time %d\n", params_tick_time(params));

	return 0;
}

static int s3c_i2s_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;

	s3cdbg("Entered %s: cmd = %d\n", __FUNCTION__, cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (!s3c24xx_snd_is_clkmaster()) {
			ret = s3c24xx_snd_lrsync();
			if (ret)
				goto exit_err;
		}

		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			s3c24xx_snd_rxctrl(1);
		else
			s3c24xx_snd_txctrl(1);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			s3c24xx_snd_rxctrl(0);
		else
			s3c24xx_snd_txctrl(0);
		break;
	default:
		ret = -EINVAL;
		break;
	}

exit_err:
	return ret;
}

static void s3c64xx_i2s_shutdown(struct snd_pcm_substream *substream)
{
	unsigned long iiscon;

	iiscon=readl(s3c24xx_i2s.regs + S3C2410_IISCON);
	iiscon &= ~S3C_IIS0CON_I2SACTIVE;

	writel(iiscon, s3c24xx_i2s.regs + S3C2410_IISCON);
}


/*
 * Set S3C24xx Clock source
 */
static int s3c_i2s_set_sysclk(struct snd_soc_cpu_dai *cpu_dai,
	int clk_id, unsigned int freq, int dir)
{
	u32 iismod = readl(s3c24xx_i2s.regs + S3C2410_IISMOD);

	switch (clk_id) {
	case S3C24XX_CLKSRC_PCLK:
		break;
	case S3C24XX_CLKSRC_MPLL:
		break;
	default:
		return -EINVAL;
	}

	writel(iismod, s3c24xx_i2s.regs + S3C2410_IISMOD);
	return 0;
}

/*
 * Set S3C24xx Clock dividers
 */
static int s3c_i2s_set_clkdiv(struct snd_soc_cpu_dai *cpu_dai,
	int div_id, int div)
{
	u32 reg;

	s3cdbg("Entered %s : div_id = %d, div = %d\n", __FUNCTION__, div_id, div);

	switch (div_id) {
	case S3C24XX_DIV_MCLK:
		reg = readl(s3c24xx_i2s.regs + S3C2410_IISMOD) & ~S3C2410_IISMOD_BFS_MASK;
		writel(reg | div, s3c24xx_i2s.regs + S3C2410_IISMOD);
		break;
	case S3C24XX_DIV_BCLK:
		reg = readl(s3c24xx_i2s.regs + S3C2410_IISMOD) & ~(S3C_IIS0MOD_FS_MASK);
		writel(reg | div, s3c24xx_i2s.regs + S3C2410_IISMOD);
		break;
	case S3C24XX_DIV_PRESCALER:
		reg = readl(s3c24xx_i2s.regs + S3C2443_IISPSR);
		reg &= ~(S3C_IISPSR_PS_MASK|S3C_IISPSR_PSRAEN); 
		writel(reg , s3c24xx_i2s.regs + S3C2443_IISPSR);
		reg |= (div|S3C_IISPSR_PSRAEN);
		writel(reg, s3c24xx_i2s.regs + S3C2443_IISPSR);
		break;
	default:
		return -EINVAL;
	}
	
	return 0;
}

/*
 * To avoid duplicating clock code, allow machine driver to
 * get the clockrate from here.
 */
u32 s3c_i2s_get_clockrate(void)
{
	return clk_get_rate(s3c24xx_i2s.iis_clk);
}
EXPORT_SYMBOL_GPL(s3c_i2s_get_clockrate);

static int s3c_i2s_probe(struct platform_device *pdev)
{
	s3cdbg("Entered %s\n", __FUNCTION__);

#if defined CONFIG_SND_SOC_I2S_V32
	s3c24xx_i2s.regs = ioremap(S3C2450_PA_IIS_1, 0x100);
	if (s3c24xx_i2s.regs == NULL)
		return -ENXIO;
#elif defined CONFIG_SND_SOC_I2S_V40
	s3c24xx_i2s.regs = ioremap(S3C2410_PA_IIS, 0x100);
	if (s3c24xx_i2s.regs == NULL)
		return -ENXIO;
#endif

	s3c24xx_i2s.iis_clk=clk_get(&pdev->dev, "iis");
	if (s3c24xx_i2s.iis_clk == NULL) {
		s3cdbg("failed to get iis_clock\n");
		return -ENODEV;
	}
	clk_enable(s3c24xx_i2s.iis_clk);

	return 0;
}

#ifdef CONFIG_PM
static int s3c_i2s_suspend(struct platform_device *dev,
	struct snd_soc_cpu_dai *dai)
{
	s3cdbg("Entered %s\n", __FUNCTION__);
	return 0;
}

static int s3c_i2s_resume(struct platform_device *pdev,
	struct snd_soc_cpu_dai *dai)
{
	s3cdbg("Entered %s\n", __FUNCTION__);
	return 0;
}

#else
#define s3c_i2s_suspend	NULL
#define s3c_i2s_resume	NULL
#endif


#define S3C24XX_I2S_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

struct snd_soc_cpu_dai s3c_i2s_dai = {
	.name = "s3c-i2s",
	.id = 0,
	.type = SND_SOC_DAI_I2S,
	.probe = s3c_i2s_probe,
	.suspend = s3c_i2s_suspend,
	.resume = s3c_i2s_resume,
	.playback = {
		.channels_min = 2,
		.channels_max = 6,
		.rates = S3C24XX_I2S_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},

	.capture = {
		.channels_min = 2,
		.channels_max = 6,
		.rates = S3C24XX_I2S_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},

	.ops = {
		.shutdown = s3c64xx_i2s_shutdown,
		.trigger = s3c_i2s_trigger,
		.hw_params = s3c_i2s_hw_params,},
	.dai_ops = {
		.set_fmt = s3c_i2s_set_fmt,
		.set_clkdiv = s3c_i2s_set_clkdiv,
		.set_sysclk = s3c_i2s_set_sysclk,
	},
};
EXPORT_SYMBOL_GPL(s3c_i2s_dai);

/* Module information */
MODULE_AUTHOR("Ryu, <ryu.real@gmail.com>");
MODULE_DESCRIPTION("s3c2450 I2S SoC Interface");
MODULE_LICENSE("GPL");
