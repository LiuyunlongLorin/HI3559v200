/*****************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : kapi_symc.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include "drv_osal_lib.h"

/* kapi_test end*/
static HI_S32 set_cipher_config(HI_HANDLE chnHandle, HI_BOOL bKeyByCA, HI_UNF_CIPHER_CA_TYPE_E enCaType,
                                      HI_UNF_CIPHER_ALG_E alg, HI_UNF_CIPHER_WORK_MODE_E mode,
                                      HI_UNF_CIPHER_KEY_LENGTH_E keyLen, const HI_U8 u8KeyBuf[16],
                                      const HI_U8 u8IVBuf[16])
{
    HI_U32 ivlen = AES_IV_SIZE;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Hardkey = 0;
    HI_UNF_CIPHER_CTRL_S CipherCtrl;
    compat_addr aad;

    memset(&CipherCtrl, 0, sizeof(HI_UNF_CIPHER_CTRL_S));
    CipherCtrl.enAlg = alg;
    CipherCtrl.enWorkMode = mode;
    CipherCtrl.enBitWidth = HI_UNF_CIPHER_BIT_WIDTH_128BIT;
    CipherCtrl.enKeyLen = keyLen;
    CipherCtrl.bKeyByCA = bKeyByCA;
    CipherCtrl.enCaType = enCaType;
    if(CipherCtrl.enWorkMode != HI_UNF_CIPHER_WORK_MODE_ECB)
    {
        CipherCtrl.stChangeFlags.bit1IV = 1;  //must set for CBC , CFB mode
        memcpy(CipherCtrl.u32IV, u8IVBuf, 16);
    }

    memcpy(CipherCtrl.u32Key, u8KeyBuf, 16);

    if (HI_TRUE == CipherCtrl.bKeyByCA)
    {
        if (CRYP_CIPHER_CA_TYPE_BUTT <= CipherCtrl.enCaType)
        {
            pr_err("Invalid enCaType with bKeyByCA is HI_TRUE.\n");
            return HI_ERR_CIPHER_INVALID_PARA;

        }
        u32Hardkey  = (CipherCtrl.enCaType & 0xFF) << BITS_IN_BYTE;
        u32Hardkey |= 0x01;
    }

    ADDR_U64(aad) = 0x00;

    if ((CRYP_CIPHER_ALG_3DES == CipherCtrl.enAlg) || (CRYP_CIPHER_ALG_DES == CipherCtrl.enAlg))
    {
        ivlen = DES_IV_SIZE;
    }

    s32Ret = kapi_symc_config(chnHandle,u32Hardkey,
                CipherCtrl.enAlg, CipherCtrl.enWorkMode,
                CipherCtrl.enBitWidth, CipherCtrl.enKeyLen, 0, (HI_U8*)CipherCtrl.u32Key, HI_NULL,
                (HI_U8*)CipherCtrl.u32IV, ivlen, CipherCtrl.stChangeFlags.bit1IV,
                aad, 0, 0);
    if(HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 set_cipher_crypt(HI_HANDLE hCipher, HI_SIZE_T u32SrcPhyAddr,
            HI_SIZE_T u32DestPhyAddr, HI_U32 u32ByteLength, HI_U32 operation)
{
    compat_addr input;
    compat_addr output;
    HI_S32 ret = HI_FAILURE;

    ADDR_U64(input) = u32SrcPhyAddr;
    ADDR_U64(output) = u32DestPhyAddr;

    ret = kapi_symc_crypto_via(hCipher, input, output, u32ByteLength, operation, 0, 0);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }

    return HI_SUCCESS;
}

static HI_S32 cipher_aes_decrypt(HI_BOOL bKeyByCA,
              HI_UNF_CIPHER_WORK_MODE_E mode,
              const unsigned char *Key, const unsigned char *iv,
              const unsigned char *in_buf, const unsigned char *out_buf,
              const unsigned char *expect_buf,
              const unsigned int buf_len)
{
    int s32Ret = 0;
    HI_HANDLE hTestchnid;

    s32Ret = kapi_symc_create(&hTestchnid);
    if(0 != s32Ret)
    {
        HI_LOG_ERROR("Error: CreateHandle failed!\n");
        return HI_FAILURE;
    }

    /* For decrypt */

    s32Ret = set_cipher_config(hTestchnid,
                                    bKeyByCA,
                                    HI_UNF_CIPHER_KEY_SRC_KLAD_1,
                                    HI_UNF_CIPHER_ALG_AES,
                                    mode,
                                    HI_UNF_CIPHER_KEY_AES_128BIT,
                                    Key,
                                    iv);
    if(0 != s32Ret)
    {
        HI_LOG_ERROR("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    s32Ret = set_cipher_crypt(hTestchnid, (HI_SIZE_T)in_buf, (HI_SIZE_T)out_buf, buf_len, SYMC_OPERATION_DECRYPT);
    if(0 != s32Ret)
    {
        HI_LOG_ERROR("[5]wrong data!\n");
        s32Ret = -1;
        goto __CIPHER_EXIT__;
    }

    /* compare */
    if ( 0 != memcmp(out_buf, expect_buf, 16) )
    {
        HI_LOG_ERROR("Memcmp failed!\n");
        s32Ret = HI_FAILURE;
        goto __CIPHER_EXIT__;
    }

__CIPHER_EXIT__:
    kapi_symc_destroy(hTestchnid);

    return s32Ret;
}

/* otp key is be written, then excute tihs test */
static HI_S32 kapi_test1(void)
{
    HI_S32 ret = HI_FAILURE;
    HI_U8 aes_128_enc_key[16]  = {0xc1,0x1b,0x54,0x4a,0x12,0x9c,0x08,0xa5,0xcc,0xd3,0xeb,0xec,0x7a,0x3b,0x00,0x2b};
    HI_U8 aes_128_cbc_IV[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    HI_U8 aes_128_src_buf[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    HI_U8 aes_128_dst_buf[16] = {0xb0,0x1b,0x77,0x09,0xe8,0xdc,0xf9,0xef,0x37,0x13,0x0b,0x13,0xda,0x11,0xbf,0x24};
    HI_U8 aes_128_src2_buf[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};

    HI_LOG_ERROR("klad test!\n");
    ret = cipher_aes_decrypt(HI_TRUE,
                       HI_UNF_CIPHER_WORK_MODE_CBC,
                       aes_128_enc_key, aes_128_cbc_IV,
                       aes_128_dst_buf, aes_128_src2_buf,
                       aes_128_src_buf,
                       16);
    if(ret!= HI_SUCCESS)
    {
      HI_LOG_ERROR("klad failed!\n");
      return -1;
    }

    HI_LOG_ERROR("klad success!\n");
    return HI_SUCCESS;
}

static HI_S32 kapi_test2(void)
{
    HI_S32 ret = HI_FAILURE;
    HI_U8 aes_key[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
    HI_U8 aes_IV[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    HI_U8 aes_src[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    HI_U8 aes_dst[16] = {0x76,0x49,0xAB,0xAC,0x81,0x19,0xB2,0x46,0xCE,0xE9,0x8E,0x9B,0x12,0xE9,0x19,0x7D};
    HI_U8 aes_src2[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};

    HI_LOG_ERROR("cbc test!\n");
    ret = cipher_aes_decrypt(
                       HI_FALSE,
                       HI_UNF_CIPHER_WORK_MODE_CBC,
                       aes_key, aes_IV,
                       aes_dst, aes_src2,
                       aes_src,
                       16);
    if(ret!= HI_SUCCESS)
    {
      HI_LOG_ERROR("cbc failed!\n");
      return -1;
    }

    HI_LOG_ERROR("cbc success!\n");
    return HI_SUCCESS;
}

void kapi_test(void)
{
    kapi_test1();
    kapi_test2();
}

/* kapi_test end */

/** @}*/  /** <!-- ==== Structure Definition end ====*/
