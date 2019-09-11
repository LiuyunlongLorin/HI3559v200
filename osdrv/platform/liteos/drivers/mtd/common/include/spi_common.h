/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef __SPI_COMMON_H__
#define __SPI_COMMON_H__

#define FEATURE_ECC_ENABLE                  (1 << 4)
#define FEATURE_QE_ENABLE                   (1 << 0)
/*****************************************************************************/

#define STATUS_P_FAIL_MASK                  (1 << 3)
#define STATUS_E_FAIL_MASK                  (1 << 2)
#define STATUS_WEL_MASK                     (1 << 1)
#define STATUS_OIP_MASK                     (1 << 0)

/*****************************************************************************/
#define FEATURES_OP_ADDR_NUM                1
#define STD_OP_ADDR_NUM                     3

/*****************************************************************************/
#define SPI_IF_TYPE_STD         0x0
#define SPI_IF_TYPE_DUAL        0x1
#define SPI_IF_TYPE_DIO         0x2
#define SPI_IF_TYPE_QUAD        0x3
#define SPI_IF_TYPE_QIO         0x4

#define SPI_IF_READ_STD			(0x01)
#define SPI_IF_READ_FAST		(0x02)
#define SPI_IF_READ_DUAL		(0x04)
#define SPI_IF_READ_DUAL_ADDR		(0x08)
#define SPI_IF_READ_QUAD		(0x10)
#define SPI_IF_READ_QUAD_ADDR		(0x20)
#define SPI_IF_READ_QUAD_DTR		(0x40)
#define  spi_is_quad(spi) \
        (((SPI_IF_TYPE_QUAD == spi->read->iftype) \
        || (SPI_IF_TYPE_QIO == spi->read->iftype) \
        || (SPI_IF_TYPE_QUAD == spi->write->iftype) \
        || (SPI_IF_TYPE_QIO == spi->write->iftype))?1:0)

#define SPI_IF_WRITE_STD		(0x01)
#define SPI_IF_WRITE_DUAL		(0x02)
#define SPI_IF_WRITE_DUAL_ADDR		(0x04)
#define SPI_IF_WRITE_QUAD		(0x08)
#define SPI_IF_WRITE_QUAD_ADDR		(0x10)

#define SPI_IF_ERASE_SECTOR_4K		(0x01)
#define SPI_IF_ERASE_SECTOR_32K		(0x02)
#define SPI_IF_ERASE_SECTOR_64K		(0x04)
#define SPI_IF_ERASE_SECTOR_128K	(0x08)
#define SPI_IF_ERASE_SECTOR_256K	(0x10)

/*****************************************************************************/

/*****************************************************************************/
#define SPI_CMD_READ_STD		0x03	/* Standard read cache */
#define SPI_CMD_READ_STD4B		0x13	/* Standard read cache 4byte mode */
#define SPI_CMD_READ_FAST		0x0B	/* Higher speed read cache */
#define SPI_CMD_READ_FAST4B		0x0C /* Higher speed read cache 4byte mode */
#define SPI_CMD_READ_DUAL		0x3B	/* 2 IO read cache only date */
#define SPI_CMD_READ_DUAL4B		0x3C /* 2 IO read cache only date 4byte mode*/
#define SPI_CMD_READ_DUAL_ADDR	0xBB	/* 2 IO read cache date&addr */
#define SPI_CMD_READ_DUAL_ADDR4B 0xBC /* 2 IO read cache date&addr 4byte mode */
#define SPI_CMD_READ_QUAD		0x6B	/* 4 IO read cache only date */
#define SPI_CMD_READ_QUAD4B		0x6C /* 4 IO read cache only date 4byte mode */
#define SPI_CMD_READ_QUAD_ADDR	0xEB	/* 4 IO read cache date&addr */
#define SPI_CMD_READ_QUAD_ADDR4B 0xEC /* 4 IO read cache date&addr 4byte mode */
#define SPI_CMD_READ_QUAD_DTR		0xED	/* 4DTR MODE */
#define SPI_CMD_READ_QUAD_DTR4B		0xEE	/* 4DTR MODE 4byte mode */
#define SPI_CMD_READ_QUAD_DTR4B_WINBOND		0xEC	/* 4DTR MODE */

#define SPI_CMD_WRITE_STD		0x02	/* Standard page program */
#define SPI_CMD_WRITE_STD4B		0x12	/* Standard page program 4byte mode */
#define SPI_CMD_WRITE_DUAL		0xA2	/* 2 IO program only date */
#define SPI_CMD_WRITE_DUAL4B	0xA2	/* 2 IO program only date 4byte mode*/
#define SPI_CMD_WRITE_DUAL_ADDR		0xD2	/* 2 IO program date&addr */
#define SPI_CMD_WRITE_DUAL_ADDR4B	0xD2 /* 2 IO program date&addr 4byte mode */
#define SPI_CMD_WRITE_QUAD		0x32	/* 4 IO program only date */
#define SPI_CMD_WRITE_QUAD4B	0x34	/* 4 IO program only date 4byte mode */
#define SPI_CMD_WRITE_QUAD_ADDR		0x38	/* 4 IO program date&addr */
#define SPI_CMD_WRITE_QUAD_ADDR4B	0x3E /* 4 IO program date&addr 4byte mode*/

#define SPI_CMD_SE_4K			0x20	/* 4KB sector Erase */
#define SPI_CMD_SE_4K4B			0x21	/* 4KB sector Erase 4byte mode */
#define SPI_CMD_SE_32K			0x52	/* 32KB sector Erase */
#define SPI_CMD_SE_32K4B		0x5C	/* 32KB sector Erase 4byte mode */
#define SPI_CMD_SE_64K			0xD8	/* 64KB sector Erase */
#define SPI_CMD_SE_64K4B		0xDC	/* 64KB sector Erase 4byte mode */
#define SPI_CMD_SE_128K			0xD8	/* 128KB sector Erase */
#define SPI_CMD_SE_128K4B		0xD8	/* 128KB sector Erase 4byte mode */
#define SPI_CMD_SE_256K			0xD8	/* 256KB sector Erase */
#define SPI_CMD_SE_256K4B		0xD8	/* 256KB sector Erase 4byte mode */

/*****************************************************************************/
#define SET_READ_STD(_dummy_, _size_, _clk_) \
	static struct spi_op read_std_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_STD, SPI_CMD_READ_STD, _dummy_, _size_, _clk_ }

#define SET_READ_STD4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_std4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_STD, SPI_CMD_READ_STD4B, _dummy_, _size_, _clk_ }

#define SET_READ_FAST(_dummy_, _size_, _clk_) \
	static struct spi_op read_fast_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_FAST, SPI_CMD_READ_FAST, _dummy_, _size_, _clk_ }

#define SET_READ_FAST4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_fast4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_FAST, SPI_CMD_READ_FAST4B, _dummy_, _size_, _clk_ }

#define SET_READ_DUAL(_dummy_, _size_, _clk_) \
	static struct spi_op read_dual_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL, SPI_CMD_READ_DUAL, _dummy_, _size_, _clk_ }

#define SET_READ_DUAL4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_dual4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL, SPI_CMD_READ_DUAL4B, _dummy_, _size_, _clk_ }

#define SET_READ_DUAL_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_op read_dual_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL_ADDR, SPI_CMD_READ_DUAL_ADDR, _dummy_, _size_, _clk_ }

#define SET_READ_DUAL_ADDR4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_dual_addr4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_DUAL_ADDR, SPI_CMD_READ_DUAL_ADDR4B, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD, SPI_CMD_READ_QUAD, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD, SPI_CMD_READ_QUAD4B, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_addr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD_ADDR, SPI_CMD_READ_QUAD_ADDR, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD_ADDR4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_addr4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD_ADDR, SPI_CMD_READ_QUAD_ADDR4B, _dummy_, _size_, _clk_ }

#ifdef CONFIG_DTR_MODE_SUPPORT
#define SET_READ_QUAD_DTR(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_dtr_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD_DTR, SPI_CMD_READ_QUAD_DTR, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD_DTR4B(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_dtr4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_READ_QUAD_DTR, SPI_CMD_READ_QUAD_DTR4B, _dummy_, _size_, _clk_ }

#define SET_READ_QUAD_DTR4B_WINBOND(_dummy_, _size_, _clk_) \
	static struct spi_op read_quad_dtr_winbond_##_dummy_##_size_##_clk_ = \
	{SPI_IF_READ_QUAD_DTR, SPI_CMD_READ_QUAD_DTR4B_WINBOND, \
		_dummy_, _size_, _clk_ }
#endif

/*****************************************************************************/
#define SET_WRITE_STD(_dummy_, _size_, _clk_) \
	static struct spi_op write_std_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_STD, SPI_CMD_WRITE_STD, _dummy_, _size_, _clk_ }

#define SET_WRITE_STD4B(_dummy_, _size_, _clk_) \
	static struct spi_op write_std4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_STD, SPI_CMD_WRITE_STD4B, _dummy_, _size_, _clk_ }

#define SET_WRITE_DUAL(_dummy_, _size_, _clk_) \
	static struct spi_op write_dual_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_DUAL, SPI_CMD_WRITE_DUAL, _dummy_, _size_, _clk_ }

#define SET_WRITE_DUAL4B(_dummy_, _size_, _clk_) \
	static struct spi_op write_dual4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_DUAL, SPI_CMD_WRITE_DUAL4B, _dummy_, _size_, _clk_ }

#define SET_WRITE_DUAL_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_op write_dual_addr_##_dummy_##_size_##_clk_ = { \
SPI_IF_WRITE_DUAL_ADDR, SPI_CMD_WRITE_DUAL_ADDR, _dummy_, _size_, _clk_ }

#define SET_WRITE_DUAL_ADDR4B(_dummy_, _size_, _clk_) \
	static struct spi_op write_dual_addr4b_##_dummy_##_size_##_clk_ = { \
SPI_IF_WRITE_DUAL_ADDR, SPI_CMD_WRITE_DUAL_ADDR4B, _dummy_, _size_, _clk_ }

#define SET_WRITE_QUAD(_dummy_, _size_, _clk_) \
	static struct spi_op write_quad_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_QUAD, SPI_CMD_WRITE_QUAD, _dummy_, _size_, _clk_ }

#define SET_WRITE_QUAD4B(_dummy_, _size_, _clk_) \
	static struct spi_op write_quad4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_WRITE_QUAD, SPI_CMD_WRITE_QUAD4B, _dummy_, _size_, _clk_ }

#define SET_WRITE_QUAD_ADDR(_dummy_, _size_, _clk_) \
	static struct spi_op write_quad_addr_##_dummy_##_size_##_clk_ = { \
SPI_IF_WRITE_QUAD_ADDR, SPI_CMD_WRITE_QUAD_ADDR, _dummy_, _size_, _clk_ }

#define SET_WRITE_QUAD_ADDR4B(_dummy_, _size_, _clk_) \
	static struct spi_op write_quad_addr4b_##_dummy_##_size_##_clk_ = { \
SPI_IF_WRITE_QUAD_ADDR, SPI_CMD_WRITE_QUAD_ADDR4B, _dummy_, _size_, _clk_ }

/*****************************************************************************/
#define SET_ERASE_SECTOR_4K(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_4k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_4K, SPI_CMD_SE_4K, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_4K4B(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_4k4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_4K, SPI_CMD_SE_4K4B, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_32K(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_32k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_32K, SPI_CMD_SE_32K, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_32K4B(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_32k4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_32K, SPI_CMD_SE_32K4B, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_64K(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_64k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_64K, SPI_CMD_SE_64K, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_64K4B(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_64k4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_64K, SPI_CMD_SE_64K4B, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_128K(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_128k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_128K, SPI_CMD_SE_128K, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_128K4B(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_128k4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_128K, SPI_CMD_SE_128K4B, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_256K(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_256k_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_256K, SPI_CMD_SE_256K, _dummy_, _size_, _clk_ }

#define SET_ERASE_SECTOR_256K4B(_dummy_, _size_, _clk_) \
	static struct spi_op erase_sector_256k4b_##_dummy_##_size_##_clk_ = { \
	SPI_IF_ERASE_SECTOR_256K, SPI_CMD_SE_256K4B, _dummy_, _size_, _clk_ }

/*****************************************************************************/
#define READ_STD(_dummy_, _size_, _clk_) read_std_##_dummy_##_size_##_clk_
#define READ_STD4B(_dummy_, _size_, _clk_) read_std4b_##_dummy_##_size_##_clk_
#define READ_FAST(_dummy_, _size_, _clk_) read_fast_##_dummy_##_size_##_clk_
#define READ_FAST4B(_dummy_, _size_, _clk_) read_fast4b_##_dummy_##_size_##_clk_
#define READ_DUAL(_dummy_, _size_, _clk_) read_dual_##_dummy_##_size_##_clk_
#define READ_DUAL4B(_dummy_, _size_, _clk_) read_dual4b_##_dummy_##_size_##_clk_
#define READ_DUAL_ADDR(_dummy_, _size_, _clk_) \
		read_dual_addr_##_dummy_##_size_##_clk_
#define READ_DUAL_ADDR4B(_dummy_, _size_, _clk_) \
		read_dual_addr4b_##_dummy_##_size_##_clk_
#define READ_QUAD(_dummy_, _size_, _clk_) read_quad_##_dummy_##_size_##_clk_
#define READ_QUAD4B(_dummy_, _size_, _clk_) read_quad4b_##_dummy_##_size_##_clk_
#define READ_QUAD_ADDR(_dummy_, _size_, _clk_) \
		read_quad_addr_##_dummy_##_size_##_clk_
#define READ_QUAD_ADDR4B(_dummy_, _size_, _clk_) \
		read_quad_addr4b_##_dummy_##_size_##_clk_
#ifdef CONFIG_DTR_MODE_SUPPORT
#define READ_QUAD_DTR(_dummy_, _size_, _clk_) \
		read_quad_dtr_##_dummy_##_size_##_clk_
#define READ_QUAD_DTR4B(_dummy_, _size_, _clk_) \
		read_quad_dtr4b_##_dummy_##_size_##_clk_
#define READ_QUAD_DTR4B_WINBOND(_dummy_, _size_, _clk_) \
		read_quad_dtr4b_winbond_##_dummy_##_size_##_clk_
#endif

/*****************************************************************************/
#define WRITE_STD(_dummy_, _size_, _clk_) write_std_##_dummy_##_size_##_clk_
#define WRITE_STD4B(_dummy_, _size_, _clk_) write_std4b_##_dummy_##_size_##_clk_
#define WRITE_DUAL(_dummy_, _size_, _clk_) write_dual_##_dummy_##_size_##_clk_
#define WRITE_DUAL4B(_dummy_, _size_, _clk_) write_dual4b_##_dummy_##_size_##_clk_
#define WRITE_DUAL_ADDR(_dummy_, _size_, _clk_) \
		write_dual_addr_##_dummy_##_size_##_clk_
#define WRITE_DUAL_ADDR4B(_dummy_, _size_, _clk_) \
		write_dual_addr4b_##_dummy_##_size_##_clk_
#define WRITE_QUAD(_dummy_, _size_, _clk_) write_quad_##_dummy_##_size_##_clk_
#define WRITE_QUAD4B(_dummy_, _size_, _clk_) write_quad4b_##_dummy_##_size_##_clk_
#define WRITE_QUAD_ADDR(_dummy_, _size_, _clk_) \
		write_quad_addr_##_dummy_##_size_##_clk_
#define WRITE_QUAD_ADDR4B(_dummy_, _size_, _clk_) \
		write_quad_addr4b_##_dummy_##_size_##_clk_

/*****************************************************************************/
#define ERASE_SECTOR_4K(_dummy_, _size_, _clk_) \
		erase_sector_4k_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_4K4B(_dummy_, _size_, _clk_) \
		erase_sector_4k4b_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_32K(_dummy_, _size_, _clk_) \
		erase_sector_32k_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_32K4B(_dummy_, _size_, _clk_) \
		erase_sector_32k4b_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_64K(_dummy_, _size_, _clk_) \
		erase_sector_64k_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_64K4B(_dummy_, _size_, _clk_) \
		erase_sector_64k4b_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_128K(_dummy_, _size_, _clk_) \
		erase_sector_128k_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_128K4B(_dummy_, _size_, _clk_) \
		erase_sector_128k4b_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_256K(_dummy_, _size_, _clk_) \
		erase_sector_256k_##_dummy_##_size_##_clk_
#define ERASE_SECTOR_256K4B(_dummy_, _size_, _clk_) \
		erase_sector_256k4b_##_dummy_##_size_##_clk_

/*****************************************************************************/
#define SPI_CMD_WREN			0x06	/* Write Enable */
#define SPI_CMD_WRDI			0x04	/* Write Disable */

/*****************************************************************************/
#define SPI_CMD_WRSR			0x01	/* Write Status Register */
#define SPI_CMD_WRSR2			0x31	/* Write Status Register-2 */
#define SPI_CMD_WRSR3			0x11	/* Write Status Register-3 */

#define SPI_CMD_RDSR			0x05	/* Read Status Register */
#define SPI_CMD_RDSR2			0x35	/* Read Status Register-2 */
#define SPI_CMD_RDSR3			0x15	/* Read Status Register-3 */

#define SPI_CMD_RDCR			0x35	/* Read Config Register */

#define SPI_CMD_RDID			0x9F	/* Read Identification */

#define SPI_CMD_RD_SFDP			0x5A	/* Read SFDP */
/*****************************************************************************/
#define SPI_CMD_GET_FEATURES		0x0F	/* Get Features */
#define SPI_CMD_SET_FEATURE		0x1F	/* Set Feature */

#define SPI_CMD_PAGE_READ		0x13	/* Page Read to Cache */

#define SPI_CMD_RESET			0xff	/* Reset the device */

/*****************************************************************************/
#define SPI_CMD_EN4B	0xB7 /* enter 4 bytes mode and set 4 byte bit as '1' */
#define SPI_CMD_EX4B	0xE9	/* exit 4 bytes mode and clear 4 byte bit */

/*****************************************************************************/
#define MAX_SPI_OP			8

/*****************************************************************************/
/* SPI general operation parameter */
struct spi_op {
	unsigned char iftype;
	unsigned char cmd;
	unsigned char dummy;
	unsigned int size;
	unsigned int clock;
};

struct spi;
/* SPI interface special operation function hook */
struct spi_drv {
    int (*wait_ready)(struct spi *spi);
    int (*write_enable)(struct spi *spi);
    int (*qe_enable)(struct spi *spi);
    int (*bus_prepare)(struct spi *spi, int op);
    int (*entry_4addr)(struct spi *spi, int en);
};

/* SPI interface all operation */
struct spi {
    char *name;
    unsigned int cs;
    unsigned long long chipsize;
    unsigned long long  erasesize;
    unsigned int addrcycle;

    struct spi_op read[MAX_SPI_OP];
    struct spi_op write[MAX_SPI_OP];
    struct spi_op erase[MAX_SPI_OP];

    void *host;

    struct spi_drv *driver;
};

#endif /* __SPI_COMMON_H__ */

