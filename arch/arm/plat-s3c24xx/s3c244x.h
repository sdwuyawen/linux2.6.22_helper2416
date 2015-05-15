/* linux/arch/arm/plat-s3c24xx/s3c244x.h
 *
 * Copyright (c) 2004-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Header file for S3C2440 and S3C2442 cpu support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#if defined(CONFIG_CPU_S3C2440) || defined(CONFIG_CPU_S3C2442)

extern void s3c244x_map_io(struct map_desc *mach_desc, int size);

extern void s3c244x_init_uarts(struct s3c2410_uartcfg *cfg, int no);

extern void s3c244x_init_clocks(int xtal);

/* 新增加的S3C2416的部分 */
extern void __init s3c2416_init_clocks(int xtal);
/* S3C2416部分结束 */

#else
#define s3c244x_init_clocks NULL
#define s3c244x_init_uarts NULL
#define s3c244x_map_io NULL

/* 新增加的S3C2416的部分 */
#define s3c2416_init_clocks NULL
/* S3C2416部分结束 */

#endif
