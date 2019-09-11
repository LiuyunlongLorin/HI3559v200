/******************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : hi_unf_cipher.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include "user_osal_lib.h"

crypto_mutex  cipher_mutex = PTHREAD_MUTEX_INITIALIZER;

#define HI_CIPHER_LOCK()        (void)crypto_mutex_lock(&cipher_mutex)
#define HI_CIPHER_UNLOCK()      (void)crypto_mutex_unlock(&cipher_mutex)

#define BYTE_BITS               (8)
#define CIPHER_MAX_MULTIPAD_NUM (5000)
#define CENC_SUBSAMPLE_MAX_NUM  (100)
#define ECDH_MAX_KEY_LEN        (72)
#define CIPHER_INIT_MAX_NUM     (0x7FFFFFFF)
#define MAX_TAG_LEN             (16)

/* handle of cipher device */
HI_S32 g_CipherDevFd = -1;

/* flag of cipher device
 * indicate the status of device that open or close
 * <0: close, 0: open>0: multiple initialization
 */
static HI_S32 g_CipherInitCounter = -1;

/* check the device of cipher whether already opend or not */
#define CHECK_CIPHER_OPEN()\
do{\
    if (0 > g_CipherInitCounter)\
    {\
        HI_LOG_ERROR("cipher init counter %d\n", g_CipherInitCounter);\
        HI_LOG_PrintErrCode(HI_ERR_CIPHER_NOT_INIT);\
        return HI_ERR_CIPHER_NOT_INIT;\
    }\
}while(0)

/**
 * Read E in public key from arry to U32,
 * so only use last byte0~byte3, others are zero
 */
#define CIPHER_GET_PUB_EXPONENT(e, rsades) \
{\
    u8 *buf = rsades->stPubKey.pu8E; \
    u8 *pub = (u8*)e; \
    u32 len = rsades->stPubKey.u16ELen; \
    u32 i; \
    for (i = 0; i< MIN(WORD_WIDTH, len); i++) \
    {\
        pub[WORD_WIDTH -i - 1] = buf[len - i - 1];\
    }\
}


/**
 * \brief  Init the cipher device.
 */
HI_S32 CRYP_CIPHER_Init(HI_VOID)
{
    HI_UNF_FuncEnter();

    HI_CIPHER_LOCK();

    if (CIPHER_INIT_MAX_NUM <= g_CipherInitCounter)
    {
        HI_CIPHER_UNLOCK();

        HI_LOG_PrintErrCode(HI_ERR_CIPHER_OVERFLOW);
        return HI_ERR_CIPHER_OVERFLOW;
    }

    if (0 <= g_CipherInitCounter)
    {
        g_CipherInitCounter++;
        HI_CIPHER_UNLOCK();

        HI_UNF_FuncExit();
        return HI_SUCCESS;
    }

    g_CipherDevFd = crypto_open("/dev/"UMAP_DEVNAME_CIPHER, O_RDWR, 0);
    if (0 > g_CipherDevFd)
    {
        HI_CIPHER_UNLOCK();

        HI_LOG_PrintFuncErr(crypto_open, HI_ERR_CIPHER_FAILED_INIT);
        return HI_ERR_CIPHER_FAILED_INIT;
    }

    g_CipherInitCounter = 0;

    HI_CIPHER_UNLOCK();

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief  Deinit the cipher device.
 */
HI_S32 CRYP_CIPHER_DeInit(HI_VOID)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_CIPHER_LOCK();

    if (0 > g_CipherInitCounter)
    {
        HI_CIPHER_UNLOCK();

        HI_UNF_FuncExit();
        return HI_SUCCESS;
    }

    if (0 < g_CipherInitCounter)
    {
        g_CipherInitCounter--;

        HI_CIPHER_UNLOCK();

        HI_UNF_FuncExit();
        return HI_SUCCESS;
    }

    ret = crypto_close(g_CipherDevFd);
    if (HI_SUCCESS != ret)
    {
        HI_CIPHER_UNLOCK();

        HI_LOG_PrintFuncErr(crypto_close, ret);
        return ret;
    }

    g_CipherInitCounter = -1;

    HI_CIPHER_UNLOCK();

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Obtain a cipher handle for encryption and decryption.
 */
HI_S32 CRYP_CIPHER_CreateHandle(HI_HANDLE* phCipher, const CRYP_CIPHER_ATTS_S *pstCipherAttr)
{
    HI_S32 ret = HI_FAILURE;
    HI_U32 id = 0;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == phCipher);
    HI_LOG_CHECK_PARAM(HI_NULL == pstCipherAttr);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_TYPE_BUTT <= pstCipherAttr->enCipherType);

    HI_DBG_PrintU32(pstCipherAttr->enCipherType);

    CHECK_CIPHER_OPEN();

    ret = mpi_symc_create(&id);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_create, ret);
        return ret;
    }

    *phCipher = id;

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

static HI_S32 CRYP_CIPHER_GetRsaAttr(HI_U32 u32SchEme, HI_U32 *pu32Hlen,
                    CRYP_CIPHER_HASH_TYPE_E *penShaType)
{
    HI_UNF_FuncEnter();

    switch(u32SchEme)
    {
        case CRYP_CIPHER_RSA_ENC_SCHEME_NO_PADDING:
        case CRYP_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_0:
        case CRYP_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_1:
        case CRYP_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_2:
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_PKCS1_V1_5:
        {
            *pu32Hlen = 0;
            *penShaType = CRYP_CIPHER_HASH_TYPE_BUTT;

            break;
        }
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA1:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA1:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA1:
        {
            *pu32Hlen = SHA1_RESULT_SIZE;
            *penShaType = CRYP_CIPHER_HASH_TYPE_SHA1;

            break;
        }
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA224:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA224:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA224:
            *pu32Hlen = SHA224_RESULT_SIZE;
            *penShaType = CRYP_CIPHER_HASH_TYPE_SHA224;

            break;
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA256:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256:
        {
            *pu32Hlen = SHA256_RESULT_SIZE;
            *penShaType = CRYP_CIPHER_HASH_TYPE_SHA256;
            break;
        }
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA384:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384:
        {
            *pu32Hlen = SHA384_RESULT_SIZE;
            *penShaType = CRYP_CIPHER_HASH_TYPE_SHA384;

            break;
        }
        case CRYP_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA512:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512:
        case CRYP_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512:
        {
            *pu32Hlen = SHA512_RESULT_SIZE;
            *penShaType = CRYP_CIPHER_HASH_TYPE_SHA512;
            break;
        }
        default:
        {
            HI_ERR_PrintU32(u32SchEme);
            HI_LOG_PrintErrCode(HI_ERR_CIPHER_UNAVAILABLE);
            return HI_ERR_CIPHER_UNAVAILABLE;
        }
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Destroy the existing cipher handle.
 */
HI_S32 CRYP_CIPHER_DestroyHandle(HI_HANDLE hCipher)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    CHECK_CIPHER_OPEN();

    ret = mpi_symc_destroy(hCipher);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_destroy, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Configures the cipher control information.
 */
HI_S32 CRYP_CIPHER_ConfigHandle(HI_HANDLE hCipher, const CRYP_CIPHER_CTRL_S * pstCtrl)
{
    HI_U32 ivlen = AES_IV_SIZE;
    HI_U32 u32Hardkey = 0;
    HI_U32 u32KeyLen = SYMC_KEY_SIZE;
    compat_addr aad;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstCtrl);
    HI_LOG_CHECK_PARAM(HI_INVALID_HANDLE == hCipher);

    if ((CRYP_CIPHER_WORK_MODE_CCM == pstCtrl->enWorkMode)
        || (CRYP_CIPHER_WORK_MODE_GCM == pstCtrl->enWorkMode))
    {
        HI_LOG_ERROR("Invalid work mode ccm or gcm by config handle.\n");
        return HI_ERR_CIPHER_INVALID_PARA;
    }

    if ((HI_TRUE != pstCtrl->bKeyByCA)
        && (HI_FALSE != pstCtrl->bKeyByCA)
        && (CRYP_CIPHER_ALG_DMA != pstCtrl->enAlg))
    {
        HI_LOG_ERROR("Invalid bKeyByCA, you should set HI_TRUE or HI_FALSE.\n");
        return HI_ERR_CIPHER_INVALID_PARA;
    }

    if (HI_TRUE == pstCtrl->bKeyByCA)
    {
        if (CRYP_CIPHER_CA_TYPE_BUTT <= pstCtrl->enCaType)
        {
            HI_LOG_ERROR("Invalid enCaType with bKeyByCA is HI_TRUE.\n");
            HI_LOG_PrintErrCode(HI_ERR_CIPHER_INVALID_PARA);
            return HI_ERR_CIPHER_INVALID_PARA;
        }
        u32Hardkey  = (pstCtrl->enCaType & 0xFF) << BITS_IN_BYTE;
        u32Hardkey |= 0x01;
    }

    CHECK_CIPHER_OPEN();

    ADDR_U64(aad) = 0x00;

    if ((CRYP_CIPHER_ALG_3DES == pstCtrl->enAlg) || (CRYP_CIPHER_ALG_DES == pstCtrl->enAlg))
    {
        ivlen = DES_IV_SIZE;
    }

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(pstCtrl->bKeyByCA);
    HI_DBG_PrintU32(pstCtrl->enAlg);
    HI_DBG_PrintU32(pstCtrl->enWorkMode);
    HI_DBG_PrintU32(pstCtrl->enBitWidth);
    HI_DBG_PrintU32(pstCtrl->enKeyLen);
    HI_DBG_PrintU32(ivlen);
    HI_DBG_PrintU32(pstCtrl->stChangeFlags.bit1IV);
    HI_DBG_PrintU64(ADDR_U64(aad));

    ret = mpi_symc_config(hCipher, u32Hardkey,
        pstCtrl->enAlg, pstCtrl->enWorkMode,
        pstCtrl->enBitWidth, pstCtrl->enKeyLen, 0, (HI_U8*)pstCtrl->u32Key, HI_NULL, u32KeyLen,
        (HI_U8*)pstCtrl->u32IV, ivlen, pstCtrl->stChangeFlags.bit1IV,
        aad, 0, 0);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_config, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Configures the cipher expand control information.
 */
HI_S32 CRYP_CIPHER_ConfigHandleEx(HI_HANDLE hCipher, const CRYP_CIPHER_CTRL_EX_S * pstExCtrl)
{
    CRYP_CIPHER_KEY_LENGTH_E enKeyLen = CRYP_CIPHER_KEY_DEFAULT;
    HI_U8 *pu8FirstKey = HI_NULL;
    HI_U8 *pu8SndKey = HI_NULL;
    HI_U32 u32KeyLen = 0;
    HI_U8 *pu8IV = HI_NULL;
    HI_U32 u32Usage = 0;
    compat_addr aad;
    HI_U32 u32ALen = 0;
    HI_U32 u32Taglen = 0;
    HI_U32 u32IVLen = 0;
    HI_U8 u8Sm1FirstKey[SYMC_SM1_SK_SIZE * 2] = {0};
    CRYP_CIPHER_SM1_ROUND_E enSm1Round = HI_UNF_CIPHER_SM1_ROUND_BUTT;
    CRYP_CIPHER_BIT_WIDTH_E enBitWidth = CRYP_CIPHER_BIT_WIDTH_128BIT;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstExCtrl);
    HI_LOG_CHECK_PARAM((HI_NULL == pstExCtrl->pParam) && (CRYP_CIPHER_ALG_DMA != pstExCtrl->enAlg));

    if ((HI_TRUE != pstExCtrl->bKeyByCA)
        && (HI_FALSE != pstExCtrl->bKeyByCA)
        && (CRYP_CIPHER_ALG_DMA != pstExCtrl->enAlg))
    {
        HI_LOG_ERROR("Invalid bKeyByCA, you should set HI_TRUE or HI_FALSE.\n");
        return HI_ERR_CIPHER_INVALID_PARA;
    }

    CHECK_CIPHER_OPEN();

    ADDR_U64(aad) = 0x00;

    /*****************************************************************************
     * for AES, the pointer should point to CRYP_CIPHER_CTRL_AES_S;
     * for AES_CCM or AES_GCM, the pointer should point to CRYP_CIPHER_CTRL_AES_CCM_GCM_S;
     * for DES, the pointer should point to CRYP_CIPHER_CTRL_DES_S;
     * for 3DES, the pointer should point to CRYP_CIPHER_CTRL_3DES_S;
     * for SM1, the pointer should point to CRYP_CIPHER_CTRL_SM1_S;
     * for SM4, the pointer should point to CRYP_CIPHER_CTRL_SM4_S;
    */
    switch(pstExCtrl->enAlg)
    {
    case CRYP_CIPHER_ALG_DES:
    {
        CRYP_CIPHER_CTRL_DES_S *pstDesCtrl = (CRYP_CIPHER_CTRL_DES_S *)pstExCtrl->pParam;

        pu8FirstKey = (HI_U8*)pstDesCtrl->u32Key;
        u32KeyLen = sizeof(pstDesCtrl->u32Key);
        pu8IV = (HI_U8*)pstDesCtrl->u32IV;
        u32Usage = pstDesCtrl->stChangeFlags.bit1IV;
        u32IVLen = DES_IV_SIZE;
        enBitWidth = pstDesCtrl->enBitWidth;

        break;
    }
    case CRYP_CIPHER_ALG_3DES:
    {
        CRYP_CIPHER_CTRL_3DES_S *pst3DesCtrl = (CRYP_CIPHER_CTRL_3DES_S *)pstExCtrl->pParam;

        pu8FirstKey = (HI_U8*)pst3DesCtrl->u32Key;
        u32KeyLen = sizeof(pst3DesCtrl->u32Key);
        pu8IV = (HI_U8*)pst3DesCtrl->u32IV;
        u32Usage = pst3DesCtrl->stChangeFlags.bit1IV;
        enKeyLen = pst3DesCtrl->enKeyLen;
        u32IVLen = DES_IV_SIZE;
        enBitWidth = pst3DesCtrl->enBitWidth;
        break;
    }
    case CRYP_CIPHER_ALG_AES:
    {
        if ((pstExCtrl->enWorkMode == CRYP_CIPHER_WORK_MODE_CCM)
            || (pstExCtrl->enWorkMode == CRYP_CIPHER_WORK_MODE_GCM))
        {
            CRYP_CIPHER_CTRL_AES_CCM_GCM_S *pstAesCcmGcmCtrl
                = (CRYP_CIPHER_CTRL_AES_CCM_GCM_S *)pstExCtrl->pParam;

            pu8FirstKey = (HI_U8*)pstAesCcmGcmCtrl->u32Key;
            u32KeyLen = sizeof(pstAesCcmGcmCtrl->u32Key);
            pu8IV = (HI_U8*)pstAesCcmGcmCtrl->u32IV;
            u32IVLen = pstAesCcmGcmCtrl->u32IVLen;

            if (AES_IV_SIZE < u32IVLen)
            {
                HI_LOG_ERROR("para set CIPHER ccm/gcm iv is invalid, u32IVLen:0x%x.\n",u32IVLen);
                HI_LOG_PrintErrCode(HI_ERR_CIPHER_INVALID_PARA);
                return HI_ERR_CIPHER_INVALID_PARA;
            }

            u32Taglen = pstAesCcmGcmCtrl->u32TagLen;
            enKeyLen = pstAesCcmGcmCtrl->enKeyLen;
            ADDR_U64(aad) = pstAesCcmGcmCtrl->szAPhyAddr;
            u32ALen = pstAesCcmGcmCtrl->u32ALen;
            u32Usage = CIPHER_IV_CHANGE_ONE_PKG;
        }
        else
        {
            CRYP_CIPHER_CTRL_AES_S *pstAesCtrl = (CRYP_CIPHER_CTRL_AES_S *)pstExCtrl->pParam;

            pu8FirstKey = (HI_U8*)pstAesCtrl->u32EvenKey;
            pu8SndKey = (HI_U8*)pstAesCtrl->u32OddKey;
            u32KeyLen = sizeof(pstAesCtrl->u32EvenKey);
            pu8IV = (HI_U8*)pstAesCtrl->u32IV;
            u32Usage = pstAesCtrl->stChangeFlags.bit1IV;
            enKeyLen = pstAesCtrl->enKeyLen;
            enBitWidth = pstAesCtrl->enBitWidth;
            u32IVLen = AES_IV_SIZE;
        }
        break;
    }
    case CRYP_CIPHER_ALG_SM1:
    {
        CRYP_CIPHER_CTRL_SM1_S *pstSm1Ctrl = (CRYP_CIPHER_CTRL_SM1_S *)pstExCtrl->pParam;

        /* SM1 key:EK+AK+SK, fist key: EK+AK, second key: SK. */
        ret = memcpy_s(u8Sm1FirstKey, sizeof(u8Sm1FirstKey), pstSm1Ctrl->u32EK, sizeof(pstSm1Ctrl->u32EK));
        if (ret != 0)
        {
            HI_LOG_PrintFuncErr(memcpy_s, HI_ERR_CIPHER_MEMCPY_S_FAILED);
            return HI_ERR_CIPHER_MEMCPY_S_FAILED;
        }

        ret = memcpy_s(u8Sm1FirstKey + sizeof(pstSm1Ctrl->u32EK), sizeof(u8Sm1FirstKey) - sizeof(pstSm1Ctrl->u32EK), pstSm1Ctrl->u32AK, sizeof(pstSm1Ctrl->u32AK));
        if (ret != 0)
        {
            HI_LOG_PrintFuncErr(memcpy_s, HI_ERR_CIPHER_MEMCPY_S_FAILED);
            crypto_zeroize(u8Sm1FirstKey, sizeof(u8Sm1FirstKey));
            return HI_ERR_CIPHER_MEMCPY_S_FAILED;
        }

        pu8FirstKey = (HI_U8*)u8Sm1FirstKey;
        pu8SndKey = (HI_U8*)pstSm1Ctrl->u32SK;
        u32KeyLen = sizeof(pstSm1Ctrl->u32EK);
        pu8IV = (HI_U8*)pstSm1Ctrl->u32IV;
        u32Usage = pstSm1Ctrl->stChangeFlags.bit1IV;
        enKeyLen = CRYP_CIPHER_KEY_DEFAULT;
        enSm1Round = pstSm1Ctrl->enSm1Round;
        enBitWidth = pstSm1Ctrl->enBitWidth;
        u32IVLen = AES_IV_SIZE;
        break;
    }
    case CRYP_CIPHER_ALG_SM4:
    {
        CRYP_CIPHER_CTRL_SM4_S *pstSm4Ctrl = (CRYP_CIPHER_CTRL_SM4_S *)pstExCtrl->pParam;

        pu8FirstKey = (HI_U8*)pstSm4Ctrl->u32Key;
        u32KeyLen = sizeof(pstSm4Ctrl->u32Key);
        pu8IV = (HI_U8*)pstSm4Ctrl->u32IV;
        u32Usage = pstSm4Ctrl->stChangeFlags.bit1IV;
        enKeyLen = CRYP_CIPHER_KEY_DEFAULT;
        u32IVLen = AES_IV_SIZE;
        break;
    }
    case CRYP_CIPHER_ALG_DMA:
    {
        break;
    }
    default:
        HI_LOG_ERROR("para set CIPHER alg is invalid.\n");
        HI_LOG_PrintErrCode(HI_ERR_CIPHER_INVALID_PARA);
        return HI_ERR_CIPHER_INVALID_PARA;
    }

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(pstExCtrl->bKeyByCA);
    HI_DBG_PrintU32(pstExCtrl->enAlg);
    HI_DBG_PrintU32(pstExCtrl->enWorkMode);
    HI_DBG_PrintU32(enBitWidth);
    HI_DBG_PrintU32(enKeyLen);
    HI_DBG_PrintU32(enSm1Round);
    HI_DBG_PrintU32(u32KeyLen);
    HI_DBG_PrintU32(u32IVLen);
    HI_DBG_PrintU32(u32Usage);
    HI_DBG_PrintU64(ADDR_U64(aad));
    HI_DBG_PrintU32(u32ALen);
    HI_DBG_PrintU32(u32Taglen);

    ret = mpi_symc_config(hCipher, pstExCtrl->bKeyByCA, pstExCtrl->enAlg,
        pstExCtrl->enWorkMode, enBitWidth, enKeyLen, enSm1Round,
        pu8FirstKey, pu8SndKey, u32KeyLen, pu8IV, u32IVLen,
        u32Usage, aad, u32ALen, u32Taglen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_config, ret);
        crypto_zeroize(u8Sm1FirstKey, sizeof(u8Sm1FirstKey));
        return ret;
    }

    crypto_zeroize(u8Sm1FirstKey, sizeof(u8Sm1FirstKey));
    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Performs encryption.
 */
HI_S32 CRYP_CIPHER_Encrypt(HI_HANDLE hCipher, HI_SIZE_T u32SrcPhyAddr, HI_SIZE_T u32DestPhyAddr, HI_U32 u32ByteLength)
{
    compat_addr input;
    compat_addr output;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    CHECK_CIPHER_OPEN();

    ADDR_U64(input) = u32SrcPhyAddr;
    ADDR_U64(output) = u32DestPhyAddr;

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU64(u32SrcPhyAddr);
    HI_DBG_PrintU64(u32DestPhyAddr);
    HI_DBG_PrintU32(u32ByteLength);

    ret = mpi_symc_crypto(hCipher, input, output, u32ByteLength, SYMC_OPERATION_ENCRYPT);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Performs descryption.
 */
HI_S32 CRYP_CIPHER_Decrypt(HI_HANDLE hCipher, HI_SIZE_T u32SrcPhyAddr,
                             HI_SIZE_T u32DestPhyAddr, HI_U32 u32ByteLength)
{
    compat_addr input;
    compat_addr output;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU64(u32SrcPhyAddr);
    HI_DBG_PrintU64(u32DestPhyAddr);
    HI_DBG_PrintU32(u32ByteLength);

    CHECK_CIPHER_OPEN();

    ADDR_U64(input) = u32SrcPhyAddr;
    ADDR_U64(output) = u32DestPhyAddr;

    ret = mpi_symc_crypto(hCipher, input, output, u32ByteLength, SYMC_OPERATION_DECRYPT);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Performs encryption.
 */
HI_S32 CRYP_CIPHER_EncryptVir(HI_HANDLE hCipher, const HI_U8 *pu8SrcData, HI_U8 *pu8DestData, HI_U32 u32ByteLength)
{
    compat_addr input;
    compat_addr output;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu8SrcData);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8DestData);

    CHECK_CIPHER_OPEN();

    ADDR_VIA_CONST(input) = pu8SrcData;
    ADDR_VIA(output) = pu8DestData;

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(u32ByteLength);

    ret = mpi_symc_crypto(hCipher, input, output, u32ByteLength, SYMC_OPERATION_ENCRYPT_VIA);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Performs decryption.
 */
HI_S32 CRYP_CIPHER_DecryptVir(HI_HANDLE hCipher, const HI_U8 *pu8SrcData, HI_U8 *pu8DestData, HI_U32 u32ByteLength)
{
    compat_addr input;
    compat_addr output;
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu8SrcData);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8DestData);

    CHECK_CIPHER_OPEN();

    ADDR_VIA_CONST(input) = pu8SrcData;
    ADDR_VIA(output) = pu8DestData;

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(u32ByteLength);

    ret = mpi_symc_crypto(hCipher, input, output, u32ByteLength, SYMC_OPERATION_DECRYPT_VIA);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Encrypt multiple packaged data.
 */
HI_S32 CRYP_CIPHER_EncryptMulti(HI_HANDLE hCipher,
                                const CRYP_CIPHER_DATA_S *pstDataPkg,
                                HI_U32 u32DataPkgNum)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    CHECK_CIPHER_OPEN();

    HI_LOG_CHECK_PARAM(HI_NULL == pstDataPkg);
    HI_LOG_CHECK_PARAM((HI_TRUE != pstDataPkg->bOddKey) && (HI_FALSE != pstDataPkg->bOddKey));
    HI_LOG_CHECK_PARAM(0x00 == u32DataPkgNum);
    HI_LOG_CHECK_PARAM(CIPHER_MAX_MULTIPAD_NUM <= u32DataPkgNum);

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(u32DataPkgNum);

    ret = mpi_symc_crypto_multi(hCipher, pstDataPkg, u32DataPkgNum, SYMC_OPERATION_ENCRYPT);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto_multi, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/**
 * \brief Encrypt multiple packaged data.
 */
HI_S32 CRYP_CIPHER_DecryptMulti(HI_HANDLE hCipher, const CRYP_CIPHER_DATA_S *pstDataPkg, HI_U32 u32DataPkgNum)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();


    HI_LOG_CHECK_PARAM(HI_NULL == pstDataPkg);
    HI_LOG_CHECK_PARAM((HI_TRUE != pstDataPkg->bOddKey) && (HI_FALSE != pstDataPkg->bOddKey));
    HI_LOG_CHECK_PARAM(0x00 == u32DataPkgNum);
    HI_LOG_CHECK_PARAM(CIPHER_MAX_MULTIPAD_NUM <= u32DataPkgNum);

    HI_DBG_PrintU32(hCipher);
    HI_DBG_PrintU32(u32DataPkgNum);

    CHECK_CIPHER_OPEN();

    ret = mpi_symc_crypto_multi(hCipher, pstDataPkg, u32DataPkgNum, SYMC_OPERATION_DECRYPT);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_crypto_multi, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_GetTag(HI_HANDLE hCipher, HI_U8 *pu8Tag, HI_U32 *pu32TagLen)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu8Tag);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32TagLen);
    HI_LOG_CHECK_PARAM(MAX_TAG_LEN != *pu32TagLen);

    HI_DBG_PrintU32(hCipher);

    CHECK_CIPHER_OPEN();

    ret = mpi_aead_get_tag(hCipher, pu8Tag, pu32TagLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_aead_get_tag, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_GetHandleConfig(HI_HANDLE hCipherHandle, CRYP_CIPHER_CTRL_S * pstCtrl)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstCtrl);

    CHECK_CIPHER_OPEN();

    ret = mpi_symc_get_config(hCipherHandle, pstCtrl);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_symc_get_config, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_KladEncryptKey(CRYP_CIPHER_CA_TYPE_E enRootKey,
                                  CRYP_CIPHER_KLAD_TARGET_E enTarget,
                                  const HI_U8 *pu8CleanKey, HI_U8* pu8EcnryptKey, HI_U32 u32KeyLen)
{
    HI_S32 ret = HI_FAILURE;
    HI_U32 i;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_UNF_CIPHER_KEY_SRC_BUTT <= enRootKey);
    HI_LOG_CHECK_PARAM(HI_UNF_CIPHER_KLAD_TARGET_BUTT <= enTarget);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8CleanKey);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8EcnryptKey);
    HI_LOG_CHECK_PARAM(0 != (u32KeyLen % AES_BLOCK_SIZE));
    HI_LOG_CHECK_PARAM(0 == u32KeyLen);

    CHECK_CIPHER_OPEN();

    for (i=0; i<u32KeyLen; i+=AES_BLOCK_SIZE)
    {
        ret = mpi_klad_encrypt_key(enRootKey, enTarget, pu8CleanKey+i, pu8EcnryptKey+i);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_PrintFuncErr(mpi_klad_encrypt_key, ret);
            return ret;
        }
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_GetRandomNumber(HI_U32 *pu32RandomNumber)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu32RandomNumber);

    ret = mpi_trng_get_random(pu32RandomNumber, 0);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_HashInit(const CRYP_CIPHER_HASH_ATTS_S *pstHashAttr, HI_HANDLE *pHashHandle)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstHashAttr);
    HI_LOG_CHECK_PARAM(HI_NULL == pHashHandle);

    HI_DBG_PrintU32(pstHashAttr->eShaType);
    HI_DBG_PrintU32(pstHashAttr->u32HMACKeyLen);

    CHECK_CIPHER_OPEN();

    ret = mpi_hash_start(pHashHandle,
                         pstHashAttr->eShaType,
                         pstHashAttr->pu8HMACKey,
                         pstHashAttr->u32HMACKeyLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_start, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_HashUpdate(HI_HANDLE hHashHandle, const HI_U8 *pu8InputData, HI_U32 u32InputDataLen)
{
    HI_S32 ret = HI_FAILURE;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu8InputData);

    HI_DBG_PrintU32(hHashHandle);
    HI_DBG_PrintU32(u32InputDataLen);

    CHECK_CIPHER_OPEN();

    ret = mpi_hash_update(hHashHandle, pu8InputData, u32InputDataLen, HASH_CHUNCK_SRC_USER);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_update, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_HashFinal(HI_HANDLE hHashHandle, HI_U8 *pu8OutputHash)
{
    HI_S32 ret = HI_FAILURE;
    HI_U32 u32HashLen = 0x00;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pu8OutputHash);
    HI_LOG_CHECK_PARAM(HI_INVALID_HANDLE == hHashHandle);

    HI_DBG_PrintU32(hHashHandle);

    CHECK_CIPHER_OPEN();

    ret = mpi_hash_finish(hHashHandle, pu8OutputHash, &u32HashLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_finish, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

static HI_S32 CHECK_RSA_PRI_key(const CRYP_CIPHER_RSA_PRI_KEY_S *pstPriKey)
{
    HI_LOG_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey);
    HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8N);
    HI_LOG_CHECK_PARAM(RSA_MIN_KEY_LEN > pstPriKey->u16NLen);
    HI_LOG_CHECK_PARAM(RSA_MAX_KEY_LEN < pstPriKey->u16NLen);

    if (HI_NULL == pstPriKey->pu8D)
    {
        HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8P);
        HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8Q);
        HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8DP);
        HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8DQ);
        HI_LOG_CHECK_PARAM(HI_NULL == pstPriKey->pu8QP);
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen/2 != pstPriKey->u16PLen);
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen/2 != pstPriKey->u16QLen);
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen/2 != pstPriKey->u16DPLen);
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen/2 != pstPriKey->u16DQLen);
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen/2 != pstPriKey->u16QPLen);
    }
    else
    {
        HI_LOG_CHECK_PARAM(pstPriKey->u16NLen != pstPriKey->u16DLen);
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_RsaPublicEncrypt(const CRYP_CIPHER_RSA_PUB_ENC_S *pstRsaEnc,
                                  const HI_U8 *pu8Input, HI_U32 u32InLen,
                                  HI_U8 *pu8Output, HI_U32 *pu32OutLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaEnc);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Input);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Output);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32OutLen);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaEnc->stPubKey.pu8N);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaEnc->stPubKey.pu8E);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_USER != pstRsaEnc->enCaType);
    HI_LOG_CHECK_PARAM(RSA_MIN_KEY_LEN > pstRsaEnc->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(RSA_MAX_KEY_LEN < pstRsaEnc->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(pstRsaEnc->stPubKey.u16NLen < pstRsaEnc->stPubKey.u16ELen);

    HI_DBG_PrintU32(pstRsaEnc->stPubKey.u16NLen);
    HI_DBG_PrintU32(pstRsaEnc->enScheme);
    HI_DBG_PrintU32(u32InLen);

    CHECK_CIPHER_OPEN();

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public = HI_TRUE;
    key.klen = pstRsaEnc->stPubKey.u16NLen;
    key.N = pstRsaEnc->stPubKey.pu8N;
    CIPHER_GET_PUB_EXPONENT(&key.e, pstRsaEnc);

    ret = mpi_rsa_encrypt(&key, pstRsaEnc->enScheme,
                          pu8Input, u32InLen,
                          pu8Output, pu32OutLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_encrypt, ret);
        crypto_zeroize(&key, sizeof(key));
        return ret;
    }

    crypto_zeroize(&key, sizeof(key));
    HI_UNF_FuncExit();
    return HI_SUCCESS;

}

HI_S32 CRYP_CIPHER_RsaPrivateDecrypt(const CRYP_CIPHER_RSA_PRI_ENC_S *pstRsaDec,
                                   const HI_U8 *pu8Input, HI_U32 u32InLen,
                                   HI_U8 *pu8Output, HI_U32 *pu32OutLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaDec);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_BUTT <= pstRsaDec->enCaType);

    ret = CHECK_RSA_PRI_key(&pstRsaDec->stPriKey);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CHECK_RSA_PRI_key, ret);
        return ret;
    }

    HI_LOG_CHECK_PARAM(HI_NULL == pu8Input);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Output);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32OutLen);

    HI_DBG_PrintU32(pstRsaDec->stPriKey.u16NLen);
    HI_DBG_PrintU32(pstRsaDec->enScheme);
    HI_DBG_PrintU32(u32InLen);

    CHECK_CIPHER_OPEN();

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public  = HI_FALSE;
    key.klen    = pstRsaDec->stPriKey.u16NLen;
    key.N       = pstRsaDec->stPriKey.pu8N;
    key.d       = pstRsaDec->stPriKey.pu8D;
    key.catype  = pstRsaDec->enCaType;
    key.p       = pstRsaDec->stPriKey.pu8P;
    key.q       = pstRsaDec->stPriKey.pu8Q;
    key.dP       = pstRsaDec->stPriKey.pu8DP;
    key.dQ       = pstRsaDec->stPriKey.pu8DQ;
    key.qP       = pstRsaDec->stPriKey.pu8QP;

    ret = mpi_rsa_decrypt(&key,
                          pstRsaDec->enScheme,
                          pu8Input, u32InLen,
                          pu8Output, pu32OutLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_decrypt, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_RsaPrivateEncrypt(const CRYP_CIPHER_RSA_PRI_ENC_S *pstRsaEnc,
                                   const HI_U8 *pu8Input, HI_U32 u32InLen,
                                   HI_U8 *pu8Output, HI_U32 *pu32OutLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaEnc);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_BUTT <= pstRsaEnc->enCaType);

    ret = CHECK_RSA_PRI_key(&pstRsaEnc->stPriKey);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CHECK_RSA_PRI_key, ret);
        return ret;
    }

    HI_LOG_CHECK_PARAM(HI_NULL == pu8Input);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Output);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32OutLen);

    HI_DBG_PrintU32(pstRsaEnc->stPriKey.u16NLen);
    HI_DBG_PrintU32(pstRsaEnc->enScheme);
    HI_DBG_PrintU32(u32InLen);

    CHECK_CIPHER_OPEN();

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public  = HI_FALSE;
    key.klen    = pstRsaEnc->stPriKey.u16NLen;
    key.N       = pstRsaEnc->stPriKey.pu8N;
    key.d       = pstRsaEnc->stPriKey.pu8D;
    key.catype  = pstRsaEnc->enCaType;
    key.p       = pstRsaEnc->stPriKey.pu8P;
    key.q       = pstRsaEnc->stPriKey.pu8Q;
    key.dP       = pstRsaEnc->stPriKey.pu8DP;
    key.dQ       = pstRsaEnc->stPriKey.pu8DQ;
    key.qP       = pstRsaEnc->stPriKey.pu8QP;

    ret = mpi_rsa_encrypt(&key,
                          pstRsaEnc->enScheme,
                          pu8Input, u32InLen,
                          pu8Output, pu32OutLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_encrypt, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}


HI_S32 CRYP_CIPHER_RsaPublicDecrypt(const CRYP_CIPHER_RSA_PUB_ENC_S *pstRsaDec,
                               const HI_U8 *pu8Input, HI_U32 u32InLen,
                               HI_U8 *pu8Output, HI_U32 *pu32OutLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaDec);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Input);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8Output);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32OutLen);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_USER != pstRsaDec->enCaType);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaDec->stPubKey.pu8N);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaDec->stPubKey.pu8E);
    HI_LOG_CHECK_PARAM(RSA_MIN_KEY_LEN > pstRsaDec->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(RSA_MAX_KEY_LEN < pstRsaDec->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(pstRsaDec->stPubKey.u16NLen < pstRsaDec->stPubKey.u16ELen);

    HI_DBG_PrintU32(pstRsaDec->enScheme);
    HI_DBG_PrintU32(u32InLen);
    HI_DBG_PrintU32(pstRsaDec->stPubKey.u16NLen);

    CHECK_CIPHER_OPEN();

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public  = HI_TRUE;
    key.klen    = pstRsaDec->stPubKey.u16NLen;
    key.N       = pstRsaDec->stPubKey.pu8N;
    CIPHER_GET_PUB_EXPONENT(&key.e, pstRsaDec);

    ret = mpi_rsa_decrypt(&key, pstRsaDec->enScheme, pu8Input, u32InLen, pu8Output, pu32OutLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_decrypt, ret);
        crypto_zeroize(&key, sizeof(key));
        return ret;
    }

    crypto_zeroize(&key, sizeof(key));
    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

static HI_S32 CRYP_CIPHER_Hash(const CRYP_CIPHER_HASH_TYPE_E enShaType,
                               const HI_U8 *pu8InData, HI_U32 u32InDataLen,
                               HI_U8 *pu8HashData, HI_U32 *pu32Hlen)
{
    HI_S32 ret = HI_FAILURE;
    HI_HANDLE hash_id;

    ret = mpi_hash_start(&hash_id, enShaType, HI_NULL, 0);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_start, ret);
        return ret;
    }

    ret = mpi_hash_update(hash_id, pu8InData, u32InDataLen, HASH_CHUNCK_SRC_USER);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_update, ret);
        return ret;
    }

    ret = mpi_hash_finish(hash_id, pu8HashData, pu32Hlen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_hash_finish, ret);
        return ret;
    }

    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_RsaSign(const CRYP_CIPHER_RSA_SIGN_S *pstRsaSign,
                             const HI_U8 *pu8InData, HI_U32 u32InDataLen,
                             const HI_U8 *pu8HashData,
                             HI_U8 *pu8OutSign, HI_U32 *pu32OutSignLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;
    HI_U8 hash[HASH_RESULT_MAX_SIZE] = {0};
    const HI_U8 *ptr = HI_NULL;
    HI_U32 u32Hlen = 0;
    CRYP_CIPHER_HASH_TYPE_E enShaType = 0;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaSign);
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_BUTT <= pstRsaSign->enCaType);

    ret = CHECK_RSA_PRI_key(&pstRsaSign->stPriKey);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CHECK_RSA_PRI_key, ret);
        return ret;
    }

    HI_LOG_CHECK_PARAM(HI_NULL == pu8OutSign);
    HI_LOG_CHECK_PARAM(HI_NULL == pu32OutSignLen);

    HI_LOG_CHECK_PARAM((HI_NULL == pu8InData) && (HI_NULL == pu8HashData));

    HI_DBG_PrintU32(pstRsaSign->enScheme);
    HI_DBG_PrintU32(pstRsaSign->stPriKey.u16NLen);

    CHECK_CIPHER_OPEN();

    ret = CRYP_CIPHER_GetRsaAttr(pstRsaSign->enScheme, &u32Hlen, &enShaType);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYP_CIPHER_GetRsaAttr, ret);
        return ret;
    }

    HI_DBG_PrintU32(u32Hlen);
    HI_DBG_PrintU32(enShaType);

    /* hash value of context,if NULL, compute hash = Hash(pu8InData */
    if (HI_NULL == pu8HashData)
    {
        ret = CRYP_CIPHER_Hash(enShaType, pu8InData, u32InDataLen, hash, &u32Hlen);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_PrintFuncErr(CRYP_CIPHER_Hash, ret);
            return ret;
        }
        ptr = hash;
    }
    else
    {
        ptr = pu8HashData;
    }

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public  = HI_FALSE;
    key.klen    = pstRsaSign->stPriKey.u16NLen;
    key.N       = pstRsaSign->stPriKey.pu8N;
    key.d       = pstRsaSign->stPriKey.pu8D;
    key.catype  = pstRsaSign->enCaType;
    key.p       = pstRsaSign->stPriKey.pu8P;
    key.q       = pstRsaSign->stPriKey.pu8Q;
    key.dP       = pstRsaSign->stPriKey.pu8DP;
    key.dQ       = pstRsaSign->stPriKey.pu8DQ;
    key.qP       = pstRsaSign->stPriKey.pu8QP;

    ret = mpi_rsa_sign_hash(&key, pstRsaSign->enScheme, ptr, u32Hlen, pu8OutSign, pu32OutSignLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_sign_hash, ret);
        return ret;
    }

    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

HI_S32 CRYP_CIPHER_RsaVerify(const CRYP_CIPHER_RSA_VERIFY_S *pstRsaVerify,
                               const HI_U8 *pu8InData, HI_U32 u32InDataLen,
                               const HI_U8 *pu8HashData,
                               const HI_U8 *pu8InSign, HI_U32 u32InSignLen)
{
    HI_S32 ret = HI_FAILURE;
    cryp_rsa_key key;
    HI_U8 hash[HASH_RESULT_MAX_SIZE] = {0};
    HI_U32 u32Hlen = 0;
    const HI_U8 *ptr = HI_NULL;
    CRYP_CIPHER_HASH_TYPE_E enShaType = 0;

    HI_UNF_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaVerify);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8InSign);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaVerify->stPubKey.pu8N);
    HI_LOG_CHECK_PARAM(HI_NULL == pstRsaVerify->stPubKey.pu8E);
    HI_LOG_CHECK_PARAM(RSA_MIN_KEY_LEN > pstRsaVerify->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(RSA_MAX_KEY_LEN < pstRsaVerify->stPubKey.u16NLen);
    HI_LOG_CHECK_PARAM(HI_NULL == pu8InData && HI_NULL == pu8HashData);
    HI_LOG_CHECK_PARAM(pstRsaVerify->stPubKey.u16NLen < pstRsaVerify->stPubKey.u16ELen);

    HI_DBG_PrintU32(pstRsaVerify->enScheme);
    HI_DBG_PrintU32(pstRsaVerify->stPubKey.u16NLen);
    HI_DBG_PrintU32(pstRsaVerify->enScheme);

    CHECK_CIPHER_OPEN();

    ret = CRYP_CIPHER_GetRsaAttr(pstRsaVerify->enScheme, &u32Hlen, &enShaType);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYP_CIPHER_GetRsaAttr, ret);
        return ret;
    }

    HI_DBG_PrintU32(enShaType);

    /* hash value of context,if NULL, compute hash = Hash(pu8InData */
    if (HI_NULL == pu8HashData)
    {
        ret = CRYP_CIPHER_Hash(enShaType, pu8InData, u32InDataLen, hash, &u32Hlen);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_PrintFuncErr(CRYP_CIPHER_Hash, ret);
            return ret;
        }
        ptr = hash;
    }
    else
    {
        ptr = pu8HashData;
    }

    HI_DBG_PrintU32(u32Hlen);

    ret = memset_s(&key, sizeof(key), 0, sizeof(key));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    key.public  = HI_TRUE;
    key.klen    = pstRsaVerify->stPubKey.u16NLen;
    key.N       = pstRsaVerify->stPubKey.pu8N;
    CIPHER_GET_PUB_EXPONENT(&key.e, pstRsaVerify);

    ret = mpi_rsa_verify_hash(&key, pstRsaVerify->enScheme, ptr, u32Hlen, pu8InSign, u32InSignLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(mpi_rsa_verify_hash, ret);
        crypto_zeroize(&key, sizeof(key));
        return ret;
    }

    crypto_zeroize(&key, sizeof(key));
    HI_UNF_FuncExit();
    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== Compat Code end ====*/
