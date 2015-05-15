/*
 * smdk6400_wm8580_pcmif.c
 *
 * Copyright (C) 2007, Ryu Euiyoul
 * 		 <steven,ryu@samsung.com, ryu.real@gmail.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
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
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>

#include <asm/hardware.h>
#include <asm/arch/audio.h>
#include <asm/io.h>
#include <asm/arch/regs-s3c6410-clock.h>

#include "../codecs/wm8580.h"
#include "s3c-pcm.h"
#include "s3c6410-pcmif.h"

#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static int smdk6410_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	//struct snd_soc_codec_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_cpu_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret = 0;
	int fs = params_rate(params);

	/*PCLK & SCLK gating enable*/

	writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_PCM0|S3C_CLKCON_PCLK_PCM1, S3C_PCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0|S3C_CLKCON_SCLK_AUDIO1, S3C_SCLK_GATE);

	/* set prescaler division for sample rate */
	ret = cpu_dai->dai_ops.set_clkdiv(cpu_dai, 0, fs);
	if (ret < 0)
		return ret;

	return 0;

}

/*
 * WM8580 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6410_hifi_ops = {
	.hw_params = smdk6410_hifi_hw_params,
};

static const struct snd_soc_dapm_widget wm8580_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("I2S Front Jack", NULL),
	SND_SOC_DAPM_LINE("I2S Center Jack", NULL),
	SND_SOC_DAPM_LINE("I2S Rear Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};

/* example machine audio_mapnections */
static const char* audio_map[][3] = {

	{ "I2S Front Jack", NULL, "VOUT1L" },
	{ "I2S Front Jack", NULL, "VOUT1R" },

	{ "I2S Center Jack", NULL, "VOUT2L" },
	{ "I2S Center Jack", NULL, "VOUT2R" },

	{ "I2S Rear Jack", NULL, "VOUT3L" },
	{ "I2S Rear Jack", NULL, "VOUT3R" },

	{ "AINL", NULL, "Line In Jack" },
	{ "AINR", NULL, "Line In Jack" },
		
	{NULL, NULL, NULL},
};

static int smdk6410_wm8580_init(struct snd_soc_codec *codec)
{
	int i;

	/* Add smdk6410 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8580_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8580_dapm_widgets[i]);

	/* set up smdk6410 specific audio paths */
	for (i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}

	/* No jack detect - mark all jacks as enabled */
	for (i = 0; i < ARRAY_SIZE(wm8580_dapm_widgets); i++)
		snd_soc_dapm_set_endpoint(codec,
					  wm8580_dapm_widgets[i].name, 1);

	return 0;
}

static struct snd_soc_dai_link smdk6410_dai[] = {
{
	.name = "WM8580",
	.stream_name = "WM8580 HiFi Playback",
	.cpu_dai = &s3c_pcmif_dai,
	.codec_dai = &wm8580_dai[1],
	.init = smdk6410_wm8580_init,
	.ops = &smdk6410_hifi_ops,
},
};

static struct snd_soc_machine smdk6410 = {
	.name = "smdk6410",
	.dai_link = smdk6410_dai,
	.num_links = ARRAY_SIZE(smdk6410_dai),
};

static struct wm8580_setup_data smdk6410_wm8580_setup = {
	.i2c_address = 0x1b,
};

static struct snd_soc_device smdk6410_snd_devdata = {
	.machine = &smdk6410,
	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8580,
	.codec_data = &smdk6410_wm8580_setup,
};

static struct platform_device *smdk6410_snd_device;

static int __init smdk6410_pcmif_init(void)
{
	int ret;

	smdk6410_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk6410_snd_device)
		return -ENOMEM;

	platform_set_drvdata(smdk6410_snd_device, &smdk6410_snd_devdata);
	smdk6410_snd_devdata.dev = &smdk6410_snd_device->dev;
	ret = platform_device_add(smdk6410_snd_device);

	if (ret)
		platform_device_put(smdk6410_snd_device);
	
	return ret;
}

static void __exit smdk6410_pcmif_exit(void)
{
	platform_device_unregister(smdk6410_snd_device);
}

module_init(smdk6410_pcmif_init);
module_exit(smdk6410_pcmif_exit);

/* Module information */
MODULE_AUTHOR("Ryu Euiyoul");
MODULE_DESCRIPTION("ALSA SoC SMDK6410 WM8580");
MODULE_LICENSE("GPL");
