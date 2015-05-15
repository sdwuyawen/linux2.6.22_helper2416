///////////////////////////////////////////////////////////////
// ������ֻ��ѧϰʹ�ã�δ������˾��ɣ��������������κ���ҵ��;
// ���ÿ������ͺ�:Tiny2416��Mini2451��Tiny2451
// ������̳:www.arm9home.net
// �޸�����:2013/7/1
// ��Ȩ���У�����ؾ���
// Copyright(C) ��������֮�ۼ�����Ƽ����޹�˾
// All rights reserved							
///////////////////////////////////////////////////////////////

// ����:��ʼ������
#include "uart_2416.h" 
//#include "../../include/asm-arm/arch-s3c2410/Map.h"
//#include <asm/mach/map.h>
//#include <asm/arch-s3c2410/map.h>

#define ULCON0   		( *((volatile unsigned long *)0x50000000) )
#define UCON0    		( *((volatile unsigned long *)0x50000004) )
#define UFCON0   	 	( *((volatile unsigned long *)0x50000008) )
#define UMCON0    		( *((volatile unsigned long *)0x5000000C) )
#define UTRSTAT0  		( *((volatile unsigned long *)0x50000010) )
#define UFSTAT0 		( *((volatile unsigned long *)0x50000018) )
#define UTXH0      		( *((volatile unsigned char *)0x50000020) )
#define URXH0      		( *((volatile unsigned char *)0x50000024) ) 
#define UBRDIV0    		( *((volatile unsigned short *)0x50000028) )
#define UDIVSLOT0  		( *((volatile unsigned short *)0x5000002C) )
#define GPHCON     		( *((volatile unsigned long *)0x56000070 ) )

#define TXD0READY   (1<<2)

#define S3C2410_ADDR(x)	  (0xF0000000 + (x))
/* UARTs */
#define S3C24XX_VA_UART	   S3C2410_ADDR(0x00400000)
#define S3C2400_PA_UART	   (0x15000000)
#define S3C2410_PA_UART	   (0x50000000)
#define S3C6400_PA_UART	   (0x7F005000)
#define S3C24XX_SZ_UART	   SZ_1M

void uart_init(void)
{
	// ��������  
	GPHCON = (GPHCON & ~0xffff ) | 0xaaaa;
		
	// �������ݸ�ʽ��  
	ULCON0 = 0x3;  					// ����λ:8, ��У��, ֹͣλ: 1, 8n1 
	UCON0  = 0x5;  					// ʱ�ӣ�PCLK����ֹ�жϣ�ʹ��UART���͡����� 
	UFCON0 = 0x01; 					// FIFO ENABLE
	UMCON0 = 0;						// ������
	
	// ���ò�����  
	// DIV_VAL = (PCLK / (bps x 16 ) ) - 1 = (66500000/(115200x16))-1 = 35.08
	// DIV_VAL = 35.08 = UBRDIVn + (num of 1��s in UDIVSLOTn)/16 
	UBRDIV0   = 35;
	UDIVSLOT0 = 0x1;
}

// ����һ���ַ�  
char getchar(void)
{
	while ((UFSTAT0 & 0x7f) == 0);  // ���RX FIFO�գ��ȴ� 
	return URXH0;                   // ȡ���� 
}

// ����һ���ַ�  
void putchar(char c)
{
	while (UFSTAT0 & (1<<14)); 		// ���TX FIFO�����ȴ� 
	UTXH0 = c;                      // д���� 

	/* �ȴ���ֱ�����ͻ������е������Ѿ�ȫ�����ͳ�ȥ */
	while (!(UTRSTAT0 & TXD0READY));
}

// ����һ���ַ��������ַ  
void putchar_VA(char c)
{
	while ((*((volatile unsigned long *)(S3C24XX_VA_UART + 0x18))) & (1<<14)); 		// ���TX FIFO�����ȴ� 
	(*((volatile unsigned char *)(S3C24XX_VA_UART + 0x20))) = c;                      // д���� 

	/* �ȴ���ֱ�����ͻ������е������Ѿ�ȫ�����ͳ�ȥ */
	while (!((*((volatile unsigned char *)(S3C24XX_VA_UART + 0x10))) & TXD0READY));
}


// ����һ���ַ���
void putstring(char *string)
{
	while((*string) != 0x00)
	{
		putchar(*string);
		string ++;
	}
}

// ����һ���ַ����������ַ  
void putstring_VA(char *string)
{
	while((*string) != 0x00)
	{
		putchar_VA(*string);
		string ++;
	}
}

void putinthex(unsigned int data)
{
	/*
	putchar((data /0x10000000)+ '0');
	putchar((data %0x10000000) / 0x01000000+ '0');
	putchar((data %0x01000000) / 0x00100000+ '0');
	putchar((data %0x00100000) / 0x00010000+ '0');
	putchar((data %0x00010000) / 0x00001000+ '0');
	putchar((data %0x00001000) / 0x00000100+ '0');
	putchar((data %0x00000100) / 0x00000010+ '0');
	putchar((data %0x00000010) / 0x00000001+ '0');
	putchar('\r');
	putchar('\n');
	*/

	unsigned int i;
	unsigned char data_char;

	for(i = 0; i < 8; i++)
	{
		data_char = (data >> (4 *(7 - i))) & 0x0F;
		if(data_char > 9)
		{
			putchar(data_char - 0x0A + 'A');
		}
		else
		{
			putchar(data_char + '0');
		}
	}
	putchar('\r');
	putchar('\n');
	
}


void putinthex_VA(unsigned int data)
{
	unsigned int i;
	unsigned char data_char;

	for(i = 0; i < 8; i++)
	{
		data_char = (data >> (4 *(7 - i))) & 0x0F;
		if(data_char > 9)
		{
			putchar_VA(data_char - 0x0A + 'A');
		}
		else
		{
			putchar_VA(data_char + '0');
		}
	}
	putchar_VA('\r');
	putchar_VA('\n');
	
}

//����������Ϣ
void put_start_info(unsigned int num)
{
	putstring("starting----");
	putchar('p');
	putchar('a');
	putchar('r');
	putchar(num + '0');
	putstring("\r\n");
}
