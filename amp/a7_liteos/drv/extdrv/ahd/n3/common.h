#ifndef __COMMON_H__
#define __COMMON_H__

unsigned char __I2CReadByte8(unsigned char devaddress, unsigned char address);
void __I2CWriteByte8(unsigned char devaddress, unsigned char address, unsigned char data);

#define  I2CReadByte   __I2CReadByte8
#define  I2CWriteByte  __I2CWriteByte8

typedef unsigned char BYTE;

#define BANK0		0
#define BANK1		1
#define BANK2		2
#define BANK3		3
#define BANK4		4
#define BANK5		5
#define BANK6		6
#define BANK7		7
#define BANK8		8
#define BANK9		9
#define BANKA		10
#define BANKB		11

#endif

