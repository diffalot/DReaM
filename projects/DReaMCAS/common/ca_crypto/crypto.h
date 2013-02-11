/*
 * The contents of this file are subject to the terms
 * of the Common Development and Distribution License
 * (the "License").  You may not use this file except
 * in compliance with the License.
 *
 * You can obtain a copy of the license at
 * http://www.opensource.org/licenses/cddl1.php
 * See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL
 * HEADER in each file and include the License file at
 * http://www.opensource.org/licenses/cddl1.php.  If 
 * applicable, add the following below this CDDL HEADER, 
 * with the fields enclosed by brackets "[]" replaced 
 * with your own identifying information: 
 * Portions Copyright [yyyy]
 * [name of copyright owner]
 */ 

/*
 * $(@)crypto.h $Revision: 1.2 $ $Date: 2006/03/22 20:00:49 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef _CA_CRYPTO_H
#define _CA_CRYPTO_H
#ifdef __cplusplus
extern "C" {
#endif
typedef enum 
{
    MODE_CFB,
    MODE_OFB,
    MODE_CTR    
}AES_MODE;

int generate_key (unsigned char* val, int size);

int decryptBufferData( unsigned char *p_in, int in_len,
	                   unsigned char *p_out, int out_len,
	                   unsigned char *key, int key_len,
	                   unsigned char *iv, int iv_len, AES_MODE mode);

int encryptBufferData( unsigned char *p_in, int in_len,
	                   unsigned char *p_out, int out_len,
	                   unsigned char *key, int key_len,
	                   unsigned char *iv, int iv_len, AES_MODE mode);

int asym_encrypt(FILE *pubKey_fp, unsigned char *inBuf, int inBufLen,
                         unsigned char *outBuf);

int asym_decrypt(FILE *privKey_fp, unsigned char*inBuf, int inBufLen, unsigned
                 char *outBuf);

int asym_genkeys(const char *pubKeyFile, const char *privKeyFile);

/* 
 * openssh defines EVP aes functions as EVP_aes** while openssl defines it as
 * evp_aes**. This define is simply to make sure that we use a consistent
 * naming scheme for evp aes functions. The evp_aes_128_ctr adds counter mode
 * support which is required for the crypto library
 */
#define EVP_aes_128_ctr(a) evp_aes_128_ctr(a)

#ifdef __cplusplus
}
#endif

#endif



