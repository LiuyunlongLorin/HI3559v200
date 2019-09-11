#ifndef __IRQ_MAP_CONFIG_H__
#define __IRQ_MAP_CONFIG_H__

#ifdef  LOSCFG_DRIVERS_HIETH_SF
#define HIETH_SF_IRQ TO_CPU1
#else
#define HIETH_SF_IRQ TO_CPU0
#endif

#ifdef LOSCFG_DRIVERS_SVP
#define HISI_IVE_IRQ  TO_CPU1
#define HISI_NNIE_IRQ TO_CPU1
#else
#define HISI_IVE_IRQ  TO_CPU0
#define HISI_NNIE_IRQ TO_CPU0
#endif

#define TO_CPU0  (1<<0x00) //to Linux
#define TO_CPU1  (1<<0x01) //to Local
const unsigned char irq_map[OS_HWI_MAX_NUM-32] = {
	/*32      33        34        35 */
	/* 		 timer0/1 timer2/3 timer4/5 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU1, /* 32  ~ 35 */
	/*timer6/7        uart0    uart1    */
	TO_CPU1, TO_CPU0, TO_CPU0, TO_CPU1, /* 36  ~ 39 */
	/*uart2   uart3   uart4    i2c0     */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU1,	/* 40  ~ 43 */
	/*i2c1   i2c2     i2c3     i2c4    */
	TO_CPU1, TO_CPU1, TO_CPU1, TO_CPU1,	/* 44  ~ 47 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0, /* 48  ~ 51 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 52  ~ 55 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 56  ~ 59 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 60  ~ 63 */
	HIETH_SF_IRQ, TO_CPU0, TO_CPU0, TO_CPU0,	/* 64  ~ 67 */
	TO_CPU1, HISI_IVE_IRQ, TO_CPU1, TO_CPU0,	/* 68  ~ 71 */
	TO_CPU1, HISI_NNIE_IRQ, TO_CPU1, TO_CPU1,	/* 72  ~ 75 */
	TO_CPU1, TO_CPU1, TO_CPU0, TO_CPU0,	/* 76  ~ 79 */
	TO_CPU1, TO_CPU0, TO_CPU0, TO_CPU0,	/* 80  ~ 83 */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU1,	/* 84  ~ 87 */
	TO_CPU1, TO_CPU1, TO_CPU1, TO_CPU0,	/* 88  ~ 91 */
	TO_CPU1, TO_CPU1, TO_CPU1, TO_CPU1,	/* 92  ~ 95 */
	TO_CPU0, TO_CPU1, TO_CPU0, TO_CPU1, /* 96  ~ 99 */
	/*ssp0	 ssp1	  ssp2				*/
	TO_CPU1, TO_CPU1, TO_CPU1, TO_CPU1,	/* 100 ~ 103*/
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 104 ~ 107*/
	/*       i2c5	  i2c6	   i2c7     */
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 108 ~ 111*/
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 112 ~ 115*/
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 116 ~ 119*/
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 120 ~ 123*/
	TO_CPU0, TO_CPU0, TO_CPU0, TO_CPU0,	/* 124 ~ 127*/
};/* [32 - 255] */
#else
#error "should not include irq_map.h twice"
#endif
