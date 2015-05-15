/*
 * smdk2416_wm8731.c  --  SoC audio for Neo1973
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  Copyright (C) 2007, Ryu Euiyoul <ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    20th Jan 2007   Initial version.
 *    05th Feb 2007   Rename all to Neo1973
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/arch/audio.h>
#include <asm/io.h>
#include <asm/arch/spi-gpio.h>
#include <asm/arch/regs-s3c2450-clock.h>

#include "../codecs/wm8731.h"
#include "s3c-pcm.h"
#include "s3c-i2s.h"


/* define the scenarios */
#define SMDK6400_AUDIO_OFF		0
#define SMDK6400_CAPTURE_MIC1		3
#define SMDK6400_STEREO_TO_HEADPHONES	2
#define SMDK6400_CAPTURE_LINE_IN	1

#define CONFIG_SND_DEBUG 1 

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static int smdk2416_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{


	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_cpu_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int pll_out = 0, bclk = 0;
	int ret = 0;
	unsigned int regs;
	unsigned int prescaler=0;
	int rate=params_rate(params);

	u32  cdfs,cdclk ;
	
	printk("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));
	
	/* Select Clock source EPLL */
//	regs = ioremap(S3C2410_PA_CLKPWR,0x20);
	regs = readl(S3C2443_CLKSRC);
	regs &= ~(3<<12);
	regs &= ~(3<<14);
	regs |= S3C2450_CLKSRC_I2S1_EPLL;
	regs = (regs & ~(3<<7))|(2<<7);
	writel(regs, S3C2443_CLKSRC);
	regs |= (1<<6);
	writel(regs, S3C2443_CLKSRC);

	regs = readl(S3C2443_SCLKCON);
	regs |= S3C2443_SCLKCON_I2SCLK_1;
	writel(regs, S3C2443_SCLKCON);

printk("%s: %d , params = %d \n", __FUNCTION__, __LINE__, params_rate(params));

	switch (params_rate(params)) {
	case 8000:
	case 16000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		//cdfs=256; 
		cdclk=12288000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		 //cdfs
		cdclk=11289600;
		break;
	default:
		printk("Unsupported rate = %d\n", params_rate(params));
		break;
	}

	u32 clkiis = s3c_i2s_get_clockrate();

//	printk("clkiis=%d,cdclk=%d \n ", clkiis,cdclk);
	
	prescaler= ((clkiis+128*rate)/(512*rate))-1;
	if(prescaler<0)
		prescaler=0;
//	printk("prescale =%d \n",prescaler);

	if(prescaler<0)
		prescaler=0;

//set for codec WM8731


	ret = codec_dai->dai_ops.set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0){
		s3cdbg("fuck here: in %s line:%d \n",__FUNCTION__,__LINE__);
		return ret;
	}

	ret = codec_dai->dai_ops.set_sysclk(codec_dai, 2, cdclk,
		SND_SOC_CLOCK_IN);
	if (ret < 0){
		s3cdbg("fuck here: %s line :%d \n",__FUNCTION__,__LINE__);
		return ret;
	}

//set for cpu iis

	ret = cpu_dai->dai_ops.set_fmt(cpu_dai,
                SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
                SND_SOC_DAIFMT_CBS_CFS ); 
        if (ret < 0){
        s3cdbg("fuck here: %s line: %d \n",__FUNCTION__,__LINE__);
                return ret;
        }

	 ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_BCLK,
                S3C2410_IISMOD_32FS );
        if (ret < 0){
                s3cdbg("fuck here: %s line :%d \n",__FUNCTION__,__LINE__);
                return ret;
                }
        ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_MCLK,
                S3C2410_IISMOD_384FS);
        if (ret < 0){
                s3cdbg("fuck here: %s line :%d \n",__FUNCTION__,__LINE__);
                return ret;
        }


	
	/* set prescaler division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER,
	(prescaler)<<0x8);
	if (ret < 0){
		s3cdbg("fuck here: %s line :%d \n",__FUNCTION__,__LINE__);
		return ret; 
	}

	return 0;

}

static int smdk2416_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec_dai *codec_dai = rtd->dai->codec_dai;

	/* disable the PLL */
	//return codec_dai->dai_ops.set_pll(codec_dai, WM8753_PLL1, 0, 0);
	return 0;
}

/*
 * Neo1973 WM8731 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk2416_hifi_ops = {
	.hw_params = smdk2416_hifi_hw_params,
	.hw_free = smdk2416_hifi_hw_free,
};


/*
 * This is an example machine initialisation for a wm8731 connected to a
 * smdk2416. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int smdk2416_wm8731_init(struct snd_soc_codec *codec)
{
	
	
	return 0;
}

static struct snd_soc_dai_link smdk2416_dai[] = {
{
 /* Hifi Playback - for similatious use with voice below */
	.name = "wm8731",
	.stream_name = "wm8731",
	.cpu_dai = &s3c_i2s_dai,
	.codec_dai = &wm8731_dai,
	.init = smdk2416_wm8731_init,
	.ops = &smdk2416_hifi_ops,
},

};

static struct snd_soc_machine smdk2416 = {
	.name = "smdk2416",
	.dai_link = smdk2416_dai,
	.num_links = ARRAY_SIZE(smdk2416_dai),
};

static struct wm8731_setup_data smdk2416_wm8731_setup = {
	.i2c_address = 0x1a,
};



static struct snd_soc_device smdk2416_snd_devdata = {
	.machine = &smdk2416,
	.platform = &s3c24xx_soc_platform,			/* s3c-pcm.c */
	.codec_dev = &soc_codec_dev_wm8731,		
	.codec_data = &smdk2416_wm8731_setup,

};

static struct platform_device *smdk2416_snd_device;

static int __init smdk2416_init(void)
{
	int ret;

	smdk2416_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk2416_snd_device)
		return -ENOMEM;

	platform_set_drvdata(smdk2416_snd_device, &smdk2416_snd_devdata);
	smdk2416_snd_devdata.dev = &smdk2416_snd_device->dev;
	ret = platform_device_add(smdk2416_snd_device);

	if (ret){
		printk("failed add smdk2416_snd_device\n");
		platform_device_put(smdk2416_snd_device);
	}
	
	return ret;
}

static void __exit smdk2416_exit(void)
{
	platform_device_unregister(smdk2416_snd_device);
}

module_init(smdk2416_init);
module_exit(smdk2416_exit);

/* Module information */
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("ALSA SoC WM8731 Neo1973");
MODULE_LICENSE("GPL");
