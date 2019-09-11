/*****************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : mpi_cipher.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/

#include "mpi_cipher.h"

/*************************** Internal Structure Definition *******************/
/** \addtogroup      symc */
/** @{*/  /** <!-- [mpi]*/


/** @}*/  /** <!-- ==== Structure Definition end ====*/

/******************************* API Code *****************************/
/** \addtogroup      symc */
/** @{*/  /** <!-- [mpi]*/

s32 mpi_symc_create(u32 *hd)
{
    s32 ret = HI_FAILURE;
    symc_create_t create = {0};

    HI_LOG_FuncEnter();

    ret = memset_s(&create, sizeof(create), 0, sizeof(create));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_CREATEHANDLE, &create);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    *hd = create.id;

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_symc_destroy(u32 id)
{
    symc_destroy_t destroy = {0};
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&destroy, sizeof(destroy), 0, sizeof(destroy));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    destroy.id = id;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_DESTROYHANDLE, &destroy);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_symc_config(u32 id,
                    u32 hard_key,
                    CRYP_CIPHER_ALG_E enAlg,
                    CRYP_CIPHER_WORK_MODE_E enWorkMode,
                    CRYP_CIPHER_BIT_WIDTH_E enBitWidth,
                    CRYP_CIPHER_KEY_LENGTH_E enKeyLen,
                    CRYP_CIPHER_SM1_ROUND_E sm1_round_num,
                    u8 *fkey, u8 *skey, u32 keylen,
                    u8 *iv, u32 ivlen, u32 iv_usage,
                    compat_addr aad, u32 alen, u32 tlen)
{
    symc_config_t config;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&config, sizeof(config), 0, sizeof(config));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    config.id = id;
    config.hard_key = hard_key;
    config.alg = enAlg;
    config.mode = enWorkMode;
    config.klen = enKeyLen;
    config.iv_usage = iv_usage;
    config.sm1_round_num = sm1_round_num;
    config.aad = aad;
    config.alen = alen;
    config.tlen = tlen;
    config.ivlen = ivlen;
    config.width = enBitWidth;

    HI_DBG_PrintU32(ivlen);
    HI_DBG_PrintH32(enKeyLen);

    if (HI_NULL != fkey)
    {
        ret = memcpy_s(config.fkey, sizeof(config.fkey), fkey, keylen);
        if (0 != ret)
        {
            HI_LOG_PrintFuncErr(memcpy_s, ret);
            return HI_ERR_CIPHER_MEMCPY_S_FAILED;
        }

        if (CRYP_CIPHER_ALG_SM1 == enAlg)
        {
            /* SM1 key:EK+AK+SK,fkey:EK+AK. */
            ret = memcpy_s(config.fkey + SYMC_SM1_SK_SIZE, SYMC_SM1_SK_SIZE,
                      fkey + SYMC_SM1_SK_SIZE, SYMC_SM1_SK_SIZE);
            if (0 != ret)
            {
                HI_LOG_PrintFuncErr(memcpy_s, ret);
                crypto_zeroize(&config, sizeof(config));
                return HI_ERR_CIPHER_MEMCPY_S_FAILED;
            }
        }
    }
    if (HI_NULL != iv)
    {
        ret = memcpy_s(config.iv, sizeof(config.iv), iv, ivlen);
        if (0 != ret)
        {
            HI_LOG_PrintFuncErr(memcpy_s, ret);
            crypto_zeroize(&config, sizeof(config));
            return HI_ERR_CIPHER_MEMCPY_S_FAILED;
        }
    }
    if (HI_NULL != skey)
    {
        ret = memcpy_s(config.skey, sizeof(config.skey), skey, keylen);
        if (0 != ret)
        {
            HI_LOG_PrintFuncErr(memcpy_s, ret);
            crypto_zeroize(&config, sizeof(config));
            return HI_ERR_CIPHER_MEMCPY_S_FAILED;
        }
    }

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_CONFIGHANDLE, &config);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        crypto_zeroize(&config, sizeof(config));
        return ret;
    }

    crypto_zeroize(&config, sizeof(config));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_symc_get_config(u32 id, CRYP_CIPHER_CTRL_S *ctrl)
{
    symc_get_config_t get_config;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&get_config, sizeof(get_config), 0, sizeof(get_config));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    get_config.id = id;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_GET_CONFIG, &get_config);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    ret = memcpy_s(ctrl, sizeof(CRYP_CIPHER_CTRL_S), &get_config.ctrl, sizeof(CRYP_CIPHER_CTRL_S));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memcpy_s, ret);
        crypto_zeroize(&get_config, sizeof(get_config));
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    crypto_zeroize(&get_config, sizeof(get_config));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_symc_crypto(u32 id, const compat_addr input,
                      const compat_addr output, u32 length,
                      u32 operation)
{
    symc_encrypt_t encrypt;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&encrypt, sizeof(encrypt), 0, sizeof(encrypt));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    encrypt.id = id;
    encrypt.input = input;
    encrypt.output = output;
    encrypt.length = length;
    encrypt.operation = operation;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_ENCRYPT, &encrypt);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_symc_crypto_multi(u32 id, const void *pkg, u32 pkg_num, u32 operation)
{
    symc_encrypt_multi_t encrypt_multi;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&encrypt_multi, sizeof(encrypt_multi), 0, sizeof(encrypt_multi));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    encrypt_multi.id = id;
    ADDR_VIA_CONST(encrypt_multi.pkg) = pkg;
    encrypt_multi.pkg_num = pkg_num;
    encrypt_multi.operation = operation;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_ENCRYPTMULTI, &encrypt_multi);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_klad_encrypt_key(u32 keysel, u32 target, const u8 clear[AES_BLOCK_SIZE], u8 encrypt[AES_BLOCK_SIZE])
{
    klad_key_t klad;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = memset_s(&klad, sizeof(klad), 0, sizeof(klad));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    klad.keysel = keysel;
    klad.target = target;
    ret = memcpy_s(klad.clear, AES_BLOCK_SIZE, clear, AES_BLOCK_SIZE);
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memcpy_s, ret);
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    ret = CRYPTO_IOCTL(CRYPTO_CMD_KLAD_KEY, &klad);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    ret = memcpy_s(encrypt, AES_BLOCK_SIZE, klad.encrypt, AES_BLOCK_SIZE);
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memcpy_s, ret);
        crypto_zeroize(&klad, sizeof(klad));
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    crypto_zeroize(&klad, sizeof(klad));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_aead_get_tag(u32 id, u8 *tag, u32 *taglen)
{
    s32 ret = HI_FAILURE;
    aead_tag_t aead_tag;

    HI_LOG_FuncEnter();

    ret = memset_s(&aead_tag, sizeof(aead_tag), 0, sizeof(aead_tag));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    aead_tag.id = id;
    aead_tag.taglen = *taglen;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_SYMC_GETTAG, &aead_tag);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    ret = memcpy_s(tag, AEAD_TAG_SIZE, aead_tag.tag, aead_tag.taglen);
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memcpy_s, ret);
        crypto_zeroize(&aead_tag, sizeof(aead_tag));
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    *taglen = aead_tag.taglen;

    crypto_zeroize(&aead_tag, sizeof(aead_tag));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_hash_start(u32 *id, CRYP_CIPHER_HASH_TYPE_E type,
                    const u8 *key, u32 keylen)
{
    s32 ret = HI_FAILURE;
    hash_start_t start;

    HI_LOG_FuncEnter();

    ret = memset_s(&start, sizeof(start), 0, sizeof(start));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    start.id = 0;
    start.type = type;
    start.keylen = keylen;
    ADDR_VIA_CONST(start.key) = key;

    ret =  CRYPTO_IOCTL(CRYPTO_CMD_HASH_START, &start);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    *id = start.id;

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_hash_update(u32 id, const u8 *input, u32 length, hash_chunk_src src)
{
    s32 ret = HI_FAILURE;
    hash_update_t update;

    HI_LOG_FuncEnter();

    ret = memset_s(&update, sizeof(update), 0, sizeof(update));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    update.id = id;
    ADDR_VIA_CONST(update.input) = input;
    update.length = length;
    update.src = src;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_HASH_UPDATE, &update);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_hash_finish(u32 id, u8 *hash, u32 *hashlen)
{
    s32 ret = HI_FAILURE;
    hash_finish_t finish;

    HI_LOG_FuncEnter();

    ret = memset_s(&finish, sizeof(finish), 0, sizeof(finish));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    finish.id = id;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_HASH_FINISH, &finish);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        return ret;
    }

    ret = memcpy_s(hash, HASH_RESULT_MAX_SIZE, finish.hash, finish.hashlen);
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memcpy_s, ret);
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    *hashlen = finish.hashlen;

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_rsa_encrypt(const cryp_rsa_key *key,
                    CRYP_CIPHER_RSA_ENC_SCHEME_E enScheme,
                    const u8 *in, u32 inlen,
                    u8 *out, u32 *outlen)
{
    s32 ret = HI_FAILURE;
    rsa_info_t rsa_info;

    HI_LOG_FuncEnter();

    ret = memset_s(&rsa_info, sizeof(rsa_info_t), 0, sizeof(rsa_info_t));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    ADDR_VIA_CONST(rsa_info.N) = key->N;
    ADDR_VIA_CONST(rsa_info.d) = key->d;
    ADDR_VIA_CONST(rsa_info.p) = key->p;
    ADDR_VIA_CONST(rsa_info.q) = key->q;
    ADDR_VIA_CONST(rsa_info.dP) = key->dP;
    ADDR_VIA_CONST(rsa_info.dQ) = key->dQ;
    ADDR_VIA_CONST(rsa_info.qP) = key->qP;

    rsa_info.e = key->e;
    rsa_info.public = key->public;
    rsa_info.klen = key->klen;
    rsa_info.enScheme = enScheme;
    ADDR_VIA_CONST(rsa_info.in) = in;
    rsa_info.inlen = inlen;
    ADDR_VIA(rsa_info.out) = out;
    rsa_info.outlen = key->klen;
    rsa_info.catype = key->catype;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_RSA_ENC, &rsa_info);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
        return ret;
    }

    *outlen = rsa_info.outlen;

    crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_rsa_decrypt(const cryp_rsa_key *key,
                    CRYP_CIPHER_RSA_ENC_SCHEME_E enScheme,
                    const u8 *in, u32 inlen,
                    u8 *out, u32 *outlen)
{
    s32 ret = HI_FAILURE;
    rsa_info_t rsa_info;

    HI_LOG_FuncEnter();

    ret = memset_s(&rsa_info, sizeof(rsa_info), 0, sizeof(rsa_info));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    ADDR_VIA_CONST(rsa_info.N) = key->N;
    ADDR_VIA_CONST(rsa_info.d) = key->d;
    ADDR_VIA_CONST(rsa_info.p) = key->p;
    ADDR_VIA_CONST(rsa_info.q) = key->q;
    ADDR_VIA_CONST(rsa_info.dP) = key->dP;
    ADDR_VIA_CONST(rsa_info.dQ) = key->dQ;
    ADDR_VIA_CONST(rsa_info.qP) = key->qP;

    rsa_info.e = key->e;
    rsa_info.public = key->public;
    rsa_info.klen = key->klen;
    rsa_info.enScheme = enScheme;
    ADDR_VIA_CONST(rsa_info.in) = in;
    rsa_info.inlen = inlen;
    ADDR_VIA(rsa_info.out) = out;
    rsa_info.outlen = key->klen;
    rsa_info.catype = key->catype;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_RSA_DEC, &rsa_info);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
        return ret;
    }

    *outlen = rsa_info.outlen;

    crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_rsa_sign_hash(const cryp_rsa_key *key,
                      CRYP_CIPHER_RSA_SIGN_SCHEME_E enScheme,
                      const u8* hash, u32 hlen,
                      u8 *sign, u32 *signlen)
{
    s32 ret = HI_FAILURE;
    rsa_info_t rsa_info;

    HI_LOG_FuncEnter();

    ret = memset_s(&rsa_info, sizeof(rsa_info), 0, sizeof(rsa_info));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    ADDR_VIA_CONST(rsa_info.N) = key->N;
    ADDR_VIA_CONST(rsa_info.d) = key->d;
    ADDR_VIA_CONST(rsa_info.p) = key->p;
    ADDR_VIA_CONST(rsa_info.q) = key->q;
    ADDR_VIA_CONST(rsa_info.dP) = key->dP;
    ADDR_VIA_CONST(rsa_info.dQ) = key->dQ;
    ADDR_VIA_CONST(rsa_info.qP) = key->qP;

    rsa_info.e = key->e;
    rsa_info.public = key->public;
    rsa_info.klen = key->klen;
    rsa_info.enScheme = enScheme;
    ADDR_VIA_CONST(rsa_info.in) = hash;
    rsa_info.inlen = hlen;
    ADDR_VIA(rsa_info.out) = sign;
    rsa_info.outlen = key->klen;
    rsa_info.catype = key->catype;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_RSA_SIGN, &rsa_info);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
        return ret;
    }

    *signlen = rsa_info.outlen;

    crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_rsa_verify_hash(const cryp_rsa_key *key,
                       CRYP_CIPHER_RSA_SIGN_SCHEME_E enScheme,
                       const u8 *hash, u32 hlen,
                       const u8 *sign, u32 signlen)
{
    s32 ret = HI_FAILURE;
    rsa_info_t rsa_info;

    HI_LOG_FuncEnter();

    ret = memset_s(&rsa_info, sizeof(rsa_info_t), 0, sizeof(rsa_info_t));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    ADDR_VIA_CONST(rsa_info.N) = key->N;

    rsa_info.e = key->e;
    rsa_info.public = key->public;
    rsa_info.klen = key->klen;
    rsa_info.enScheme = enScheme;
    ADDR_VIA_CONST(rsa_info.in) = sign;
    rsa_info.inlen = signlen;
    ADDR_VIA_CONST(rsa_info.out) = hash;
    rsa_info.outlen = hlen;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_RSA_VERIFY, &rsa_info);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(CRYPTO_IOCTL, ret);
        crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
        return ret;
    }

    crypto_zeroize(&rsa_info.e, sizeof(rsa_info.e));
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 mpi_trng_get_random(u32 *randnum, u32 timeout)
{
    s32 ret = HI_FAILURE;
    trng_t trng;

    HI_LOG_FuncEnter();

    ret = memset_s(&trng, sizeof(trng), 0, sizeof(trng));
    if (0 != ret)
    {
        HI_LOG_PrintFuncErr(memset_s, ret);
        return HI_ERR_CIPHER_MEMSET_S_FAILED;
    }

    trng.timeout = timeout;

    ret = CRYPTO_IOCTL(CRYPTO_CMD_TRNG, &trng);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }

    *randnum = trng.randnum;
    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== API Code end ====*/
