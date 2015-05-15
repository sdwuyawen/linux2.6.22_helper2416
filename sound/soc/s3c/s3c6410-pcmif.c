/*
 * s3c6410-pcmif.c  --  ALSA Soc Audio Layer
 *
 * (c) 2008 Samsung Electronics
 *	Ryu Euiyoul <steven.ryu@samsung.com, ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
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

#include <asm/arch/regs-pcm.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/audio.h>
#include <asm/arch/dma.h>

#include <asm/arch/regs-s3c6410-clock.h>

#include "s3c-pcm.h"
#include "s3c6410-pcmif.h"

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static struct s3c2410_dma_client s3c24xx_dma_client_out = {
	.name = "PCM Mono out"
};

static struct s3c2410_dma_client s3c24xx_dma_client_in = {
	.name = "PCM Mono in"
};

static struct s3c24xx_pcm_dma_params s3c_pcmif_dma_out = {
	.client		= &s3c24xx_dma_client_out,
	.channel	= DMACH_PCMIF_CH1_TX,
	.dma_addr	= S3C6410_PA_PCM_PORT1 + S3C_PCM_TX_FIFO,
	.dma_size	= 2, /* transfer width = half word(16bit) */
};

static struct s3c24xx_pcm_dma_params s3c_pcmif_dma_in = {
	.client		= &s3c24xx_dma_client_in,
	.channel	= DMACH_PCMIF_CH1_RX,
	.dma_addr	= S3C6410_PA_PCM_PORT1 + S3C_PCM_RX_FIFO,
	.dma_size	= 2, /* transfer width = half word(16bit) */
};

struct s3c_pcmif_info {
	void __iomem	*regs;
	struct clk	*pcmif_clk;
};
static struct s3c_pcmif_info s3c_pcmif;

static int s3c_pcmif_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	u32 pcm_ctl;	
	
	s3cdbg("Entered %s\n", __FUNCTION__);
	
	s3c_gpio_cfgpin(S3C_GPH7, S3C_GPH7_I2S_V40_CDCLK);
	s3c_gpio_pullup(S3C_GPH7, 0x2);

        s3c_gpio_cfgpin(S3C_GPE0,S3C_GPE0_PCM_DCLK1);
        s3c_gpio_cfgpin(S3C_GPE1,S3C_GPE1_PCM_EXTCLK1);
        s3c_gpio_cfgpin(S3C_GPE2,S3C_GPE2_PCM_FSYNC1);
        s3c_gpio_cfgpin(S3C_GPE3,S3C_GPE3_PCM_SIN1);
        s3c_gpio_cfgpin(S3C_GPE4,S3C_GPE4_PCM_SOUT1);

        s3c_gpio_pullup(S3C_GPE0,0);
        s3c_gpio_pullup(S3C_GPE1,0);
        s3c_gpio_pullup(S3C_GPE2,0);
        s3c_gpio_pullup(S3C_GPE3,0);
        s3c_gpio_pullup(S3C_GPE4,0);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rtd->dai->cpu_dai->dma_data = &s3c_pcmif_dma_out;

	} else {
		rtd->dai->cpu_dai->dma_data = &s3c_pcmif_dma_in;
	}

	pcm_ctl = readl(s3c_pcmif.regs + S3C_PCM_CTL);

	pcm_ctl |= S3C_PCM_CTL_TX_DMA_EN; 
	pcm_ctl |= S3C_PCM_CTL_TXFIFO_EN; 
	pcm_ctl |= S3C_PCM_CTL_TX_FIFO_DIPSTICK(0x8);
	pcm_ctl |= S3C_PCM_CTL_RX_DMA_EN; 
	pcm_ctl |= S3C_PCM_CTL_RXFIFO_EN; 
	pcm_ctl |= S3C_PCM_CTL_RX_FIFO_DIPSTICK(0x20);
	pcm_ctl |= S3C_PCM_CTL_PCM_ENABLE; 

	writel(pcm_ctl, s3c_pcmif.regs + S3C_PCM_CTL);
	writel(S3C_PCM_TX_FIFO_DVALID, s3c_pcmif.regs + S3C_PCM_TX_FIFO);
	writel(S3C_PCM_RX_FIFO_DVALID, s3c_pcmif.regs + S3C_PCM_RX_FIFO);

	s3cdbg("s3c pcm ctl: 0x%08x\n", readl(s3c_pcmif.regs + S3C_PCM_CTL));

	return 0;

}

static void s3c_pcmif_shutdown(struct snd_pcm_substream *substream)
{
	u32 pcm_ctl; 

	s3cdbg("Entered %s\n", __FUNCTION__);

	pcm_ctl = readl(s3c_pcmif.regs + S3C_PCM_CTL);

	pcm_ctl &= ~S3C_PCM_CTL_PCM_ENABLE; 

	writel(pcm_ctl, s3c_pcmif.regs + S3C_PCM_CTL);

	s3c_gpio_cfgpin(S3C_GPH7,0);

        s3c_gpio_cfgpin(S3C_GPE0,0);
        s3c_gpio_cfgpin(S3C_GPE1,0);
        s3c_gpio_cfgpin(S3C_GPE2,0);
        s3c_gpio_cfgpin(S3C_GPE3,0);
        s3c_gpio_cfgpin(S3C_GPE4,0);
}

/*
 * Set S3C24xx Clock dividers
 */
static int s3c_pcmif_set_clkdiv(struct snd_soc_cpu_dai *cpu_dai,
	int div_id, int fs)
{
	u32 pcm_clk, pcmsource, sclk;
	u32 sync_div = 31; /* must grater than 16 */
	u32 sclk_div;
	unsigned long pclk;
	

	s3c_pcmif.pcmif_clk=clk_get(NULL, "pcmif_1");
	if (s3c_pcmif.pcmif_clk == NULL) {
		printk("failed to get pcmif\n");
		iounmap(s3c_pcmif.regs);
		return -ENODEV;
	}
	clk_enable(s3c_pcmif.pcmif_clk);

	pclk = clk_get_rate(s3c_pcmif.pcmif_clk);

	pcmsource = pclk/1000;
	sclk = fs * (sync_div +1)/1000;
	
	/* To reduce noise */
	//sclk_div = pcmsource /(2*sclk) - 1;
	sclk_div = pcmsource /(2*sclk) + 1;
	
	s3cdbg("fs = %d,pcmsource = %d,sclk = %d,sclk_div = %d\n",
				 __FUNCTION__, fs, pcmsource, sclk, sclk_div);

	/* PCM clock control */
	pcm_clk = S3C_PCM_CTLCTL_SERCLK_EN; /* using pclk */
	pcm_clk |= S3C_PCM_CTLCTL_SERCLK_SEL; /* using pclk */

	pcm_clk |= S3C_PCM_CLKCTL_SCLK_DIV(sclk_div);
	pcm_clk |= S3C_PCM_CLKCTL_SYNC_DIV(sync_div);
	s3cdbg("pcm_clk = 0x%08x\n",pcm_clk);

	writel(pcm_clk, s3c_pcmif.regs + S3C_PCM_CLKCTL);
	
	return 0;
}

static int s3c_pcmif_probe(struct platform_device *pdev)
{
	s3cdbg("Entered %s\n", __FUNCTION__);

	s3c_pcmif.regs = ioremap(S3C6410_PA_PCM_PORT1, 0x100);
	if (s3c_pcmif.regs == NULL)
		return -ENXIO;
#if 0	

	s3c_pcmif.pcmif_clk=clk_get(&pdev->dev, "pcmif");
	if (s3c_pcmif.pcmif_clk == NULL) {
		printk("failed to get pcmif\n");
		iounmap(s3c_pcmif.regs);
		return -ENODEV;
	}
	clk_enable(s3c_pcmif.pcmif_clk);
#endif


	return 0;
}

#ifdef CONFIG_PM
static int s3c_pcmif_suspend(struct platform_device *dev,
	struct snd_soc_cpu_dai *dai)
{
	s3cdbg("Entered %s\n", __FUNCTION__);
	return 0;
}

static int s3c_pcmif_resume(struct platform_device *pdev,
	struct snd_soc_cpu_dai *dai)
{
	s3cdbg("Entered %s\n", __FUNCTION__);
	return 0;
}

#else
#define s3c_pcmif_suspend	NULL
#define s3c_pcmif_resume	NULL
#endif

#define S3C_PCMIF_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

struct snd_soc_cpu_dai s3c_pcmif_dai = {
	.name = "s3c-pcmif",
	.id = 0,
	.type = SND_SOC_DAI_PCM,
	.probe = s3c_pcmif_probe,
	.suspend = s3c_pcmif_suspend,
	.resume = s3c_pcmif_resume,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = S3C_PCMIF_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.capture = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = S3C_PCMIF_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,},
	.ops = {
		.shutdown = s3c_pcmif_shutdown,
		.hw_params = s3c_pcmif_hw_params,},
	.dai_ops = {
		.set_clkdiv = s3c_pcmif_set_clkdiv,
	},
};
EXPORT_SYMBOL_GPL(s3c_pcmif_dai);

/* Module information */
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("S3C PCM INTERFACE");
MODULE_LICENSE("GPL");
