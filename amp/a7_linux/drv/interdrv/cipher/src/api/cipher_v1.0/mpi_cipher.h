/*****************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : mpi_cipher.h
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#ifndef __MPI_CIPHER_H__
#define __MPI_CIPHER_H__

#include "user_osal_lib.h"

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/******************************* API Declaration *****************************/
/** \addtogroup      mpi */
/** @{ */  /** <!--[mpi]*/

/** @}*/  /** <!-- ==== Structure Definition end ====*/

/******************************* API Code *****************************/
/** \addtogroup      crypto*/
/** @{*/  /** <!-- [link]*/

/**
\brief   mpi Init.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_init(void);

/**
\brief   Kapi Deinit.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_deinit(void);

/**
\brief   Create symc handle.
\param[in]  id The channel number.
\param[in]  uuid The user identification.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_create(u32 *id);

/**
\brief   Destroy symc handle.
\param[in]  id The channel number.
\param[in]  uuid The user identification.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_destroy(u32 id);

/**
\brief  set work params.
* \param[in]  id            The channel number.
* \param[in]  hard_key      whether use the hard key or not.
* \param[in]  enAlg         The symmetric cipher algorithm.
* \param[in]  enWorkMode    The symmetric cipher mode.
* \param[in]  enBitWidth    The symmetric cipher bit width.
* \param[in]  enKeyLen      The symmetric cipher key len.
* \param[in]  sm1_round_num The round number of sm1.
* \param[in]  fkey          first key buffer, defualt
* \param[in]  skey          second key buffer, expand
* \param[in]  keylen        The length of fkey buffer,if skey is not null,equal to the length of skey.
* \param[in]  iv            iv buffer.
* \param[in]  ivlen         The length of iv buffer.
* \param[in]  iv_usage      iv change.
* \param[in]  aad           Associated Data
* \param[in]  alen          Associated Data Length
* \param[in]  tlen          Tag length
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_config(u32 id,
                    u32 hard_key,
                    CRYP_CIPHER_ALG_E enAlg,
                    CRYP_CIPHER_WORK_MODE_E enWorkMode,
                    CRYP_CIPHER_BIT_WIDTH_E enBitWidth,
                    CRYP_CIPHER_KEY_LENGTH_E enKeyLen,
                    CRYP_CIPHER_SM1_ROUND_E sm1_round_num,
                    u8 *fkey, u8 *skey, u32 keylen,
                    u8 *iv, u32 ivlen, u32 iv_usage,
                    compat_addr aad, u32 alen, u32 tlen);


/**
\brief  get work params.
* \param[in]  id The channel number.
* \param[out] ctrl infomation.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_symc_get_config(u32 id, CRYP_CIPHER_CTRL_S *ctrl);

/**
 * \brief          SYMC  buffer encryption/decryption.
 *
 * Note: Due to the nature of aes you should use the same key schedule for
 * both encryption and decryption.
 *
 * \param[in]  id The channel number.
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 * \param length   length of the input data
 * \param decrypt  decrypt or encrypt
 *
 * \return         0 if successful
 */
s32 mpi_symc_crypto(u32 id, const compat_addr input,
                      const compat_addr output, u32 length,
                      u32 operation);

/**
 * \brief          SYMC multiple buffer encryption/decryption.
 *
 * Note: Due to the nature of aes you should use the same key schedule for
 * both encryption and decryption.
 *
 * \param[in]  id The channel number.
 * \param pkg       Buffer of package infomation
 * \param pkg_num   Number of package infomation
 * \param decrypt  decrypt or encrypt
 *
 * \return         0 if successful
 */
s32 mpi_symc_crypto_multi(u32 id, const void *pkg,
                           u32 pkg_num, u32 operation);

/**
 * \brief          SYMC multiple buffer encryption/decryption.
 * \param[in]  id The channel number.
 * \param[in]  tag tag data of CCM/GCM
 * \param uuid uuid The user identification.
 *
 * \return         0 if successful
 */
s32 mpi_aead_get_tag(u32 id, u8 *tag, u32 *taglen);

/**
 * \brief      Klad encrypt clear key.
 * \keysel[in] keysel root key seclect.
 * \param[in]  target target seclect
 * \param[in]  clear clear key
 * \param[out]  encrypt encrypt key
 * \param uuid uuid The user identification.
 *
 * \return         0 if successful
 */
s32 mpi_klad_encrypt_key(u32 keysel, u32 target, const u8 clear[AES_BLOCK_SIZE], u8 encrypt[AES_BLOCK_SIZE]);

/**
\brief   Kapi Init.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_hash_init(void);

/**
\brief   Kapi Deinit.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_hash_deinit(void);

/**
 * \brief          HASH context setup.
 *
 *
 * \param[out] id The channel number.
 * \param[in] type    Hash type
 * \param[in] key     hmac key
 * \param[in] keylen  hmac key length
 *
 * \return         0 if successful
 */
s32 mpi_hash_start(u32 *id, CRYP_CIPHER_HASH_TYPE_E type,
                    const u8 *key, u32 keylen);

/**
 * \brief          HASH process buffer.
 *
 * \param[in]  id The channel number.
 * \param[in] input    buffer holding the input data
 * \param[in] length   length of the input data
 * \param[in] src      source of hash message
 *
 * \return         0 if successful
 */
s32 mpi_hash_update(u32 id, const u8 *input, u32 length,
                     hash_chunk_src src);

/**
 * \brief          HASH final digest.
 *
 * \param[in]  id The channel number.
 * \param[out] hash    buffer holding the hash data
 * \param[out] hashlen length of the hash data
 * \param[in] uuid uuid The user identification.
 *
 * \return         0 if successful
 */
s32 mpi_hash_finish(u32 id, u8 *hash, u32 *hashlen);

/**
\brief   Kapi Init.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_rsa_init(void);

/**
\brief   Kapi Deinitialize.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_rsa_deinit(void);

/**
* \brief RSA encryption a plaintext with a RSA private key.
*
* \param[in] key:       rsa key struct.
* \param[in] enScheme:  rsa encrypt/decrypt scheme.
* \param[in] in:      input data to be encryption
* \param[out] inlen:  length of input data to be encryption
* \param[out] out:    output data of encryption
* \param[out] outlen: length of output data to be encryption
* \retval HI_SUCCESS  Call this API successful
* \retval HI_FAILURE  Call this API fails.
*/
s32 mpi_rsa_encrypt(const cryp_rsa_key *key,
                    CRYP_CIPHER_RSA_ENC_SCHEME_E enScheme,
                    const u8 *in, u32 inlen, u8 *out, u32 *outlen);

/**
* \brief RSA decryption a ciphertext with a RSA public key.
*
* \param[in] key:       rsa key struct.
* \param[in] enScheme:  rsa encrypt/decrypt scheme.
* \param[in] in:     input data to be encryption
* \param[in] inlen:  length of input data to be encryption
* \param[out] out:   output data to be encryption
* \param[out] outlen: length of output data to be encryption
* \retval HI_SUCCESS  Call this API successful
* \retval HI_FAILURE  Call this API fails.
*/
s32 mpi_rsa_decrypt(const cryp_rsa_key *key,
                    CRYP_CIPHER_RSA_ENC_SCHEME_E enScheme,
                    const u8 *in, u32 inlen, u8 *out, u32 *outlen);

/**
* \brief RSA signature a context with appendix, where a signer's RSA private key is used.
*
* \param[in] key:       rsa key struct.
* \param[in] enScheme:  rsa signature/verify scheme.
* \param[in] in:     input data to be encryption
* \param[in] inlen:  length of input data to be encryption
* \param[in] hash:   hash value of context,if NULL, let hash = Hash(context) automatically
* \param[out] out:   output data to be encryption
* \param[out] outlen: length of output data to be encryption
* \param[in]  src      source of hash message
* \param[in]  uuid uuid The user identification.
* \retval HI_SUCCESS  Call this API successful
* \retval HI_FAILURE  Call this API fails.
*/
s32 mpi_rsa_sign_hash(const cryp_rsa_key *key,
                      CRYP_CIPHER_RSA_SIGN_SCHEME_E enScheme,
                      const u8* hash, u32 hlen, u8 *sign, u32 *signlen);

/**
* \brief RSA verify a ciphertext with a RSA public key.
*
* \param[in]  key_info:   encryption struct.
* \param[in]  in:     input data to be encryption
* \param[in]  inlen:  length of input data to be encryption
* \param[in]  hash:   hash value of context,if NULL, let hash = Hash(context) automatically
* \param[out] out:    output data to be encryption
* \param[out] outlen: length of output data to be encryption
* \param[in]  src      source of hash message
* \param[in]  uuid uuid The user identification.
* \retval HI_SUCCESS  Call this API successful
* \retval HI_FAILURE  Call this API fails.
*/
s32 mpi_rsa_verify_hash(const cryp_rsa_key *key,
                        CRYP_CIPHER_RSA_SIGN_SCHEME_E enScheme,
                        const u8 *hash, u32 hlen, const u8 *sign, u32 signlen);

/**
\brief get rand number.
\param[out]  randnum rand number.
\param[in]   timeout time out.
\retval     On success, HI_SUCCESS is returned.  On error, HI_FAILURE is returned.
*/
s32 mpi_trng_get_random( u32 *randnum, u32 timeout);

/** @}*/  /** <!-- ==== API Code end ====*/

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif    /* End of #ifndef __MPI_CIPHER_H__*/
