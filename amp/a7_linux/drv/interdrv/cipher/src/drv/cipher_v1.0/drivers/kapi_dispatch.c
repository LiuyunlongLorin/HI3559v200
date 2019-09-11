/*****************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : kapi_dispatch.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/

#include "drv_osal_lib.h"
#include "cryp_symc.h"
#include "ext_alg.h"
#include "hi_drv_compat.h"

/*************************** Internal Structure Definition *******************/

/* ! \max pakage numher of symc mutli encrypt */
#define SYMC_MULTI_MAX_PKG      (0x1000)

#define RSA_PUBLIC_BUFFER_NUM   (0x03)
#define RSA_PRIVATE_BUFFER_NUM  (0x07)

#define MAX_CENC_SUB_SAMPLE     (100)

typedef s32 (*hi_drv_func)(void *param);

typedef struct
{
    const char *name;
    hi_drv_func func;
    u32 cmd;
}crypto_dispatch_func;

/** @}*/  /** <!-- ==== Structure Definition end ====*/

/******************************* API Code *****************************/
/** \addtogroup      link*/
/** @{*/  /** <!-- [link]*/

static s32 dispatch_symc_create_handle(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_create_t *symc_create = argp;

    HI_LOG_FuncEnter();

    /* allocate a aes channel */
    ret = kapi_symc_create(&symc_create->id);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_create, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_destroy_handle(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_destroy_t *destroy = argp;

    HI_LOG_FuncEnter();

    ret = kapi_symc_destroy(destroy->id);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_destroy, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_config(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_config_t *config = argp;

    HI_LOG_FuncEnter();

    ret = kapi_symc_config(config->id,
                           config->hard_key,
                           config->alg,
                           config->mode,
                           config->width,
                           config->klen,
                           config->sm1_round_num,
                           (HI_U8*)config->fkey,
                           (u8*)config->skey,
                           (u8*)config->iv,
                           config->ivlen,
                           config->iv_usage,
                           config->aad,
                           config->alen,
                           config->tlen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_config, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_encrypt(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_encrypt_t *encrypt = argp;

    HI_LOG_FuncEnter();

    if((SYMC_OPERATION_ENCRYPT == encrypt->operation)
        || (SYMC_OPERATION_DECRYPT == encrypt->operation))
    {
        ret = cipher_check_mmz_phy_addr(ADDR_U64(encrypt->input), encrypt->length);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_ERROR("Invalid input mmz phy addr for crypt.\n");
            HI_LOG_PrintFuncErr(cipher_check_mmz_phy_addr, ret);
            return ret;
        }

        ret = cipher_check_mmz_phy_addr(ADDR_U64(encrypt->output), encrypt->length);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_ERROR("Invalid output mmz phy addr for crypt.\n");
            HI_LOG_PrintFuncErr(cipher_check_mmz_phy_addr, ret);
            return ret;
        }

        ret = kapi_symc_crypto(encrypt->id,
                               encrypt->input,
                               encrypt->output,
                               encrypt->length,
                               encrypt->operation,
                               encrypt->last);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_PrintFuncErr(kapi_symc_crypto, ret);
            return ret;
        }
    }
    else if ((SYMC_OPERATION_ENCRYPT_VIA == encrypt->operation)
         || (SYMC_OPERATION_DECRYPT_VIA == encrypt->operation))
    {
        ret = kapi_symc_crypto_via(encrypt->id,
                           encrypt->input,
                           encrypt->output,
                           encrypt->length,
                           encrypt->operation,
                           encrypt->last,
                           HI_TRUE);
        if (HI_SUCCESS != ret)
        {
            HI_LOG_PrintFuncErr(kapi_symc_crypto_via, ret);
            return ret;
        }
    }
    else
    {
        HI_LOG_ERROR("encrypt operation(0x%x) is unsupported.\n", encrypt->operation);
        return HI_ERR_CIPHER_UNSUPPORTED;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_encrypt_multi(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_encrypt_multi_t *encrypt_mutli = argp;

    HI_LOG_FuncEnter();

    HI_LOG_DEBUG("operation %d\n", encrypt_mutli->operation);
    ret = kapi_symc_crypto_multi(encrypt_mutli->id,
                                 ADDR_VIA(encrypt_mutli->pkg),
                                 encrypt_mutli->pkg_num,
                                 encrypt_mutli->operation,
                                 HI_TRUE);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_crypto_multi, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_get_tag(void *argp)
{
    s32 ret = HI_FAILURE;
    aead_tag_t *aead_tag = argp;

    HI_LOG_FuncEnter();

    ret = kapi_aead_get_tag(aead_tag->id,
                            aead_tag->tag,
                            &aead_tag->taglen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_aead_get_tag, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_symc_get_config(void *argp)
{
    s32 ret = HI_FAILURE;
    symc_get_config_t *get_config = argp;

    HI_LOG_FuncEnter();

    ret = kapi_symc_get_config(get_config->id, &get_config->ctrl);

    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_get_config, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_klad_key(void *argp)
{
    s32 ret = HI_FAILURE;
    klad_key_t *klad = argp;

    HI_LOG_FuncEnter();

    ret = klad_encrypt_key(klad->keysel, klad->target, klad->clear, klad->encrypt);
    if (HI_SUCCESS != ret)
    {
       HI_LOG_PrintFuncErr(klad_encrypt_key, ret);
       return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_hash_start(void *argp)
{
    s32 ret = HI_FAILURE;
    hash_start_t *start = argp;
    u8 *key = HI_NULL;

    HI_LOG_FuncEnter();
    HI_LOG_CHECK_PARAM(CRYP_CIPHER_HASH_TYPE_BUTT <= start->type);

    if (CRYP_CIPHER_HASH_TYPE_SM3 == start->type)
    {
        HI_LOG_ERROR("Sm3 is unsupported.\n");
        return HI_ERR_CIPHER_UNSUPPORTED;
    }

    if (CRYP_CIPHER_HASH_TYPE_HMAC_SHA1 == start->type
        || CRYP_CIPHER_HASH_TYPE_HMAC_SHA224 == start->type
        || CRYP_CIPHER_HASH_TYPE_HMAC_SHA256 == start->type
        || CRYP_CIPHER_HASH_TYPE_HMAC_SHA384 == start->type
        || CRYP_CIPHER_HASH_TYPE_HMAC_SHA512 == start->type)
    {

        HI_LOG_CHECK_PARAM(MAX_MALLOC_BUF_SIZE < start->keylen);
        HI_LOG_CHECK_PARAM(HI_NULL == ADDR_VIA(start->key));

        key = (u8*)crypto_calloc(1, start->keylen);
        if (HI_NULL == key)
        {
            HI_LOG_PrintErrCode(HI_ERR_CIPHER_FAILED_MEM);
            HI_LOG_PrintFuncErr(crypto_calloc, ret);
            return HI_ERR_CIPHER_FAILED_MEM;
        }

        CHECK_EXIT(crypto_copy_from_user(key, ADDR_VIA(start->key), start->keylen));
    }

    CHECK_EXIT(kapi_hash_start(&start->id, start->type, key, start->keylen));

    if (HI_NULL != key)
    {
        crypto_zeroize(key, start->keylen);
        crypto_free(key);
        key = HI_NULL;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;

exit__:
    if (HI_NULL != key)
    {
        crypto_zeroize(key, start->keylen);
        crypto_free(key);
        key = HI_NULL;
    }

    return ret;
}

static s32 dispatch_hash_update(void *argp)
{
    s32 ret = HI_FAILURE;
    hash_update_t *update = argp;

    HI_LOG_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == ADDR_VIA(update->input));

    update->src = HASH_CHUNCK_SRC_USER;
    ret = kapi_hash_update(update->id,
                           ADDR_VIA(update->input),
                           update->length,
                           update->src);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_hash_update, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_hash_finish(void *argp)
{
    s32 ret = HI_FAILURE;
    hash_finish_t *finish = argp;

    HI_LOG_FuncEnter();

    ret = kapi_hash_finish(finish->id, (u8*)finish->hash, &finish->hashlen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_hash_finish, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 rsa_alloc_buffer(cryp_rsa_key *key, rsa_info_t *rsa_info,
                            u8 **in, u8 **out)
{
    u32 size = 0, klen = 0;
    s32 ret = HI_FAILURE;
    u8 *buf = HI_NULL;

    HI_LOG_FuncEnter();

    if (HI_FALSE == rsa_info->public)
    {
        HI_LOG_CHECK_PARAM((HI_NULL == ADDR_VIA(rsa_info->d))
            && ((HI_NULL == ADDR_VIA(rsa_info->p))
            || (HI_NULL == ADDR_VIA(rsa_info->q))
            || (HI_NULL == ADDR_VIA(rsa_info->dP))
            || (HI_NULL == ADDR_VIA(rsa_info->dQ))
            || (HI_NULL == ADDR_VIA(rsa_info->qP))));
    }

    HI_LOG_CHECK_PARAM(rsa_info->inlen > rsa_info->klen);
    HI_LOG_CHECK_PARAM(rsa_info->outlen > rsa_info->klen);
    HI_LOG_CHECK_PARAM(RSA_KEY_BITWIDTH_1024 > rsa_info->klen);
    HI_LOG_CHECK_PARAM(RSA_KEY_BITWIDTH_4096 < rsa_info->klen);

    crypto_memset(key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    key->klen = klen = rsa_info->klen;
    key->public = rsa_info->public;
    key->catype = rsa_info->catype;

    if (rsa_info->public)
    {
        HI_LOG_CHECK_PARAM(CRYP_CIPHER_CA_TYPE_USER != key->catype);

        /* buffer size of key, input and output */
        size = rsa_info->klen * RSA_PUBLIC_BUFFER_NUM;

        buf = crypto_calloc(1, size);
        if (HI_NULL == buf)
        {
            HI_LOG_PrintErrCode(HI_ERR_CIPHER_FAILED_MEM);
            HI_LOG_PrintFuncErr(crypto_calloc, ret);
            return HI_ERR_CIPHER_FAILED_MEM;
        }

        key->N = buf; buf += klen;
        *in = buf;    buf += klen;
        *out = buf;   buf += klen;
        key->bufsize = size;

        CHECK_EXIT(crypto_copy_from_user(key->N, ADDR_VIA(rsa_info->N), klen));
        CHECK_EXIT(crypto_copy_from_user(*in, ADDR_VIA(rsa_info->in), klen));
        key->e = rsa_info->e;
    }
    else
    {
        /* n + d or n + p + q + dP + dQ + qp
         * the length of n/d is klen,
         * the length of p/q/dP/dQ/qp is klen/2,
         * the length of input is klen
         * the length of output is klen
         */
        size = klen * RSA_PRIVATE_BUFFER_NUM;

        buf = crypto_calloc(1, size);
        HI_LOG_CHECK_PARAM(HI_NULL == buf);

        key->N  = buf;       buf += klen;
        key->d  = buf;       buf += klen;
        key->p  = buf;       buf += klen / 2;
        key->q  = buf;       buf += klen / 2;
        key->dP = buf;       buf += klen / 2;
        key->dQ = buf;       buf += klen / 2;
        key->qP = buf;       buf += klen / 2;
        key->e  = rsa_info->e;
        key->bufsize = size;

        if (HI_NULL != ADDR_VIA(rsa_info->N))
        {
            CHECK_EXIT(crypto_copy_from_user(key->N, ADDR_VIA(rsa_info->N), klen));
        }

        if (HI_NULL != ADDR_VIA(rsa_info->d))
        {
            CHECK_EXIT(crypto_copy_from_user(key->d, ADDR_VIA(rsa_info->d), klen));
        }
        else
        {
            CHECK_EXIT(crypto_copy_from_user(key->p, ADDR_VIA(rsa_info->p), klen/2));
            CHECK_EXIT(crypto_copy_from_user(key->q, ADDR_VIA(rsa_info->q), klen/2));
            CHECK_EXIT(crypto_copy_from_user(key->dP, ADDR_VIA(rsa_info->dP), klen/2));
            CHECK_EXIT(crypto_copy_from_user(key->dQ, ADDR_VIA(rsa_info->dQ), klen/2));
            CHECK_EXIT(crypto_copy_from_user(key->qP, ADDR_VIA(rsa_info->qP), klen/2));
            key->d = HI_NULL;
        }

        *in  = buf;       buf += klen;
        *out = buf;       buf += klen;

        if (HI_NULL != ADDR_VIA(rsa_info->in))
        {
            CHECK_EXIT(crypto_copy_from_user(*in, ADDR_VIA(rsa_info->in), rsa_info->inlen));
        }
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;

exit__:
    if (HI_NULL != key->N)
    {
        crypto_zeroize(key->N, key->bufsize);
        crypto_free(key->N);
        key->N = HI_NULL;
    }

    HI_LOG_ERROR("error, copy rsa key from user failed\n");
    HI_LOG_PrintErrCode(HI_ERR_CIPHER_FAILED_MEM);

    return HI_ERR_CIPHER_FAILED_MEM;
}

static void rsa_free_buffer(cryp_rsa_key *key)
{
    HI_LOG_FuncEnter();

    if (HI_NULL != key->N)
    {
        crypto_zeroize(key->N, key->bufsize);
        crypto_free(key->N);
        key->N = HI_NULL;
    }

    HI_LOG_FuncExit();
    return;
}

static s32 dispatch_rsa_encrypt(void *argp)
{
    s32 ret = HI_FAILURE;
    rsa_info_t *rsa_info = argp;
    u8 *in = HI_NULL;
    u8 *out = HI_NULL;
    cryp_rsa_key key;

    HI_LOG_FuncEnter();

    crypto_memset(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &in, &out);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("error, rsa_alloc_key failed\n");
        HI_LOG_PrintFuncErr(rsa_alloc_buffer, ret);
        return ret;
    }

    ret = kapi_rsa_encrypt(&key, rsa_info->enScheme, in, rsa_info->inlen,
            out, &rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(kapi_rsa_encrypt, ret);
        return ret;
    }

    ret = crypto_copy_to_user(ADDR_VIA(rsa_info->out), out, rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_free_buffer(&key);

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_rsa_decrypt(void *argp)
{
    s32 ret = HI_FAILURE;
    rsa_info_t *rsa_info = argp;
    u8 *in = HI_NULL;
    u8 *out = HI_NULL;
    cryp_rsa_key key;

    HI_LOG_FuncEnter();

    crypto_memset(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &in, &out);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("error, rsa_alloc_key failed\n");
        HI_LOG_PrintFuncErr(rsa_alloc_buffer, ret);
        return ret;
    }

    ret = kapi_rsa_decrypt(&key, rsa_info->enScheme,
                    in, rsa_info->inlen, out, &rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(kapi_rsa_decrypt, ret);
        return ret;
    }

    ret = crypto_copy_to_user(ADDR_VIA(rsa_info->out), out, rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_free_buffer(&key);

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_rsa_sign_hash(void *argp)
{
    s32 ret = HI_FAILURE;
    rsa_info_t *rsa_info = argp;
    u8 *in= HI_NULL;
    u8 *out = HI_NULL;
    cryp_rsa_key key;

    HI_LOG_FuncEnter();

    crypto_memset(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &in, &out);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("error, rsa alloc key buffer failed\n");
        HI_LOG_PrintFuncErr(rsa_alloc_buffer, ret);
        return ret;
    }

    ret = kapi_rsa_sign_hash(&key, rsa_info->enScheme, in,
            rsa_info->inlen, out, &rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(kapi_rsa_sign_hash, ret);
        return ret;
    }

    ret = crypto_copy_to_user(ADDR_VIA(rsa_info->out), out, rsa_info->outlen);
    if (HI_SUCCESS != ret)
    {
        rsa_free_buffer(&key);
        HI_LOG_PrintFuncErr(crypto_copy_to_user, ret);
        return ret;
    }

    rsa_free_buffer(&key);

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static s32 dispatch_rsa_verify_hash(void *argp)
{
    s32 ret = HI_FAILURE;
    rsa_info_t *rsa_info = argp;
    u8 *in= HI_NULL;
    u8 *out = HI_NULL;
    cryp_rsa_key key;

    HI_LOG_FuncEnter();

    crypto_memset(&key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));

    ret = rsa_alloc_buffer(&key, rsa_info, &in, &out);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("error, rsa_alloc_key failed\n");
        HI_LOG_PrintFuncErr(rsa_alloc_buffer, ret);
        return ret;
    }

    /* copy hash value from user */
    CHECK_EXIT(crypto_copy_from_user(out, ADDR_VIA(rsa_info->out), rsa_info->outlen));
    CHECK_EXIT(kapi_rsa_verify_hash(&key,
                                    rsa_info->enScheme,
                                    out,
                                    rsa_info->outlen,
                                    in,
                                    rsa_info->inlen));
    rsa_free_buffer(&key);
    HI_LOG_FuncExit();
    return HI_SUCCESS;

exit__:
    rsa_free_buffer(&key);

    return ret;
}

static s32 dispatch_trng_get_random(void *argp)
{
    trng_t *trng = argp;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = kapi_trng_get_random(&trng->randnum, trng->timeout);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

static crypto_dispatch_func dispatch_func[CRYPTO_CMD_COUNT] =
{
    {"CreateHandle",  dispatch_symc_create_handle,  CRYPTO_CMD_SYMC_CREATEHANDLE},
    {"DestroyHandle", dispatch_symc_destroy_handle, CRYPTO_CMD_SYMC_DESTROYHANDLE},
    {"ConfigChn",     dispatch_symc_config,         CRYPTO_CMD_SYMC_CONFIGHANDLE},
    {"Encrypt",       dispatch_symc_encrypt,        CRYPTO_CMD_SYMC_ENCRYPT},
    {"EncryptMulti",  dispatch_symc_encrypt_multi,  CRYPTO_CMD_SYMC_ENCRYPTMULTI},
    {"GetTag",        dispatch_symc_get_tag,        CRYPTO_CMD_SYMC_GETTAG},
    {"HashStart",     dispatch_hash_start,          CRYPTO_CMD_HASH_START},
    {"HashUpdate",    dispatch_hash_update,         CRYPTO_CMD_HASH_UPDATE},
    {"HashFinish",    dispatch_hash_finish,         CRYPTO_CMD_HASH_FINISH},
    {"RsaEncrypt",    dispatch_rsa_encrypt,         CRYPTO_CMD_RSA_ENC},
    {"RsaDecrypt",    dispatch_rsa_decrypt,         CRYPTO_CMD_RSA_DEC},
    {"RsaSign",       dispatch_rsa_sign_hash,       CRYPTO_CMD_RSA_SIGN},
    {"RsaVerify",     dispatch_rsa_verify_hash,     CRYPTO_CMD_RSA_VERIFY},
    {"TRNG",          dispatch_trng_get_random,     CRYPTO_CMD_TRNG},
    {"GetSymcConfig", dispatch_symc_get_config,     CRYPTO_CMD_SYMC_GET_CONFIG},
    {"KladKey",       dispatch_klad_key,            CRYPTO_CMD_KLAD_KEY},
};

s32 crypto_ioctl(u32 cmd, HI_VOID *argp)
{
    u32 nr = 0;
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    nr = CRYPTO_IOC_NR (cmd);

    HI_LOG_DEBUG("cmd 0x%x, nr %d, size %d, local cmd 0x%x\n",
        cmd, nr, CRYPTO_IOC_SIZE(cmd), dispatch_func[nr].cmd);

    HI_LOG_CHECK_PARAM(HI_NULL == argp);
    HI_LOG_CHECK_PARAM(CRYPTO_CMD_COUNT <= nr);
    HI_LOG_CHECK_PARAM(cmd != dispatch_func[nr].cmd);

    HI_LOG_INFO("Link Func NR %d, Name:  %s\n", nr, dispatch_func[nr].name);
    ret = dispatch_func[nr].func(argp);
    if (HI_SUCCESS != ret)
    {
        /*TRNG may be empty in FIFO, don't report error, try to read it again */
        if (CRYPTO_CMD_TRNG != cmd)
        {
            HI_LOG_ERROR("error, call dispatch_fun fun failed!\n");
            HI_LOG_PrintFuncErr(crypto_dispatch_func, ret);
        }
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

s32 crypto_entry(void)
{
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    crypto_mem_init();

    ret = module_addr_map();
    if (HI_SUCCESS != ret)
    {
       HI_LOG_ERROR("module addr map failed\n");
       HI_LOG_PrintFuncErr(module_addr_map, ret);
       return ret;
    }

    ret = kapi_symc_init();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("kapi symc init failed\n");
        HI_LOG_PrintFuncErr(kapi_symc_init, ret);
        goto error;
    }

    ret = kapi_hash_init();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("kapi hash init failed\n");
        HI_LOG_PrintFuncErr(kapi_hash_init, ret);
        goto error1;
    }

    ret = kapi_rsa_init();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("kapi rsa init failed\n");
        HI_LOG_PrintFuncErr(kapi_rsa_init, ret);
        goto error2;
    }

    ret = hi_drv_compat_init();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(hi_drv_compat_init, ret);
        goto error3;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;

error3:
    kapi_rsa_deinit();
error2:
    kapi_hash_deinit();
error1:
    kapi_symc_deinit();
error:
    module_addr_unmap();

    return ret;

}

s32 crypto_exit(void)
{
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    ret = kapi_symc_deinit();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_deinit, ret);
        return ret;
    }

    ret = kapi_hash_deinit();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_hash_deinit, ret);
        return ret;
    }

    ret = kapi_rsa_deinit();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_rsa_deinit, ret);
        return ret;
    }

    ret = hi_drv_compat_deinit();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(hi_drv_compat_deinit, ret);
        return ret;
    }

    ret = module_addr_unmap();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(module_addr_unmap, ret);
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}


s32 crypto_release(void)
{
    s32 ret = HI_FAILURE;

    ret = kapi_symc_release();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_symc_release, ret);
        return ret;
    }

    ret = kapi_hash_release();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(kapi_hash_release, ret);
        return ret;
    }

    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== Structure Definition end ====*/

