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
 * $(@)crypto.c $Revision: 1.2 $ $Date: 2007/03/09 02:50:05 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>

#include "crypto.h"
const EVP_CIPHER * EVP_aes_128_ctr(void);
/* sys info structure */
typedef struct crypto_sys_s *p_crypto_sys;
typedef unsigned char uint8_t;
typedef struct crypto_sys_s
{
    unsigned char  *key;
    unsigned char  *iv;
    int            key_len;
    int            iv_len;
    EVP_CIPHER_CTX *ctx;
    
} crypto_sys_t;

#define TEST_MEMORY(p) if(p==NULL){fprintf(stderr,"Out of Memory: %s line %d\n",__FILE__,__LINE__);exit(-1);}

void freeCrypto(p_crypto_sys p_sys);

p_crypto_sys initCrypto(unsigned char  *key, int key_len, unsigned char *iv, int iv_len,
	                     int encrypt, AES_MODE mode);

void freeCrypto(p_crypto_sys p_sys)
{
	if (p_sys)
	{
		if (p_sys->key)
		{
			free(p_sys->key);
		}
		if (p_sys->iv)
		{
			free(p_sys->iv);
		};
		if (p_sys->ctx)
		{
			EVP_CIPHER_CTX_cleanup(p_sys->ctx);
		}
		free(p_sys);
	}
	return;
}

static const EVP_CIPHER *getAESCipher(AES_MODE mode)
{
    const EVP_CIPHER *p_cipher = NULL;
    switch (mode)
    {
        case MODE_CFB:
            p_cipher = EVP_aes_128_cfb();
            break;
        case MODE_OFB:
            p_cipher = EVP_aes_128_ofb();
            break;
        case MODE_CTR:
        default:
            p_cipher = EVP_aes_128_ctr();
            break;
    }
    return p_cipher;
}

p_crypto_sys initCrypto(unsigned char  *key, int key_len, unsigned char *iv, int iv_len,
	                int encrypt, AES_MODE mode)
{
	crypto_sys_t *p_sys  = NULL;
	p_sys = (crypto_sys_t *)malloc(sizeof(crypto_sys_t));
	TEST_MEMORY(p_sys);
	memset(p_sys, 0, sizeof(crypto_sys_t));
	
	/* setup data members */
    if (key && key_len > 0)
    {
        p_sys->key = (unsigned char *)malloc(sizeof(unsigned char) * key_len);
        TEST_MEMORY(p_sys->key);
        memcpy(p_sys->key, key, key_len);
        p_sys->key_len = key_len;
    }
    if (iv && iv_len > 0)
    {
        p_sys->iv = (unsigned char *)malloc(sizeof(unsigned char) * iv_len);
        TEST_MEMORY(p_sys->iv);
        memcpy(p_sys->iv, iv, iv_len);
        p_sys->iv_len = iv_len;
    }

    if (p_sys->key && p_sys->iv)
    {
    	/* use OPENSSL library */
    	p_sys->ctx = (EVP_CIPHER_CTX *)malloc(sizeof (EVP_CIPHER_CTX));
    	TEST_MEMORY(p_sys->ctx);
    	EVP_CIPHER_CTX_init (p_sys->ctx);
        const EVP_CIPHER * p_cipher = getAESCipher(mode);
    	if (encrypt)
    	{
    	    EVP_EncryptInit(p_sys->ctx, p_cipher, key, iv);
    	}
    	else
    	{
    	    EVP_DecryptInit(p_sys->ctx, p_cipher, key, iv);
    	}
        return p_sys;
    }
    else
    {
    	freeCrypto(p_sys);
        return NULL;
    }
}

int generate_key (unsigned char* val, int size)
{
	int fd;
	if ((fd = open ("/dev/random", O_RDONLY)) == -1)
		perror ("open error");

	if ((read (fd, val, size)) == -1)
		perror ("read key error");

	close (fd);
	
#if 0	
	printf("%d bit key:\n", size*8);
	for (i = 0; i < size; i++)
		printf ("%d \t", val[i]);
	printf ("\n ------ \n");
#endif

	return 0;
}

static int encryptData (p_crypto_sys p_sys,
	         unsigned char *inbuff, int in_len, 
	         unsigned char *outbuf, int out_len)
{
	int olen = 0;
	int tlen = 0;
	
    if (EVP_EncryptUpdate (p_sys->ctx, outbuf, &olen, inbuff, in_len) != 1)
    {
    	fprintf(stderr, "error in encrypt update\n");
    	return -1;
    }
    else
    {
        EVP_EncryptFinal (p_sys->ctx, outbuf + olen, &tlen);
		olen += tlen;
    	return olen;
    }
}
    
static int decryptData (p_crypto_sys p_sys,
	         unsigned char *inbuff, int in_len, 
	         unsigned char *outbuf, int out_len)
{
	int olen = 0;
	int tlen = 0;
	
    if (EVP_DecryptUpdate (p_sys->ctx, outbuf, &olen, inbuff, in_len) != 1)
    {
    	fprintf(stderr, "error in decrypt update\n");
    	return -1;
    }
    else
    {
        EVP_DecryptFinal (p_sys->ctx, outbuf + olen, &tlen);
	olen += tlen;
    	return olen;
    }
}


int encryptBufferData( uint8_t *p_in, int in_len,
	                    uint8_t *p_out, int out_len,
	                    uint8_t *key, int key_len,
	                    uint8_t *iv, int iv_len, AES_MODE mode)
{
    uint8_t        *inbuff = NULL;
    p_crypto_sys  p_sys_en = NULL;
    int               olen = 0;
   
    inbuff = (uint8_t *)malloc( sizeof(uint8_t) * in_len );
    memcpy( inbuff, p_in, in_len ); 
    
	p_sys_en = initCrypto( key, key_len, iv, iv_len, 1, mode);
	olen     = encryptData( p_sys_en, inbuff, in_len, p_out, out_len );
	freeCrypto(p_sys_en);
	free(inbuff);
	return olen;
}

int decryptBufferData(uint8_t *p_in, int in_len,
	                   uint8_t *p_out, int out_len,
	                    uint8_t *key, int key_len,
	                    uint8_t *iv, int iv_len, AES_MODE mode)
{
    uint8_t        *inbuff = NULL;
    p_crypto_sys  p_sys_de = NULL;
    int               olen = 0;
    
    inbuff = (uint8_t *)malloc( sizeof(uint8_t) * in_len );
    memcpy( inbuff, p_in, in_len ); 
    
	p_sys_de = initCrypto( key, key_len, iv, iv_len, 0, mode );
	olen     = decryptData( p_sys_de, inbuff, in_len, p_out, out_len );
	freeCrypto(p_sys_de);
	free(inbuff);
	return olen;
}




