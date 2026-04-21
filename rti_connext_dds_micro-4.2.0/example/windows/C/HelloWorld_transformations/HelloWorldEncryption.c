/*********************************************************************************************
Copyright (c) 2017-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#include <openssl/aes.h>
#include <openssl/evp.h>

#include "stdlib.h"
#include "string.h"

#include "HelloWorldEncryption.h"

/*ci
 * \brief Size of the key to use in bytes. 256 bits key.
 */
#define KEY_SIZE (256 / 8)

/*ci
 * \brief This is just an example, for an actual application the
 *        key should be ramdonly generated
 */
static unsigned char aes_key[KEY_SIZE/sizeof(unsigned char)] =
                 {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                  0x30, 0x31};

RTI_BOOL HelloWorldEncryption_initalize(EVP_CIPHER_CTX **context)
{
    if (context == NULL)
    {
        return RTI_FALSE;
    }

    *context = EVP_CIPHER_CTX_new();

    return (context != NULL) ? RTI_TRUE : RTI_FALSE;
}

RTI_BOOL HelloWorldEncryption_finalize(EVP_CIPHER_CTX *context)
{
    if (context == NULL)
    {
        return RTI_FALSE;
    }

    EVP_CIPHER_CTX_free(context);

    return RTI_TRUE;
}

/*ci
 * \brief Encrypts/Decrypts the data in the input buffer using EVP_aes_256_cbc
 *
 * \details
 *  This function encrypts or decrypts the data in the input buffer. The length
 *  of the output buffer must be at least ENCRYPTED_LENGTH(input_length)
 *
 * @param[in] ctx Pointer to cypher context
 * @param[in] input Pointer to input buffer to encrypt
 * @param[in] input_length Length in bytes of the input buffer
 * @param[in] output Pointer to output buffer where to store the encrypted data
 * @param[in/out] output_length As input contains the maximum length of the
 *                              output buffer. As output contains the length of
 *                              the encrypted data
 * @param[in] encrypt RTI_TRUE if the function should encrypt or RTI_FALSE to
 *                    decrypt
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
HelloWorldEncryption_evp_update(EVP_CIPHER_CTX *ctx,
                                unsigned char *input,
                                int input_length,
                                unsigned char *output,
                                int *output_length,
                                RTI_BOOL encrypt)
{
    unsigned char iv[AES_BLOCK_SIZE];
    int outLen2;

    if ((size_t)(*output_length) < ENCRYPTED_LENGTH(input_length))
    {
        return RTI_FALSE;
    }

    memset(iv, 0x00, AES_BLOCK_SIZE);

    if (encrypt)
    {
        EVP_EncryptInit(ctx, EVP_aes_256_cbc(), aes_key, iv);

        EVP_EncryptUpdate(ctx, output, output_length, input, input_length);

        EVP_EncryptFinal(ctx, output + (*output_length) , &outLen2);
    }
    else
    {
        EVP_DecryptInit(ctx, EVP_aes_256_cbc(), aes_key, iv);

        EVP_DecryptUpdate(ctx, output, output_length, input, input_length);

        EVP_DecryptFinal(ctx, output + (*output_length) , &outLen2);
    }

    *output_length = *output_length + outLen2;

    return RTI_TRUE;
}

RTI_BOOL
HelloWorldEncryption_encrypt(EVP_CIPHER_CTX *ctx,
                             unsigned char *input,
                             int input_length,
                             unsigned char *output,
                             int *output_length)
{
    return HelloWorldEncryption_evp_update(ctx, input, input_length,
                                           output, output_length, RTI_TRUE);
}

RTI_BOOL
HelloWorldEncryption_decrypt(EVP_CIPHER_CTX *ctx,
                             unsigned char *input,
                             int input_length,
                             unsigned char *output,
                             int *output_length)
{
    return HelloWorldEncryption_evp_update(ctx, input, input_length,
                                           output, output_length, RTI_FALSE);
}
