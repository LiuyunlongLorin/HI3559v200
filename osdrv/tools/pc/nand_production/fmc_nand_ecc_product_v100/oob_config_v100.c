/******************************************************************************
*    Copyright (c) 2009-2010 by Hisi
*    All rights reserved.
* ***
*    written by Czy. 2010-10-13 14:01:24
*
******************************************************************************/

#include "oob_config.h"


#define NULL    (void *)0

/*****************************************************************************/
const char *nand_controller_version = "Flash Memory Controller V100";

/*****************************************************************************/
struct nand_oob_free oobfree_def[] =
{
    {2, 30}, {0, 0},
};

struct nand_oob_free oobfree_def_2k16bit[] =
{
    {2, 6}, {0, 0},
};

struct nand_oob_free oobfree_def_4k16bit[] =
{
    {2, 14}, {0, 0},
};

/*****************************************************************************/
static struct oob_info hifmc100_oob_info[] =
{
    {NANDIP_V100, pt_pagesize_2K, et_ecc_8bit1k,   64, oobfree_def, 0},
    {NANDIP_V100, pt_pagesize_2K, et_ecc_16bit1k,  64, oobfree_def_2k16bit, 0},
    {NANDIP_V100, pt_pagesize_2K, et_ecc_24bit1k, 128, oobfree_def, 0},

    {NANDIP_V100, pt_pagesize_4K, et_ecc_8bit1k,  128, oobfree_def, 0},
    {NANDIP_V100, pt_pagesize_4K, et_ecc_16bit1k, 128, oobfree_def_4k16bit, 0},
    {NANDIP_V100, pt_pagesize_4K, et_ecc_24bit1k, 200, oobfree_def, 0},

    {0},
};

/*****************************************************************************/
static struct oobuse_info hifmc100_oobuse_info[] =
{
    {NANDIP_V100, pt_pagesize_2K, et_ecc_8bit1k,     60},
    {NANDIP_V100, pt_pagesize_2K, et_ecc_16bit1k,    64},
    {NANDIP_V100, pt_pagesize_2K, et_ecc_24bit1k,   116},

    {NANDIP_V100, pt_pagesize_4K, et_ecc_8bit1k,     88},
    {NANDIP_V100, pt_pagesize_4K, et_ecc_16bit1k,   128},
    {NANDIP_V100, pt_pagesize_4K, et_ecc_24bit1k,   200},

    {0},
};

/*****************************************************************************/
static struct reverse_pagesize_ecctype_sector hifmc100_page_ecc_reverse_sector[] =
{
    {NANDIP_V100, pt_pagesize_2K,  et_ecc_8bit1k,    0},
    {NANDIP_V100, pt_pagesize_2K,  et_ecc_24bit1k,   1},
    {NANDIP_V100, pt_pagesize_4K,  et_ecc_8bit1k,    2},
    {NANDIP_V100, pt_pagesize_4K,  et_ecc_24bit1k,   3},

    {0},
};

/*****************************************************************************/
static struct reverse_blocksize_sector hifmc100_blocksize_reverse_sector[] =
{
    {NANDIP_V100, pt_blocksize_64,     0},
    {NANDIP_V100, pt_blocksize_128,    1},
    {NANDIP_V100, pt_blocksize_256,    2},
    {NANDIP_V100, pt_blocksize_512,    3},

    {0},
};
/*****************************************************************************/

struct oob_info *get_n_oob_info(int selectindex)
{
    struct oob_info *info = hifmc100_oob_info; 
    int index = 0;
    
    for (; info->freeoob; info++, index++)
    {
        if (index == selectindex)
        {
            return info;
        }
    }

    return 0;
}
/*****************************************************************************/

struct oob_info * get_oob_info(enum page_type pagetype, 
    enum ecc_type ecctype)
{
    struct oob_info *info = hifmc100_oob_info; 
    
    for (; info->freeoob; info++)
    {
        if (info->ecctype == ecctype
            && info->pagetype == pagetype)
        {
            return info;
        }
    }

    return NULL;
}

/*****************************************************************************/
struct oobuse_info * get_oobuse_info(enum page_type pagetype, 
    enum ecc_type ecctype)
{
    struct oobuse_info *info = hifmc100_oobuse_info; 
    
    for (; info->nandip; info++)
    {
        if (info->ecctype == ecctype
            && info->pagetype == pagetype)
        {
            return info;
        }
    }

    return NULL;
}

/*****************************************************************************/
struct reverse_pagesize_ecctype_sector* get_pagesize_ecctype_reverse_sector(enum page_type pagetype, 
    enum ecc_type ecctype)
{
    struct reverse_pagesize_ecctype_sector *sector = hifmc100_page_ecc_reverse_sector; 
    
    for (; sector->nandip; sector++)
    {
        if (sector->ecctype == ecctype
            && sector->pagetype == pagetype)
        {
            return sector;
        }
    }

    return NULL;
}

/*****************************************************************************/
struct reverse_blocksize_sector* get_blocksize_reverse_sector(unsigned int blocksize) 
{
    struct reverse_blocksize_sector *sector = hifmc100_blocksize_reverse_sector; 
    
    for (; sector->nandip; sector++)
    {
        if (sector->blocksize == blocksize)
        {
            return sector;
        }
    }

    return NULL;
}

/*****************************************************************************/
char *get_ecctype_str(enum ecc_type ecctype)
{
    char *ecctype_str[et_ecc_last+1] = { (char *)"None", (char *)"8bit/1K",
    (char *)"16bit/1K", (char *)"24bits/1K", (char *)"28bits/1K",
    (char *)"40bits/1K", (char *)"64bits/1K"};
    if (ecctype < et_ecc_8bit1k || ecctype > et_ecc_last)
    {
        ecctype = et_ecc_last;
    }
    return ecctype_str[ecctype];
}
/*****************************************************************************/

char *get_pagesize_str(enum page_type pagetype)
{
    char *pagesize_str[pt_pagesize_last+1] = {(char *)"2K",
        (char *)"4K", (char *)"8K", (char *)"16K", (char *)"unknown" };
    if (pagetype < pt_pagesize_2K || pagetype >= pt_pagesize_last)
    {
        pagetype = pt_pagesize_last;
    }
    return pagesize_str[pagetype];
}
/*****************************************************************************/

unsigned int get_pagesize(enum page_type pagetype)
{
    unsigned int pagesize[] = {2048, 4096, 8192, 16384, 0};
    return pagesize[pagetype];
}
