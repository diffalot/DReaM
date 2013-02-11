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
 * $(@)asym-crypto.c $Revision: 1.2 $ $Date: 2006/09/29 18:19:00 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <assert.h>
#include <string.h>

#include "crypto.h"

#define MY_RSA_E 65537
#define MY_RSA_KEYLEN 1024
#define MY_PADDING RSA_PKCS1_PADDING
#define MY_PADDING_EXCESS 11 /* padding excess + input bytes <= RSA_KEYLEN/8 */
                             /* see man page for RSA_public_encrypt */

int checkError(unsigned char isErr) 
{
    unsigned long errCode = 0;

    if ( isErr ) {
        errCode = ERR_get_error();
        fprintf( stderr, "Error: %s", ERR_error_string(errCode, NULL) ); 
        return 1;   
    } else {
        return 0;
    }
    
}

#define TEST_MEMORY(p) if(p==NULL){fprintf(stderr,"Out of Memory: %s line %d\n",__FILE__,__LINE__);exit(-1);}

#define FP(blah) printf(blah); fflush(stdout);


int rsa_readPublicKey(FILE *fp, RSA **rsa)
{
    RSA *retRsa = (RSA *)PEM_read_RSAPublicKey(fp, rsa, 0, NULL);
    if ( checkError(retRsa == NULL ) )
        return -1;
    return 0;
}

int rsa_writePublicKey(FILE *fp, RSA *rsa)
{
    int retVal = 0;

    retVal = RSA_check_key(rsa);
    if ( checkError( retVal != 1 ) )
        return -1;
    
    retVal = PEM_write_RSAPublicKey(fp, rsa); 
    if ( checkError( retVal == 0 ) )
        return -1;

    return 0;
}

int rsa_readPrivateKey(FILE *fp, RSA **rsa)
{
    RSA * retRsa = (RSA *)PEM_read_RSAPrivateKey(fp, rsa, 0, NULL);
    if ( checkError( retRsa == NULL ) )
        return -1;
    return 0;
}

int rsa_writePrivateKey(FILE *fp, RSA *rsa)
{
    int retVal = 0;

    retVal = RSA_check_key(rsa);
    
    if ( checkError( retVal != 1 ) )
        return -1;
    
    /* writes private keys in unencrypted format
       no cb function and no kstr used. */
    retVal = PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, 0, NULL);
    if ( checkError( retVal == 0 ) )
        return -1;
 
    return 0;   
}

RSA * rsa_generateKeys( int keylen, int e)
{
   /* initalize random number generator */
   srand( time(NULL) );
   RSA * retRsa = RSA_generate_key(keylen, e, NULL, NULL);
   if( checkError(retRsa == NULL) )
       return NULL;
   return retRsa; 
}

int rsa_publicEncrypt( int len, unsigned char * plainText, unsigned char * cipherText,
                  RSA * rsaPub )
{
    int bufsize = RSA_public_encrypt( len, plainText, cipherText, rsaPub,
                                       MY_PADDING  );
    checkError( bufsize == -1 );
    return bufsize;
}

int rsa_privateDecrypt( int bufsize, unsigned char * cipherText, 
                  unsigned char * plainText, RSA *rsaPriv )
{
    int newBufsize = RSA_private_decrypt( bufsize, cipherText, plainText, rsaPriv,
                                          MY_PADDING); 
    checkError( newBufsize == -1 );
    return newBufsize;
}

int asym_genkeys(const char *pubKeyFile, const char *privKeyFile)
{

    FILE * pub_fp = NULL;
    FILE * priv_fp = NULL;
    RSA * rsa = 0;
    rsa = rsa_generateKeys( MY_RSA_KEYLEN, MY_RSA_E );
    
    pub_fp = fopen(pubKeyFile, "w");
    priv_fp = fopen(privKeyFile, "w"); 
    
    rsa_writePublicKey(pub_fp, rsa);
    rsa_writePrivateKey(priv_fp, rsa); 

    fclose(pub_fp);
    fclose(priv_fp);

    return 0;
}

int asym_encrypt(FILE *pubKey_fp, unsigned char *inBuf, int inBufLen,
                         unsigned char *outBuf)
{
    RSA * rsaPub = NULL;
    int outBufLen = 0;
    /* Sanity check */
    if ( inBufLen + MY_PADDING_EXCESS  >= MY_RSA_KEYLEN/8 ) {
        fprintf(stderr, "inBuf can be at most %d",
            (int)(MY_RSA_KEYLEN/8 - MY_PADDING_EXCESS));
        return 0;
    }
        
    if ( rsa_readPublicKey(pubKey_fp, &rsaPub) != 0 ) {
        fprintf(stderr, "asym_encrypt: Cannot read public key");    
        return 0;
    }
    
    outBufLen = rsa_publicEncrypt( inBufLen, (unsigned char*)inBuf, 
                        (unsigned char *)outBuf, rsaPub );

    RSA_free(rsaPub);
    return outBufLen;
    
}

int asym_decrypt(FILE *privKey_fp, unsigned char*inBuf, int inBufLen, unsigned
                 char *outBuf)
{
    RSA * rsaPriv = NULL;
    int outBufLen = 0;
        
    if ( rsa_readPrivateKey(privKey_fp, &rsaPriv) != 0 ) {
        fprintf(stderr, "asym_decrypt: Cannot read private key");    
        return 0;
    }
    
    outBufLen = rsa_privateDecrypt( inBufLen, (unsigned char*)inBuf, 
                        (unsigned char *)outBuf, rsaPriv );

    RSA_free(rsaPriv);
    return outBufLen;
   
}

#if 0
int test1(const char *pubKeyFile, const char *privKeyFile)
{
    FILE * pubFP = NULL;
    FILE * privFP = NULL;
    unsigned char * cipherTxt = NULL;
    char * plainTxt =
    "asdfasdfasdfasdfasdfasdfasdfasdfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    unsigned char * newTxt = NULL;
    int plainLen = 0;
    int len;
    int newLen;
    RSA *rsaPub = 0;
    RSA *rsaPriv = 0;
    
    asym_genkeys( pubKeyFile, privKeyFile );
    
    plainLen = strlen(plainTxt) + 1; 
    printf("\n plain text len %d", plainLen); 
    FP(""); 
    pubFP = fopen(pubKeyFile, "r");
    privFP = fopen(privKeyFile, "r"); 
   
    rsa_readPrivateKey(privFP, &rsaPriv);
    rsa_readPublicKey(pubFP, &rsaPub);     

    cipherTxt = (unsigned char *)malloc( RSA_size(rsaPriv) ); 
    newTxt = (unsigned char *)malloc( RSA_size(rsaPriv) );
    
    len = rsa_publicEncrypt( plainLen, (unsigned char*)plainTxt, 
                        (unsigned char *)cipherTxt, rsaPub );
    
    printf("\nrsa_publicEncrypt returned len: %d", len);
    
    newLen = rsa_privateDecrypt( len, (unsigned char *)cipherTxt, 
                           (unsigned char *)newTxt, rsaPriv ); 
   
    printf("\nPlainText new len %d", newLen); 
    assert( memcmp(newTxt, plainTxt, newLen) == 0 ); 

    RSA_free(rsaPub);
    RSA_free(rsaPriv);
    fclose(pubFP);
    fclose(privFP);
}

int main(int argc, char ** argv)
{
    test1("pub.key", "priv.key");
}
#endif // if 0
