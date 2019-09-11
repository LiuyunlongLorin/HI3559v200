/******************************************************************************
*    Copyright (c) 2009-2010 by czy.
*    All rights reserved.
* ***
*    written by CaiZhiYong. 2010-10-13 14:01:19
*
******************************************************************************/

#ifndef OOB_CONFIGH
#define OOB_CONFIGH

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif

#define MAX_PAGE_SIZE	16384	/* 必须大于或等于器件实际的PAGESIZE大小 */
#define MAX_OOB_SIZE	2000    /* 必须大于或等于器件实际的OOB区域大小 */	

#define NANDIP_V100    0x100
#define NANDIP_V300    0x300
#define NANDIP_V301    0x301
#define NANDIP_V504    0x504
#define NANDIP_V610    0x610
/******************************************************************************/

/* this follow must be consistent with fastboot !!! */
enum ecc_type
{
    et_ecc_none    = 0x00,
    et_ecc_8bit1k  = 0x01,
    et_ecc_16bit1k = 0x02,
    et_ecc_24bit1k = 0x03,
    et_ecc_28bit1k = 0x04,
    et_ecc_40bit1k = 0x05,
    et_ecc_64bit1k = 0x06,
    et_ecc_last    = 0x07,
};
/*****************************************************************************/

enum page_type
{
    pt_pagesize_2K    = 0x00,
    pt_pagesize_4K    = 0x01,
    pt_pagesize_8K    = 0x02,
    pt_pagesize_16K   = 0x03,
    pt_pagesize_last  = 0x04,
};

/*****************************************************************************/
enum blocksize_type
{
    pt_blocksize_64    = 64,
    pt_blocksize_128   = 128,
    pt_blocksize_256   = 256,
    pt_blocksize_512   = 512,
    pt_blocksize_last  = 0x04,
};

/*****************************************************************************/
struct nand_oob_free
{
	unsigned long offset;
	unsigned long length;
};
/*****************************************************************************/

struct oob_info
{
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int oobsize;
	struct nand_oob_free *freeoob;
	int random;
};
struct reverse_pagesize_ecctype_sector
{
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int reverse_sec;
};

struct reverse_blocksize_sector
{
	unsigned int nandip;
	enum blocksize_type blocksize;
	unsigned int reverse_sec;
};
struct oobuse_info
{
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int oobuse;
};
/*****************************************************************************/

struct reverse_pagesize_ecctype_sector * get_pagesize_ecctype_reverse_sector(enum page_type pagetype, 
							enum ecc_type ecctype);
struct reverse_blocksize_sector * get_blocksize_reverse_sector(enum blocksize_type blocktype);

EXTERN_C struct oob_info *get_n_oob_info(int selectindex);

struct oob_info * get_oob_info(enum page_type pagetype, 
    enum ecc_type ecctype);

EXTERN_C char *get_ecctype_str(enum ecc_type ecctype);

EXTERN_C char *get_pagesize_str(enum page_type pagetype);

EXTERN_C unsigned int get_pagesize(enum page_type pagetype);

extern const char *nand_controller_version;

/******************************************************************************/
#endif /* OOB_CONFIGH */
