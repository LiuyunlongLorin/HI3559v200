#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "nandc_ecc.h"

#define REVERSE_DATA_LENTH         32

struct oobuse_info * get_oobuse_info(enum page_type pagetype, enum ecc_type ecctype);

static int reverse_buf(unsigned char *buf, unsigned int buflen)
{
	unsigned int i;
	for (i = 0; i < buflen; i++)
		buf[i] = ~buf[i];

	return 0;
}

int main(int argc, char **argv)
{
	FILE *infile = NULL;
	FILE *outfile = NULL;
	char *infilepath, *outfilepath;
	struct oob_info *info;
	struct oobuse_info *oobuseinfo;
	unsigned int pagetype, ecctype, yaffs;

	/* for nandcv610/nandcv620/fmcv100 省管脚模式 */
	struct reverse_pagesize_ecctype_sector *reverse_page_ecc_sector;
	struct reverse_blocksize_sector *reverse_block_sector;
	unsigned int page_ecc_reverse_sector;
	unsigned int block_reverse_sector;
	unsigned int page0_reverse_flag = 0;
	unsigned int page1_reverse_flag = 0;

	/* oobsize: 器件实际的oob区大小 
	 * oobuse: 逻辑实际使用的OOB区大小
	 * info->oobsize: 原始镜像中预留给软件使用的oobsize，NANDCV610是32字节，如果是yaffs镜像，需要与pagesize+info->oobsize对齐。
	 */
	unsigned int oobsize, oobuse;
	unsigned int infilesize, ret, pagesize, pageindex = 0, pagecount, random, pagenum, savepin;
	unsigned int page_no_total_block, page_no_incl_data, page_no_fill_null;
	unsigned char pagebuf[MAX_PAGE_SIZE + MAX_OOB_SIZE];
	int i, j;

	if (argc != 10) {
		printf("Usage:\n%s\tinputfile\toutputfile\t"
		       "pagetype\tecctype\toobsize\tyaffs\trandomizer\tpagenum/block\t"
		       "save_pin:\n"
		       "For example:\t\t |\t\t |\t\t |\t\t |\t |\t |\t |\t\t |\t\t |\n"
		       "%s\tin_2k4b.yaffs\tout_2k4b.yaffs\t"
		       " 0\t\t 1\t 64\t 1\t 0\t\t 64\t\t 0\n"
		       "Page type Page size:\n"
		       "Input file:\n"
		       "Output file:\n"
		       "Pagetype:\n"
		       "0        2KB\n"
		       "1        4KB\n"
		       "ECC type ECC size:\n"
		       "1        4bit/512B\n"
		       "2        16bit/1K\n"
		       "3        24bit/1K\n"
		       "Chip OOB size:\n"
		       "yaffs2 image format:\n"
		       "0	 NO\n"
		       "1	 YES\n"
		       "Randomizer:\n"
		       "0        randomizer_disabled\n"
		       "1        randomizer_enabled\n"
		       "Pages_per_block:\n"
		       "64       64pages/block\n"
		       "128      128pages/block\n"
		       "Save Pin Mode:\n"
		       "0	 disable\n"
		       "1	 enable\n", argv[0], argv[0]);
		return -EINVAL;
	}

	infilepath = argv[1];
	outfilepath = argv[2];
	pagetype = strtol(argv[3], NULL, 10);
	ecctype = strtol(argv[4], NULL, 10);
	oobsize = strtol(argv[5], NULL, 10); /* real size of chip OOB area */
	yaffs = strtol(argv[6], NULL, 10);
	random = strtol(argv[7], NULL, 10);
	/* random_pagenum = 64/128/512 */
	pagenum = strtol(argv[8], NULL, 10);
	/* save pin mode, only for nandcv620/fmcv100 */
	savepin = strtol(argv[9], NULL, 10);

	printf("pagetype=%d,ecctype=%d,oobsize=%d,yaffs=%d,randomizer=%d,pagenum=%d,savepin=%d,infilepath=%s,outfilepath=%s\n", 
		pagetype, ecctype, oobsize, yaffs, random, pagenum, savepin, 
		infilepath, outfilepath);

	info = get_oob_info(pagetype, ecctype);
	/* for nandc v610/v620/ and fmcv100, info->oobsize = 32 */
	if(info == NULL)
	{
		fprintf(stderr, "Not support this parameter page: %d ecc: %d\n",
				pagetype, ecctype);
		return -EINVAL;
	}
	/* for nandcv610/nandcv620/fmcv100 省管脚模式，得到需要数据取反的sector number */
	if (savepin) {

		reverse_page_ecc_sector = get_pagesize_ecctype_reverse_sector(pagetype, ecctype);
		/* for nandc v610/v620 and fmcv100 省管脚模式, info->oobsize = 32 */
		if(reverse_page_ecc_sector == NULL)
		{
			fprintf(stderr, "get reverse sector failed.\n");
			return -EINVAL;
		}
		page_ecc_reverse_sector = reverse_page_ecc_sector->reverse_sec;
		reverse_block_sector = get_blocksize_reverse_sector(pagenum);
		/* for nandc v610/v620 and fmcv100 省管脚模式, info->oobsize = 32 */
		if(reverse_block_sector == NULL)
		{
			fprintf(stderr, "get block sector failed.\n");
			return -EINVAL;
		}
		block_reverse_sector = reverse_block_sector->reverse_sec;
	}
	if(oobsize < info->oobsize)
	{
		fprintf(stderr, "Chip OOB size too small.\n");
		return -EINVAL;
	}

	pagesize = get_pagesize(pagetype);
	printf("pagesize=%d\n", pagesize);
	infile = fopen(infilepath, "rb");
	if (infile == NULL)
	{
		fprintf(stderr, "Could not open input file %s\n", infilepath);
		return errno;
	}

	fseek(infile, 0, SEEK_END);
	infilesize = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	printf("infilesize=0x%x\n", infilesize);
	if(yaffs) {
		if ((infilesize == 0) || ((infilesize % (pagesize + info->oobsize)) != 0))
		{
			fprintf(stderr, "Input file length is not page + oob aligned."
			"infilesize=%d, pagesize=%d oobsize=%d\n",
			infilesize, pagesize, info->oobsize);
			ret = -EINVAL;
			goto err;
		}
		pagecount = infilesize / (pagesize + info->oobsize);
	} else {
		pagecount = (infilesize + pagesize -1) / pagesize;
	}
	page_no_incl_data = pagecount;

	outfile = fopen(outfilepath, "wb");
	if (outfile == NULL)
	{
		fprintf(stderr, "Could not open output file %s\n", outfilepath);
		ret = errno;
		goto err;
	}

	printf("pagecount=%d\n", pagecount);
	printf("outfilepath=%s\n", outfilepath);
	while(pagecount--)
	{
		memset(pagebuf, 0xFF, sizeof(pagebuf));

		if(!yaffs) {
			ret = fread(pagebuf, 1, pagesize, infile);
			memset(pagebuf + pagesize, 0xff, info->oobsize);
			/* for nandcv610/nandcv620 only, empty page flag. */
			if (ecctype != et_ecc_16bit1k) {
				*(pagebuf + pagesize + 32 - 1) = 0;
				*(pagebuf + pagesize + 32 - 2) = 0;
			} else {
				if (pagesize == 2048) {
					*(pagebuf + pagesize + 8 - 1) = 0;
					*(pagebuf + pagesize + 8 - 2) = 0;
				} else if (pagesize == 4096) {
					*(pagebuf + pagesize + 16 - 1) = 0;
					*(pagebuf + pagesize + 16 - 2) = 0;
				}
			}
		} else {
			ret = fread(pagebuf, 1, pagesize + info->oobsize, infile);
			/* for nandcv610/nandcv620 only, empty page flag.*/
			*(pagebuf + pagesize + 32 - 1) = 0;
			*(pagebuf + pagesize + 32 - 2) = 0;
		}

		if(ret == 0)
			break;

		if(ret < 0)
		{
			fprintf(stderr, "Could not read input file %s\n", infilepath);
			ret = errno;
			goto err;
		}

		page_ecc_gen(pagebuf, pagetype, ecctype);

		/* randomzer enable. */
		/* 1. 首先需要获取逻辑实际使用的OOB大小
		 * 2. 其次按照逻辑实际使用的OOB大小进行数据randomizer。
		 */
		oobuseinfo = get_oobuse_info(pagetype, ecctype);
		if (oobsize < oobuseinfo->oobuse) {
			fprintf(stderr, "CHIP OOBSIZE too small.\n");
			return -EINVAL;
		}
		if(random)
			page_random_gen(pagebuf, pagetype, ecctype, pageindex%pagenum, oobuseinfo->oobuse);

		/* for nandcv610/nandcv620省管脚模式，分别对pag0(pagesize和ecc组合)和page1(blocksize)的sector数据取反 */
		/* 1. 先对page0的对应sector数据取反 */
		if (savepin && (!page0_reverse_flag)) {
			unsigned char *reverse_sector;
			reverse_sector = pagebuf + REVERSE_DATA_LENTH * page_ecc_reverse_sector;
			reverse_buf(reverse_sector, REVERSE_DATA_LENTH);
		}
		/* 2. page0的数据取反之后，再对page1的数据取反 */
		if (savepin && page0_reverse_flag && (!page1_reverse_flag)) {
			unsigned char *reverse_sector;
			reverse_sector = pagebuf + REVERSE_DATA_LENTH * block_reverse_sector;
			reverse_buf(reverse_sector, REVERSE_DATA_LENTH);
			page1_reverse_flag = 1;
		}
		page0_reverse_flag = 1;
#if 0
		for (j = 0; j < 14; j ++) {
			printf("===========================\n");
			for (i = 0; i < 70; i ++)
				 printf("%02x ", *(pagebuf + 1026 + (1026+70)*j + i));
			printf("\n");
		}

#endif
//		printf("begin to write: pagesize=%d, oobsize=%d\n", pagesize, oobsize);
		ret = fwrite(pagebuf, 1, pagesize + oobsize, outfile);
//		printf("exit to write, writelen=%d!\n", ret);

		if(ret != (pagesize + oobsize))
		{
			fprintf(stderr, "Could not write output file %s\n", outfilepath);
			ret = errno;
			goto err;
		}
		pageindex++;
	}

	page_no_total_block = page_no_incl_data & ~(pagenum - 1);
	page_no_total_block += (page_no_incl_data & (pagenum - 1)) ? pagenum : 0;
	page_no_fill_null = page_no_total_block - page_no_incl_data;

	printf("Total block page number:%d\n", page_no_total_block);
	printf("Include data page number:%d\n", page_no_incl_data);
	printf("Need fill NULL page number:%d\n", page_no_fill_null);

	while(page_no_fill_null--)
	{
		memset(pagebuf, 0xff, sizeof(pagebuf));

		if(!yaffs)
			memset(pagebuf + pagesize, 0xff, info->oobsize);

		if (ecctype != et_ecc_16bit1k) {
			*(pagebuf + pagesize + 32 - 1) = 0;
			*(pagebuf + pagesize + 32 - 2) = 0;
		} else {
			if (pagesize == 2048) {
				*(pagebuf + pagesize + 8 - 1) = 0;
				*(pagebuf + pagesize + 8 - 2) = 0;
			} else if (pagesize == 4096) {
				*(pagebuf + pagesize + 16 - 1) = 0;
				*(pagebuf + pagesize + 16 - 2) = 0;
			}
		}

		page_ecc_gen(pagebuf, pagetype, ecctype);
		ret = fwrite(pagebuf, 1, pagesize + oobsize, outfile);
		if(ret != (pagesize + oobsize))
		{
			fprintf(stderr, "Could not fill NULL page in output file %s\n", outfilepath);
			ret = errno;
			goto err;
		}
	}

	ret = 0;
	if (savepin && ((!page0_reverse_flag) || (!page1_reverse_flag))) {
		printf("savepin mode: reverse data failed.\n");
		ret = -1;
	}
err:
	if(infile)
		fclose(infile);
	if(outfile)
		fclose(outfile);
	return ret;
}

