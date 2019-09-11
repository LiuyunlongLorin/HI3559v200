
#include "nandc_ecc.h"
#include <stdio.h>

//enc_data_last[24] 初始值 0xFFFFFF ;
// [  23    22    21     20   19    18   17    16   15    14  ]
//  P2048 P2048' P1024 P1024' P512 P512' P256 P256' P128 P128'
// [ 13  12   11  10   9   8    7  6  5   4  3   2  1   0 ]
//  P64 P64' P32 P32' P16 P16' P8 P8' P4 P4' P2 P2' P1 P1'
//addr 表示第几个字节
void make_ecc_1bit(unsigned int addr, unsigned char data_in, unsigned char *enc_data, unsigned char *enc_data_last)
{
	int i = 0;
	// ecc_data_in to ECC calculate module
	unsigned char ecc_l1[8] ;
	unsigned char ecc_h1[8] ;
	unsigned char nxt_ecc_reg[24];
	unsigned char ecc_p4ol;
	unsigned char ecc_p4el;
	unsigned char ecc_p2ol;
	unsigned char ecc_p2el;
	unsigned char ecc_p1ol;
	unsigned char ecc_p1el;
	unsigned char ecc_p4oh;
	unsigned char ecc_p4eh;
	unsigned char ecc_p2oh;
	unsigned char ecc_p2eh;
	unsigned char ecc_p1oh;
	unsigned char ecc_p1eh;
	unsigned char ecc_p8o;
	unsigned char ecc_p8e;
	unsigned char ecc_p4o;
	unsigned char ecc_p4e;
	unsigned char ecc_p2o;
	unsigned char ecc_p2e;
	unsigned char ecc_p1o;
	unsigned char ecc_p1e;
	unsigned char ecc_pr;
	unsigned char ecc_data_in[16] = {0};
	unsigned char addr_in[9] = {0};

	for (i = 0; i < 8; i++)
	{
		ecc_data_in[i] = (data_in >> i)&0x1;
	}

	for (i = 0; i < 9; i++)
	{
		addr_in[i] = (addr >> i)&0x1;
	}
	
	//
    ecc_l1[0] = ecc_data_in[0] ^ ecc_data_in[1];
    ecc_l1[1] = ecc_data_in[0] ^ ecc_data_in[2];
    ecc_l1[2] = ecc_data_in[1] ^ ecc_data_in[3];
    ecc_l1[3] = ecc_data_in[2] ^ ecc_data_in[3];
    ecc_l1[4] = ecc_data_in[4] ^ ecc_data_in[5];
    ecc_l1[5] = ecc_data_in[4] ^ ecc_data_in[6];
    ecc_l1[6] = ecc_data_in[5] ^ ecc_data_in[7];
    ecc_l1[7] = ecc_data_in[6] ^ ecc_data_in[7];

// ---------------------------------------------------------------------
// level one ECC calculate for high 8bit
// ---------------------------------------------------------------------
    ecc_h1[0] = ecc_data_in[8] ^ ecc_data_in[9];
    ecc_h1[1] = ecc_data_in[8] ^ ecc_data_in[10];
    ecc_h1[2] = ecc_data_in[9] ^ ecc_data_in[11];
    ecc_h1[3] = ecc_data_in[10] ^ ecc_data_in[11];
    ecc_h1[4] = ecc_data_in[12] ^ ecc_data_in[13];
    ecc_h1[5] = ecc_data_in[12] ^ ecc_data_in[14];
    ecc_h1[6] = ecc_data_in[13] ^ ecc_data_in[15];
    ecc_h1[7] = ecc_data_in[14] ^ ecc_data_in[15];

// ---------------------------------------------------------------------
// level two ECC calculate for low 8bit
// ---------------------------------------------------------------------
    ecc_p4ol = ecc_l1[1] ^ ecc_l1[2];
    ecc_p2ol = ecc_l1[0] ^ ecc_l1[4];
    ecc_p1ol = ecc_l1[1] ^ ecc_l1[5];
    ecc_p1el = ecc_l1[2] ^ ecc_l1[6];
    ecc_p2el = ecc_l1[3] ^ ecc_l1[7];
    ecc_p4el = ecc_l1[5] ^ ecc_l1[6];

// ---------------------------------------------------------------------
// level two ECC calculate for high 8bit
// ---------------------------------------------------------------------
    ecc_p4oh = ecc_h1[1] ^ ecc_h1[2];
    ecc_p2oh = ecc_h1[0] ^ ecc_h1[4];
    ecc_p1oh = ecc_h1[1] ^ ecc_h1[5];
    ecc_p1eh = ecc_h1[2] ^ ecc_h1[6];
    ecc_p2eh = ecc_h1[3] ^ ecc_h1[7];
    ecc_p4eh = ecc_h1[5] ^ ecc_h1[6];

// ---------------------------------------------------------------------
// level three ECC calculate
// ---------------------------------------------------------------------
    ecc_p1o = ecc_p1ol ^ ecc_p1oh;
    ecc_p1e = ecc_p1el ^ ecc_p1eh;
    ecc_p2o = ecc_p2ol ^ ecc_p2oh;
    ecc_p2e = ecc_p2el ^ ecc_p2eh;
    ecc_p4o = ecc_p4ol ^ ecc_p4oh;
    ecc_p4e = ecc_p4el ^ ecc_p4eh;
    ecc_p8o = ecc_p2ol ^ ecc_p2el;
    ecc_p8e = ecc_p2oh ^ ecc_p2eh;

// ---------------------------------------------------------------------
// line ECC calculate
// ---------------------------------------------------------------------
    ecc_pr  = ecc_p8o  ^ ecc_p8e;

// generate all row and column ecc result for one read/write cylce nxt_ecc_reg[23:0] = 24'h0;
// ---------------------------------------------------------------------
    nxt_ecc_reg[0] = ecc_p1o;
    nxt_ecc_reg[1] = ecc_p1e;
    nxt_ecc_reg[2] = ecc_p2o;
    nxt_ecc_reg[3] = ecc_p2e;
    nxt_ecc_reg[4] = ecc_p4o;
    nxt_ecc_reg[5] = ecc_p4e;
    
    nxt_ecc_reg[6] = (~addr_in[0]) & ecc_pr;
    nxt_ecc_reg[7] =   addr_in[0]  & ecc_pr;
    
    nxt_ecc_reg[8]  = (~addr_in[1]) & ecc_pr;
    nxt_ecc_reg[9]  =   addr_in[1]  & ecc_pr;
    nxt_ecc_reg[10] = (~addr_in[2]) & ecc_pr;
    nxt_ecc_reg[11] =   addr_in[2]  & ecc_pr;
    nxt_ecc_reg[12] = (~addr_in[3]) & ecc_pr;
    nxt_ecc_reg[13] =   addr_in[3]  & ecc_pr;
    nxt_ecc_reg[14] = (~addr_in[4]) & ecc_pr;
    nxt_ecc_reg[15] =   addr_in[4]  & ecc_pr;
    nxt_ecc_reg[16] = (~addr_in[5]) & ecc_pr;
    nxt_ecc_reg[17] =   addr_in[5]  & ecc_pr;
    nxt_ecc_reg[18] = (~addr_in[6]) & ecc_pr;
    nxt_ecc_reg[19] =   addr_in[6]  & ecc_pr;
    nxt_ecc_reg[20] = (~addr_in[7]) & ecc_pr;
    nxt_ecc_reg[21] =   addr_in[7]  & ecc_pr;
    nxt_ecc_reg[22] = (~addr_in[8]) & ecc_pr;
    nxt_ecc_reg[23] =   addr_in[8]  & ecc_pr;

	for (i = 0; i < 24; i++)
	{
		enc_data[i] = enc_data_last[i] ^ nxt_ecc_reg[i];
	}
    return;
}

void ecc_1bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	unsigned int i = 0;
	unsigned char enc_data_last[24] = {0x1};
	unsigned char enc_data[24] ;

	memset(enc_data_last, 0x1, 24);

	for (i = 0; i < len; i++)
	{
		make_ecc_1bit(i, data[i], enc_data, enc_data_last);
		memcpy(enc_data_last, enc_data, 24);
	}

	if (len == 16)
	{
		enc_data[14] = 0x1;
		enc_data[15] = 0x1;
	}
	
	if (len >= 256)
	{
		for (i = 0; i < 3; i++)
		{
			ecc_code[2 - i] = enc_data[i*8]  + (enc_data[i*8 + 1] << 1) + (enc_data[i*8 + 2] << 2) 
				+ (enc_data[i*8 + 3] << 3) + (enc_data[i*8 + 4] << 4) + (enc_data[i*8 + 5] << 5) 
				+ (enc_data[i*8 + 6] << 6) + (enc_data[i*8 + 7] << 7);
		}
	}
	else 
	{
		for (i = 0; i < 2; i++)
		{
			ecc_code[1 - i] = enc_data[i*8]  + (enc_data[i*8 + 1] << 1) + (enc_data[i*8 + 2] << 2) 
				+ (enc_data[i*8 + 3] << 3) + (enc_data[i*8 + 4] << 4) + (enc_data[i*8 + 5] << 5) 
				+ (enc_data[i*8 + 6] << 6) + (enc_data[i*8 + 7] << 7);
		}
	}
}

#define MAX_LFSR_BITS 2048

int lfsr_len;
char lfsr_poly[MAX_LFSR_BITS];
char lfsr_value[MAX_LFSR_BITS];

void inttolfsr(char *lfsr, int value)
{
	int i;
	for(i=0; i <= lfsr_len; i++) {
    if( value & (1 << i)) 
		lfsr[i] = 1;
    else
		lfsr[i] = 0;
  }
}
void strtolfsr(char *lfsr, char *value)
{
	unsigned int i;
	char c;
	size_t len = strlen(value);

	for(i = 0; i < len; i++)
	{
		c = *(value + len - 1 - i);
		if(c == '1')
			lfsr[i] = 1;
		else
			lfsr[i] = 0;
	}
}

lfsr_init(int len, char *poly, int value)
{
	lfsr_len = len;
	memset(lfsr_poly, 0x00, MAX_LFSR_BITS);
	memset(lfsr_value, 0x00, MAX_LFSR_BITS);
	strtolfsr(lfsr_poly, poly);
	inttolfsr(lfsr_value, value);
}

void parity_lfsr_shift(int din)
{
	char feedback;
	int i;

	feedback = lfsr_value[lfsr_len - 1] ^ din;

	for(i = lfsr_len - 1; i > 0; i--)
		lfsr_value[i]=(feedback & lfsr_poly[i]) ^ lfsr_value[i-1];

	lfsr_value[0] = (feedback & lfsr_poly[0]);
}

void get_parity(char *parity)
{
	int i;
	unsigned int value;
	int shift;

	shift = 0;
	value = 0;
	for(i = lfsr_len - 1; i >= 0; i--)
	{
		value |= lfsr_value[i] << shift;
		shift++;
		if(shift == 8) 
		{
			*parity = value;
			parity++;
			shift = 0;
			value = 0;
		}
	}
}

int ecc_parity_gen(unsigned char *data, unsigned int bits,unsigned int ecc_level,unsigned char *ecc_code)
{
	unsigned int i;
	unsigned char c;

	switch(ecc_level)
	{
		case 8:
			lfsr_init(14*8, "b1111111001111011100101111111111001010011100001000011110001110110010110011110001001110011110011010101110000101101", 0);
			break;
		case 16:
			lfsr_init(14*16, "b11001100001001000000110001110000010010101010001101001001000101001100001010100010100100010010000001011001010011110111111111010111111000111100001111010101100110000100010011101001111011011000100110101010100000110111011011001111", 0);
			break;
		case 24:
			lfsr_init(14*24, "b100011101001010011100000001001001000110110010000100111010010101101000101001001010111001011010001111011011101100111010000100110001111111001110011000011101000111010001101001001101100001011010010001010001001001110100011101000000100100001011011110100001010101101101110000010110100100110010010100110100011010101101011110101000011000011101111", 0);
			break;
		/* 28bit, todo */
		case 40:
			lfsr_init(14*40, "b11000000011111111000100110110001101000001101110001011101100101100110000110011111001100101101000001001001011001110101010011110110110111101001110101001111100100111111010100100111111011110001010011101111101100001111110101010011100110010001010110101000001011001100110110010010010101010010100010000000001100000100011101111101111011100011111100110011100010100101100111101100010111111010001000010000101011111110001011101111110111111010111011010010010001001101111100110001010011011010010100000111011000101011011100100100101000000000001010011100111011110010110111000001", 0);
			break;
		case 64:
			lfsr_init(14*64,  "b11110111101100010100111001110011001101100000001111000111100111101110101100101110110101100000111111010111101000101000000101101100011001001001110111010011000111100000011110001110010011001001101100011111000100010001010111000101010100011111000100011101000100110100010100000110000101101000101111010000111111010100100001011101011010111010101011010001000101101101011011001110111111100001010111101110010100000010000111111011001110001011001011100000001100010000000000011101000010011001111110001000001010010001010101001011010011010111001100010001101001011011001100111111111100011100001110100001001000110011011110101100001010101011000101010010101001111000011010000100011010101010000101011101001010011101100011010101100001011011101100001111001101100110111011110011100001010100001110111101111101100010010001010110110101100010001110000000101110100100010111000111111101001110000100000001011110100001000001010101", 0); 
			break;
		default:
			return -1;
	}


	for(i = 0; i < bits; i++) 
	{
		c = *(data + (i >> 3));
		c = (c >> (i & 0x7)) & 0x1;

		parity_lfsr_shift(c);
	}

	get_parity(ecc_code);

	return 0;
}

void ecc_data_gen(unsigned char *data, unsigned int len)
{
	unsigned int i;
	unsigned char *value;

	for(i = 0; i < len; i++) 
	{
		data[i] = ~data[i];
	}
}

int ecc_4bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 8, ecc_code);
	for (i=0; i < 14; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_8bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 16, ecc_code);
	for (i=0; i < 28; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_13bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 13, ecc_code);
	for (i=0; i < 24; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_24bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 24, ecc_code);
	for (i=0; i < 42; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_27bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 27, ecc_code);
	for (i=0; i < 48; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_40bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 40, ecc_code);
	for (i=0; i < 70; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_64bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 64, ecc_code);
	for (i=0; i < 112; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_41bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 41, ecc_code);
	for (i=0; i < 72; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_60bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 60, ecc_code);
	for (i=0; i < 108; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

int ecc_80bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len*8, 80, ecc_code);
	for (i=0; i < 140; i++)
	{
		ecc_code[i] = ~ecc_code[i];
	}
	return ret;
}

extern struct oobuse_info * get_oobuse_info(enum page_type pagetype, enum ecc_type ecctype);
void page_ecc_gen(unsigned char *pagebuf, enum page_type pagetype, enum ecc_type ecctype)
{
	unsigned char buf[MAX_PAGE_SIZE + MAX_OOB_SIZE];
	struct oobuse_info *info;
	unsigned int pagesize, i, j;
	
	info = get_oobuse_info(pagetype, ecctype);
	if(info == NULL)
		return;

	pagesize = get_pagesize(pagetype);

	if(ecctype == et_ecc_8bit1k)
	{
		unsigned char ecc_buf[14];

		if(pagetype == pt_pagesize_2K) 
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_4bit_gen(buf, 1040, ecc_buf);
			memcpy(pagebuf + 1040, ecc_buf, 14);
/* 1040,1032.etc These means the number of the data for ecc , find the details in chapter 4.3.4 in 3535 datasheet */
			memcpy(pagebuf + 1040 + 14, buf + 1040, 994);
			memcpy(pagebuf + 1040 + 14 + 994, buf + pagesize, 2);
			memcpy(pagebuf + 1040 + 14 + 994 + 2, buf + 1040 + 994, 14);
			memcpy(pagebuf + 1040 + 14 + 994 + 2 + 14 + 14, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_4bit_gen(buf + 1040, 1040, ecc_buf);
			memcpy(pagebuf + 1040 + 14 + 994 + 2 + 14, ecc_buf, 14);
		} else if (pagetype == pt_pagesize_4K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			for (i=0; i < 3; i++)
			{
				memcpy(pagebuf + (1032 + 14)*i, buf + 1032*i, 1032);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_4bit_gen(buf + 1032*i, 1032, ecc_buf);
				memcpy(pagebuf + (1032 + 14)*i + 1032, ecc_buf, 14);
			}

			memcpy(pagebuf + (1032 + 14)*3, buf + 1032*3, 958);
			memcpy(pagebuf + (1032 + 14)*3 + 958, buf + pagesize, 2);
			memcpy(pagebuf + (1032 + 14)*3 + 958 + 2, buf + 1032*3 + 958, 42);
			memcpy(pagebuf + (1032 + 14)*3 + 958 + 2 + 42 + 14, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_4bit_gen(buf + 1032*3, 1032, ecc_buf);
			memcpy(pagebuf + (1032 + 14)*3 + 958 + 2 + 42, ecc_buf, 14);
		} 
	}else if (ecctype == et_ecc_16bit1k)
	{
		unsigned char ecc_buf[28];

		if(pagetype == pt_pagesize_2K) 
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_8bit_gen(buf, 1028, ecc_buf);
			memcpy(pagebuf + 1028, ecc_buf, 28);

			memcpy(pagebuf + 1028 + 28, buf + 1028, 992);
			memcpy(pagebuf + 1028 + 28 + 992, buf + pagesize, 2);
			memcpy(pagebuf + 1028 + 28 + 992 + 2, buf + 1028 + 992, 28);
			memcpy(pagebuf + 1028 + 28 + 992 + 2 + 28 + 28, buf + pagesize + 2, 6);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_8bit_gen(buf + 1028, 1028, ecc_buf);
			memcpy(pagebuf + 1028 + 28 + 992 + 2 + 28, ecc_buf, 28);
		}else if (pagetype == pt_pagesize_4K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			for (i=0; i < 3; i++)
			{
				memcpy(pagebuf + (1028 + 28)*i, buf + 1028*i, 1028);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_8bit_gen(buf + 1028*i, 1028, ecc_buf);
				memcpy(pagebuf + (1028 + 28)*i + 1028, ecc_buf, 28);
			}

			memcpy(pagebuf + (1028 + 28)*3, buf + 1028*3, 928);
			memcpy(pagebuf + (1028 + 28)*3 + 928, buf + pagesize, 2);
			memcpy(pagebuf + (1028 + 28)*3 + 928 + 2, buf + 1028*3 + 928, 84);
			memcpy(pagebuf + (1028 + 28)*3 + 928 + 2 + 84 + 28, buf + pagesize + 2, 14);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_8bit_gen(buf + 1028*3, 1028, ecc_buf);
			memcpy(pagebuf + (1028 + 28)*3 + 928 + 2 + 84, ecc_buf, 28);
		}
	}else if(ecctype == et_ecc_24bit1k)
	{
		unsigned char ecc_buf[42];

		if(pagetype == pt_pagesize_2K) 
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_24bit1k_gen(buf, 1040, ecc_buf);
			memcpy(pagebuf + 1040, ecc_buf, 42);

			memcpy(pagebuf + 1040 + 42, buf + 1040, 966);
			memcpy(pagebuf + 1040 + 42 + 966, buf + pagesize, 2);
			memcpy(pagebuf + 1040 + 42 + 966 + 2, buf + 1040 + 966, 42);
			memcpy(pagebuf + 1040 + 42 + 966 + 2 + 42 + 42, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_24bit1k_gen(buf + 1040, 1040, ecc_buf);
			memcpy(pagebuf + 1040 + 42 + 966 + 2 + 42, ecc_buf, 42);
		}else if(pagetype == pt_pagesize_4K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);
			
			for (i=0; i < 3; i++)
			{
				memcpy(pagebuf + (1032 + 42)*i, buf + 1032*i, 1032);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_24bit1k_gen(buf + 1032*i, 1032, ecc_buf);
				memcpy(pagebuf + (1032 + 42)*i + 1032, ecc_buf, 42);
			}

			memcpy(pagebuf + (1032 + 42)*3, buf + 1032*3, 874);
			memcpy(pagebuf + (1032 + 42)*3 + 874, buf + pagesize, 2);
			memcpy(pagebuf + (1032 + 42)*3 + 874 + 2, buf + 1032*3 + 874, 126);
			memcpy(pagebuf + (1032 + 42)*3 + 874 + 2 + 126 + 42, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_24bit1k_gen(buf + 1032*3, 1032, ecc_buf);
			memcpy(pagebuf + (1032 + 42)*3 + 874 + 2 + 126, ecc_buf, 42);
		}else if(pagetype == pt_pagesize_8K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);
			
			for (i=0; i < 7; i++)
			{
				memcpy(pagebuf + (1028 + 42)*i, buf + 1028*i, 1028);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_24bit1k_gen(buf + 1028*i, 1028, ecc_buf);
				memcpy(pagebuf + (1028 + 42)*i + 1028, ecc_buf, 42);
			}

			memcpy(pagebuf + (1028 + 42)*7, buf + 1028*7, 702);
			memcpy(pagebuf + (1028 + 42)*7 + 702, buf + pagesize, 2);
			memcpy(pagebuf + (1028 + 42)*7 + 702 + 2, buf + 1028*7 + 702, 294);
			memcpy(pagebuf + (1028 + 42)*7 + 702 + 2 + 294 + 42, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_24bit1k_gen(buf + 1028*7, 1028, ecc_buf);
			memcpy(pagebuf + (1028 + 42)*7 + 702 + 2 + 294, ecc_buf, 42);
		}
	} else if(ecctype == et_ecc_40bit1k)
	{
		unsigned char ecc_buf[70];
		if(pagetype == pt_pagesize_8K) 
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);
			for (i=0; i < 7; i++)
			{
				memcpy(pagebuf + (1028 + 70)*i, buf + 1028*i, 1028);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_40bit1k_gen(buf + 1028*i, 1028, ecc_buf);
				memcpy(pagebuf + (1028 + 70)*i + 1028, ecc_buf, 70);
			}

			memcpy(pagebuf + (1028 + 70)*7, buf + 1028*7, 506);
			memcpy(pagebuf + (1028 + 70)*7 + 506, buf + pagesize, 2);
			memcpy(pagebuf + (1028 + 70)*7 + 506 + 2, buf + 1028*7 + 506, 490);
			memcpy(pagebuf + (1028 + 70)*7 + 506 + 2 + 490 + 70, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_40bit1k_gen(buf + 1028*7, 1028, ecc_buf);
			memcpy(pagebuf + (1028 + 70)*7 + 506 + 2 + 490, ecc_buf, 70);
		}else if (pagetype == pt_pagesize_16K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			for (i=0; i < 14; i++)
			{
				memcpy(pagebuf + (1026 + 70)*i, buf + 1026*i, 1026);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_40bit1k_gen(buf + 1026*i, 1026, ecc_buf);
				memcpy(pagebuf + (1026 + 70)*i + 1026, ecc_buf, 70);
			}

			memcpy(pagebuf + (1026 + 70)*14, buf + 1026*14, 1026);
			memcpy(pagebuf + (1026 + 70)*14 + 1026 + 14, buf + pagesize, 2);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_40bit1k_gen(buf + 1026*14, 1026, ecc_buf);
			memcpy(pagebuf + (1026 + 70)*14 + 1026, ecc_buf, 14);
			memcpy(pagebuf + (1026 + 70)*14 + 1026 + 2 + 14, ecc_buf + 14, 56);

			memcpy(pagebuf + (1026 + 70)*14 + 1026 + 2 + 14 + 56, buf + 1026*14 + 1026, 994);
			memcpy(pagebuf + (1026 + 70)*14 + 1026 + 2 + 14 + 56 + 994 + 70, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_40bit1k_gen(buf + 1026*14 + 1026, 1026, ecc_buf);
			memcpy(pagebuf + (1026 + 70)*14 + 1026 + 2 + 14 + 56 + 994, ecc_buf, 70);
		}
	} else if(ecctype == et_ecc_64bit1k)
	{
		unsigned char ecc_buf[112];
		if(pagetype == pt_pagesize_8K) 
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);
			for (i=0; i < 7; i++)
			{
				memcpy(pagebuf + (1028 + 112)*i, buf + 1028*i, 1028);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_64bit1k_gen(buf + 1028*i, 1028, ecc_buf);
				memcpy(pagebuf + (1028 + 112)*i + 1028, ecc_buf, 112);
			}

			memcpy(pagebuf + (1028 + 112)*7, buf + 1028*7, 212);
			memcpy(pagebuf + (1028 + 112)*7 + 212, buf + pagesize, 2);
			memcpy(pagebuf + (1028 + 112)*7 + 212 + 2, buf + 1028*7 + 212, 784);
			memcpy(pagebuf + (1028 + 112)*7 + 212 + 2 + 784 + 112, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_64bit1k_gen(buf + 1028*7, 1028, ecc_buf);
			memcpy(pagebuf + (1028 + 112)*7 + 212 + 2 + 784, ecc_buf, 112);
		}else if (pagetype == pt_pagesize_16K)
		{
			memcpy(buf, pagebuf, pagesize + info->oobuse);

			for (i=0; i < 14; i++)
			{
				memcpy(pagebuf + (1026 + 112)*i, buf + 1026*i, 1026);
				memset(ecc_buf, 0xFF, sizeof(ecc_buf));
				ecc_64bit1k_gen(buf + 1026*i, 1026, ecc_buf);
				memcpy(pagebuf + (1026 + 112)*i + 1026, ecc_buf, 112);
			}

			memcpy(pagebuf + (1026 + 112)*14, buf + 1026*14, 452);
			memcpy(pagebuf + (1026 + 112)*14 + 452, buf + pagesize, 2);
			memcpy(pagebuf + (1026 + 112)*14 + 452 + 2, buf + 1026*14 + 452, 574);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_64bit1k_gen(buf + 1026*14, 1026, ecc_buf);
			memcpy(pagebuf + (1026 + 112)*14 + 452 + 2 + 574, ecc_buf, 112);
			
			memcpy(pagebuf + (1026 + 112)*14 + 452 + 2 + 574 + 112, buf + 1026*14 + 452 + 574, 994);
			memcpy(pagebuf + (1026 + 112)*14 + 452 + 2 + 574 + 112 + 994 + 112, buf + pagesize + 2, 30);
			memset(ecc_buf, 0xFF, sizeof(ecc_buf));
			ecc_64bit1k_gen(buf + 1026*14 + 1026, 1026, ecc_buf);
			memcpy(pagebuf + (1026 + 112)*14 + 452 + 2 + 574 + 112 + 994, ecc_buf, 112);
		}
	}
}
