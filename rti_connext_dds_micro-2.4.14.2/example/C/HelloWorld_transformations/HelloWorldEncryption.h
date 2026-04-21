/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#ifndef HelloWorldEncryption_h
#define HelloWorldEncryption_h

#include <openssl/evp.h>

#include "rti_me_c.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*ci
 * \brief Calculates the length that a buffer of certain length will have
 *        when it is encrypted or decrypted
 */
#define ENCRYPTED_LENGTH(x)    ((size_t)(((size_t)(x) + AES_BLOCK_SIZE) / \
                                              AES_BLOCK_SIZE) * AES_BLOCK_SIZE)

/*ci
 * \brief Initializes the encryption library
 *
 * @param[out] ctx Pointer to cypher context
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL HelloWorldEncryption_initalize(EVP_CIPHER_CTX **context);

/*ci
 * \brief Finalizes the encryption library
 *
 * @param[out] ctx Pointer to cypher context. This context is released
 *                 in this function and cannot be used afterwards.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL HelloWorldEncryption_finalize(EVP_CIPHER_CTX *context);

/*ci
 * \brief Encrypts the data in the input buffer using EVP_aes_256_cbc
 *
 * \details
 *  This function encrypts the data in the input buffer. The length of
 * the output buffer must be at least ENCRYPTED_LENGTH(input_length)
 *
 * @param[in] ctx Pointer to cypher context
 * @param[in] input Pointer to input buffer to encrypt
 * @param[in] input_length Length in bytes of the input buffer
 * @param[in] output Pointer to output buffer where to store the encrypted data
 * @param[in/out] output_length As input contains the maximum length of the 
 *                              output buffer. As output contains the length of
 *                              the encrypted data
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL 
HelloWorldEncryption_encrypt(EVP_CIPHER_CTX *ctx,
                             unsigned char *input, int input_length, 
                             unsigned char *output, int *output_length);

/*ci
 * \brief Decrypts the data in the input buffer using EVP_aes_256_cbc
 *
 * \details
 *  This function decrypts the data in the input buffer. The length of
 * the output buffer must be at least ENCRYPTED_LENGTH(input_length)
 *
 * @param[in] ctx Pointer to cypher context
 * @param[in] input Pointer to input buffer to decrypt
 * @param[in] input_length Length in bytes of the input buffer
 * @param[in] output Pointer to output buffer where to store the decrypted data
 * @param[in/out] output_length As input contains the maximum length of the 
 *                              output buffer. As ouput contains the length of
 *                              the decrypted data
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL 
HelloWorldEncryption_decrypt(EVP_CIPHER_CTX *ctx,
                             unsigned char *input, int input_length, 
                             unsigned char *output, int *output_length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HelloWorldEncryption_h */

