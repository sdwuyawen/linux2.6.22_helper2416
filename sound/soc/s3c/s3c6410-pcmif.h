/*
 * s3c24xx-i2s.c  --  ALSA Soc Audio Layer
 *
 * Copyright 2005 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    10th Nov 2006   Initial version.
 */

#ifndef S3C_PCMIF_H_
#define S3C_PCMIF_H_

/* clock sources */
#define S3C24XX_CLKSRC_PCLK 0
#define S3C24XX_CLKSRC_MPLL 1

/* Clock dividers */
#define S3C24XX_DIV_MCLK	0
#define S3C24XX_DIV_BCLK	1
#define S3C24XX_DIV_PRESCALER	2

/* prescaler */

u32 s3c_i2s_get_clockrate(void);

extern struct snd_soc_cpu_dai s3c_pcmif_dai;

#endif /*S3C_PCMIF_H_*/
