/*
 * This file is derived from crc32.c from the zlib-1.1.3 distribution
 * by Jean-loup Gailly and Mark Adler.
 */

/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#include "crc.h"

#define cpu_to_le32(x)      (x)
#define le32_to_cpu(x)      (x)

static int crc_table_empty = 1;
static uint32_t crc_table[256];

/*
  Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

  Polynomials over GF(2) are represented in binary, one bit per coefficient,
  with the lowest powers in the most significant bit.  Then adding polynomials
  is just exclusive-or, and multiplying a polynomial by x is a right shift by
  one.  If we call the above polynomial p, and represent a byte as the
  polynomial q, also with the lowest power in the most significant bit (so the
  byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
  where a mod b means the remainder after dividing a by b.

  This calculation is done using the shift-register method of multiplying and
  taking the remainder.  The register is initialized to zero, and for each
  incoming bit, x^32 is added mod p to the register if the bit is a one (where
  x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
  x (which is shifting right by one and adding x^32 mod p if the bit shifted
  out is a one).  We start with the highest power (least significant bit) of
  q and repeat for all eight bits of q.

  The table is simply the CRC of all possible eight bit values.  This is all
  the information needed to generate CRC's on data a byte at a time for all
  combinations of CRC register values and incoming bytes.
*/
static void make_crc_table(void)
{
    uint32_t c;
    int n, k;
    unsigned long poly;  /* polynomial exclusive-or pattern */
    /* terms of polynomial defining this crc (except x^32): */
    static const char p[] = {0, 1, 2, 4, 5, 7, 8,
                             10, 11, 12, 16, 22, 23, 26
                            };

    /* make exclusive-or pattern from polynomial (0xedb88320L) */
    poly = 0L;
    for (n = 0; n < (sizeof(p) / sizeof(char)); n++)
    { poly |= 1L << (31 - p[n]); }

    for (n = 0; n < 256; n++)
    {
        c = (unsigned long)n;
        for (k = 0; k < 8; k++)
        { c = c & 1 ? poly ^ (c >> 1) : c >> 1; }
        crc_table[n] = cpu_to_le32(c);
    }
    crc_table_empty = 0;
}

/* ========================================================================= */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define DO_CRC(x) (crc = tab[(crc ^ (x)) & 255] ^ (crc >> 8))
#else
#define DO_CRC(x) (crc = tab[((crc >> 24) ^ (x)) & 255] ^ (crc << 8))
#endif

/* ========================================================================= */

/* No ones complement version. JFFS2 (and other things ?)
 * don't use ones compliment in their CRC calculations.
 */
uint32_t crc32_no_comp(uint32_t crc,  unsigned char* buf, uint32_t len)
{
    const uint32_t* tab = crc_table;
    const uint32_t* b = (const uint32_t*)buf;
    size_t rem_len;

    if (crc_table_empty)
    {
        make_crc_table();
    }

    crc = cpu_to_le32(crc);
    /* Align it */
    if (((long)b) & 3 && len)
    {
        uint8_t* p = (uint8_t*)b;
        do
        {
            DO_CRC(*p++);
        }
        while ((--len) && ((long)p) & 3);
        b = (uint32_t*)p;
    }

    rem_len = len & 3;
    len = len >> 2;
    for (--b; len; --len)
    {
        /* load data 32 bits wide, xor data 32 bits wide. */
        crc ^= *++b; /* use pre increment for speed */
        DO_CRC(0);
        DO_CRC(0);
        DO_CRC(0);
        DO_CRC(0);
    }
    len = rem_len;
    /* And the last few bytes */
    if (len)
    {
        uint8_t* p = (uint8_t*)(b + 1) - 1;
        do
        {
            DO_CRC(*++p); /* use pre increment for speed */
        }
        while (--len);
    }

    return le32_to_cpu(crc);
}
#undef DO_CRC

uint32_t __crc32(uint32_t crc,  unsigned char* p, uint32_t len)
{
    return crc32_no_comp(crc ^ 0xffffffffL, p, len) ^ 0xffffffffL;
}

