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
 * $(@)main.c $Revision: 1.3 $ $Date: 2006/09/29 18:17:39 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform TS parsing

 * Created:   July 29, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/

#include <stdio.h>

//#include "trickplay/trickplay.h"
#include "cas/cas_lib.h"
#include "ca_crypto/crypto.h"
#define HAVE_ENCRYPTION_LIB
//#define _DEBUG

#include "ecmfile.h"
#include <libgen.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "cobjs/Macros.h"
#include <sys/mman.h>

#include "cobjs/ArrayOf.h"
#include "cobjs/HashTable.h"
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"
#include "cobjs/libcobjsVersion.h"

#include "mmp/Mmp.h"
#include "mmp/Mpeg1Pes.h"
#include "mmp/Mpeg1Sys.h"
#include "mmp/Mpeg2.h"
#include "mmp/Mpeg2Pat.h"
#include "mmp/Mpeg2Pes.h"
#include "mmp/Mpeg2Pmt.h"
#include "mmp/Mpeg2Ps.h"
#include "mmp/Mpeg2Psi.h"
#include "mmp/Mpeg2Ts.h"
#include "mmp/Pipe.h"
#include "mmp/libmmpVersion.h"
#include "mmp/Mpeg2VideoEs.h"
#include "mmp/Mpeg1AudioEs.h"

#define CA_SYS_ID  0x222
#define FREQUENCY 5
/* AES key size 128 bits */
#define KEY_LENGTH 16 
/* AES use block size 16bytes */
#define BLOCK_SIZE 16


typedef struct 
{
    uint8_t  key_len;
    uint8_t  iv_len;
    uint8_t  key[KEY_LENGTH];
    uint8_t  iv[BLOCK_SIZE];
} cas_ecm_payload, *cas_ecm_payload_p;

typedef struct 
{
    uint8_t  aes_mode;
    uint8_t  key_len;
    uint8_t  iv_len;
    uint8_t  key[KEY_LENGTH];
    uint8_t  iv[BLOCK_SIZE];
} cas_emm_payload, *cas_emm_payload_p;


typedef struct 
{
	cas_ecm_payload  *p_ecm;
	cas_emm_payload  *p_emm;
	/* input ecm file name,only used for generating encrypted Trickplay stream */
	char in_ecm_file_name[256]; 
	/* output ecm recording the ECM offset in the encrypted file 
	   used when generating encrypted Trickplay stream */ 
	char out_ecm_file_name[256];
	char emm_file_name[256];
	int  ecm_no;
	int  ecm_no_inserting;
    ca_sys_p p_ecm_sys;
	short ecm_id;
	int b_reverse;

} sys_info_s, *sys_info_p;

#define	MAP_CHUNK	(1024*1024)
static void  OutputCallBack (void *token, uint8_t *buf, int len);

static void  ECMCallBack (void *token, uint8_t *buf, int len, uint8_t *out_buf, int *out_len, long long  position);
static void  ECMIOnlyCallBack (void *token, uint8_t *buf, int len, uint8_t *out_buf, int *out_len, long long  position);

static void  ScrambleCallBack (void *token, uint8_t *key_buf, int key_len, 
	                       uint8_t *pkt, uint8_t *payload_buf, int payload_len,
	                       long long position);

static void  EMMOutputCallBack (void *token, uint8_t *ts_buf, int ts_len);
static void EMMAsymEncrypt(void *token, uint8_t *emm_buf, int emm_len, 
                        uint8_t *out_buf, int *out_len, long long  position);
static void TSCallBack(void *token, long long position);

/* put the file dat into a MMP object */
static long long file2mmp(const char *file, ca_sys_p p_sys)
{
    long                pagesize = sysconf(_SC_PAGESIZE);
    off64_t             offset = 0;
    long                mapChunk;
    int                 fd;
    struct stat64       st;
    caddr_t             buf;
    long long           retVal;
    int		retCode;

    if ((fd = open64(file, O_RDONLY)) <= 0 || fstat64(fd, &st) < 0) {
	perror(file);
	retVal = -1;
	goto done;
    }
    retVal = st.st_size;
    (void) printf("File size:     %lld\n", st.st_size);

    mapChunk = ROUND_UP(MAP_CHUNK, pagesize);
    while (offset < st.st_size) {
	buf = mmap64((caddr_t) 0, mapChunk, PROT_READ, MAP_PRIVATE, fd, offset);
	if (buf == (caddr_t) - 1) {
	    perror("mmap");
	    retVal = -1;
	    goto done;
	}
	if (madvise(buf, mapChunk, MADV_SEQUENTIAL) < 0) {
	    perror("madvise");
	    retVal = -1;
	    goto done;
	}
	retCode = casPutData(p_sys, (uint8_t *)buf,
		(size_t) MIN(mapChunk, st.st_size - offset));
	if (munmap(buf, mapChunk) < 0) {
	    perror("munmap");
	    retVal = -1;
	    goto done;
	}
	if (retCode != 0) {
	    break;
	}
	offset += mapChunk;
    }
done:
    if (fd >= 0) {
	(void) close(fd);
    }
    return retVal;
}

static void  EMMOutputCallBack (void *token, uint8_t *ts_buf, int ts_len)
{
    /* generate the EMM key file */
    FILE *emmFile = NULL;
    char *filename = (char *)token;
    emmFile = fopen(filename,"wb");
    fwrite(ts_buf, 1, ts_len, emmFile);
    fclose(emmFile);
    return;
}


static void EMMWritePayloadCallBack(void *token, uint8_t *emm_buf, int emm_len, 
                            uint8_t *out_buf, int *out_len, long long  position)
{
    FILE *emmFile = NULL;
    char *filename = (char *)token;
    emmFile = fopen(filename,"wb");
    fwrite(emm_buf, 1, emm_len, emmFile);
    fclose(emmFile);

    /* 
     * Hacky!
     * function expects something in the out_buf hence copy contents into
     * out_buf 
     */
    memcpy(out_buf, emm_buf, emm_len);
    *out_len = emm_len;
    return;
 
}
/* generate the EMM data file */

static void createEMM(cas_emm_payload *p_emm, uint8_t aes_mode, char *filename)
{
    ca_sys_p            p_sys, p_emm_sys;
    int i = 0;
#ifndef HAVE_ENCRYPTION_LIB
    unsigned char access_key[KEY_LENGTH+1]  = "2222222222222222";
    unsigned char access_iv[BLOCK_SIZE+1]   = "0123456789012345";
    memcpy(p_emm->iv, access_iv, BLOCK_SIZE);
    memcpy(p_emm->key, access_key, KEY_LENGTH);
#else
    generate_key(p_emm->iv, BLOCK_SIZE);
    generate_key(p_emm->key, KEY_LENGTH);
#endif

    p_emm->key_len = KEY_LENGTH ;
    p_emm->iv_len  = BLOCK_SIZE;
    p_emm->aes_mode = aes_mode;
    
#ifdef _DEBUG
    printf ("key: ");
    for (i = 0; i<KEY_LENGTH; i++)
    {
        printf("%#x ", p_emm->key[i]);
    }
    printf("\n");
    printf ("iv : ");
    for (i = 0; i<BLOCK_SIZE; i++)
    {
        printf("%#x ", p_emm->iv[i]);
    }
    printf("\n");
#endif
    
    p_emm_sys = createCAS(CA_SYS_ID,FREQUENCY);
    /* we dont want to write the whole TS packet. Just the payload. */
    // casSetOutputCallback(p_emm_sys, EMMOutputCallBack, filename);

    casSetEMSCallBack(p_emm_sys, EMMWritePayloadCallBack, filename);
    generateCAMessage(p_emm_sys, 0, 600, (uint8_t *)p_emm, sizeof(*p_emm));
    freeCAS(p_emm_sys);
    return;
	
}

void GetEncryptedBitstream(ca_sys_p  p_sys, sys_info_s *p_sys_info, char *infilename, FILE *outfile, int ecm_after_pmt,
	      casScrambleCallBack f_scramble,
	      casEMSCallBack      f_ems
	      )
{
    long long           fileSize;
    int             retCode;
    
    casECMafterPMT(p_sys, ecm_after_pmt);
    
    casSetOutputCallback(p_sys, OutputCallBack, outfile);
    casSetEMSCallBack(p_sys, f_ems, p_sys_info);
    casSetScrambleCallBack(p_sys, f_scramble, p_sys_info);
    
    addKey2CAS(p_sys, FOR_ANY_PROGRAM, ENCRYPT_ALL, 
    	       (uint8_t *)p_sys_info->p_ecm, sizeof(cas_ecm_payload));
//    addKey2CAS(p_sys, FOR_ANY_PROGRAM, ENCRYPT_VIDEO_ONLY, (uint8_t *)&ecm, sizeof( cas_ecm_payload));
    
    /* read file, parse it, and then generate the new file */
    printf("Start ECM insertion...\n");
    fileSize = file2mmp(infilename, p_sys);
    retCode = casEOF(p_sys);
    printf("Done!\n");
    return;
}

static void  OutputCallBack (void *token, uint8_t *ts_buf, int ts_len)
{
	FILE *outfile = (FILE *)token;
	fwrite(ts_buf, 1, ts_len, outfile);
	return;
}

/* position -> the postion of ECM in the newfile */
/* key_buf: the original payload added by addKey2CAS */
/* out_buf is the buffer being inserted into the bitstream */
/* this function is called each time a ecm is generated */
static void  ECMCallBack (void *token, uint8_t *key_buf, int key_len, uint8_t *out_buf, int *out_len, long long  position)
{
    sys_info_s *p_sys_info = (sys_info_s *)token;
    FILE *ecm_file = NULL;
    cas_ecm_payload_p p_ecm = p_sys_info->p_ecm;
    cas_emm_payload_p p_emm = p_sys_info->p_emm;
    
    int encrypted_data_size = 0;
    uint8_t *p_ecm_buf = (uint8_t *)(p_ecm);
    
#ifndef HAVE_ENCRYPTION_LIB
    unsigned char access_key[KEY_LENGTH+1]  = "2222222222222222";
    unsigned char access_iv[BLOCK_SIZE+1]   = "0123456789012345";
    memcpy(p_ecm->iv, access_iv, BLOCK_SIZE);
    memcpy(p_ecm->key, access_key, KEY_LENGTH);
#else
    generate_key(p_ecm->iv, BLOCK_SIZE);
    generate_key(p_ecm->key, KEY_LENGTH);
#endif

    /* when everytime inserting the ECM, the key and IV are changed */
    p_ecm->key_len = KEY_LENGTH ;
    p_ecm->iv_len  = BLOCK_SIZE;
    


    /* save the ECM payload for later used in encrypting the trickplay stream */
    ecm_file = fopen(p_sys_info->out_ecm_file_name, "r+b");
    if(ecm_file)
    {
        ECMfile_info *info = GetECMFileInfo(ecm_file);
        /* save the ECM, not yet encrypted by EMM! */
        WriteOneECMItem(ecm_file, info, p_sys_info->ecm_no,
                        p_ecm_buf, sizeof(*p_ecm), position);
    	p_sys_info->ecm_no ++;
        fclose(ecm_file);
        CleanECMInfo(info);
    }

#ifdef HAVE_ENCRYPTION_LIB
    encrypted_data_size =  encryptBufferData(p_ecm_buf, sizeof(cas_ecm_payload), out_buf, *out_len, 
    	              p_emm->key, p_emm->key_len, p_emm->iv, p_emm->iv_len, (AES_MODE)p_emm->aes_mode);
    *out_len = encrypted_data_size;
#else
    memcpy(out_buf, p_ecm_buf, *out_len);
    *out_len = sizeof(cas_ecm_payload);
    
#endif

    return;
}

/* position -> the postion of ECM in the newfile */
/* key_buf: the original payload added by addKey2CAS */
/* out_buf is the buffer being inserted into the bitstream */
/* this function is called each time a ecm is generated */
static void  ECMIOnlyCallBack (void *token, uint8_t *key_buf, int key_len, uint8_t *out_buf, int *out_len, long long  position)
{
    sys_info_s *p_sys_info = (sys_info_s *)token;
    FILE *ecm_file = NULL;
    cas_ecm_payload_p p_ecm = p_sys_info->p_ecm;
    cas_emm_payload_p p_emm = p_sys_info->p_emm;
    
    int encrypted_data_size = 0;

    memcpy(p_ecm, key_buf, key_len);

#ifdef _DEBUG
    ecm_file = fopen(p_sys_info->out_ecm_file_name, "r+b");
    if(ecm_file)
    {
        ECMfile_info *info = GetECMFileInfo(ecm_file);
        /* save the ECM, not yet encrypted by EMM! */
        WriteOneECMItem(ecm_file, info, p_sys_info->ecm_no,
                        key_buf, key_len, position);
    	p_sys_info->ecm_no ++;
        fclose(ecm_file);
        CleanECMInfo(info);
    }
#endif

#ifdef HAVE_ENCRYPTION_LIB
    encrypted_data_size =  encryptBufferData(key_buf, key_len, out_buf, *out_len, 
    	              p_emm->key, p_emm->key_len, p_emm->iv, p_emm->iv_len, (AES_MODE)p_emm->aes_mode);
    *out_len = encrypted_data_size;
#endif
    return;
}

static void counterAddUInt32( uint8_t *op1, uint32_t op_size, uint32_t op2)
{
    uint32_t len;
    uint8_t  op2_8; 
    
    len = op_size - 1;
    op2_8 = op2 & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 2;
    op2_8 = (op2 >> 8 )& 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 3;
    op2_8 = (op2 >> 16 ) & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 4;
    op2_8 = (op2 >> 24 ) & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }
    
    return;  	
}

static void counterInc( uint8_t *op1, uint32_t op_size)
{
    uint32_t len;
    uint8_t  op2_8; 
    
    len = op_size - 1;
    while ( len >= 0 )
    {
    	if ( ++op1[len] )
    		break;
    	len--;
    }
   
    return;  	
}

static void TSCallBack(void *token, long long position)
{
    sys_info_s *p_sys_info = (sys_info_s *)token;
    cas_ecm_payload_p p_ecm = p_sys_info->p_ecm;
    FILE *ecm_file = NULL;
    
    /* inserting the ECM at proper positions */
    ecm_file = fopen(p_sys_info->in_ecm_file_name, "rb");
    if (ecm_file == NULL)
    {
        printf("Error: could not open ECM file, no ECM inserted!\n");
    	return;
    }
    else 
    {
        ECMfile_info *info;
        int ecm_index = 0;
        info = GetECMFileInfo(ecm_file);
        ecm_index = p_sys_info->b_reverse ? info->header.no_of_ECMs-1-p_sys_info->ecm_no_inserting : p_sys_info->ecm_no_inserting;
        
        ReWrOneECMItem(ecm_file, info, ecm_index, TRUE);
    	while (info->ecmItem.insert_offset == -1 
               && p_sys_info->ecm_no_inserting < info->header.no_of_ECMs)
        {
            p_sys_info->ecm_no_inserting ++;
            ecm_index = p_sys_info->b_reverse ? info->header.no_of_ECMs-1-p_sys_info->ecm_no_inserting : p_sys_info->ecm_no_inserting;
            ReWrOneECMItem(ecm_file, info, ecm_index, TRUE);
        }
        
    	if (info->ecmItem.insert_offset == position)
    	{
            memcpy(p_ecm, (uint8_t *)info->ecmItem.ecm_payload, info->header.ecm_len);
            generateCAMessage(p_sys_info->p_ecm_sys, TRUE, p_sys_info->ecm_id, 
    			(uint8_t *)p_ecm, info->header.ecm_len);
    	    p_sys_info->ecm_no_inserting ++;
    	}
    	
    	CleanECMInfo(info);
    }
    fclose(ecm_file);
    return;

}


/* key_buf: the original payload added by addKey2CAS */
static void  ScrambleCallBack (void *token, uint8_t *key_buf, int key_len, 
	                       uint8_t *pkt, uint8_t *payload_buf, int payload_len,
	                       long long  position)
{
    sys_info_s *p_sys_info = (sys_info_s *)token;
    cas_ecm_payload_p p_ecm = p_sys_info->p_ecm;
    cas_emm_payload_p p_emm = p_sys_info->p_emm;

    unsigned char stream_iv[BLOCK_SIZE] ;
    static int pre_continuity_counter = 0;
    int continuity_counter = pkt[3] & 0x0f;
    
    uint8_t *counter = (uint8_t *)malloc(p_ecm->iv_len);
    memcpy(counter, p_ecm->iv, p_ecm->iv_len);
    
    counterAddUInt32( counter, p_ecm->iv_len, continuity_counter * 12 ); /* 184*8/128 */
    if ( continuity_counter != pre_continuity_counter + 1 )
    {
       /* recompute the counter	 */
    }
    else
    {
    	/*counter ++; */
    	
    }
    if (payload_len > 0 && p_ecm->key_len && p_ecm->iv_len)
    {
#ifdef HAVE_ENCRYPTION_LIB
        encryptBufferData(payload_buf, payload_len, payload_buf, payload_len,
    	              p_ecm->key, p_ecm->key_len, counter, p_ecm->iv_len, (AES_MODE)p_emm->aes_mode);
#endif
    }
    
    pre_continuity_counter =  continuity_counter;
    free(counter);
    return;
}

typedef enum InputOption 
{
    CREATE_1X,
    CREATE_TRICKPLAY
} InputOption;


/*Prunes the file extension(characters after the last . inclusive and returns the
rest of the string*/

static char * getResourceName(char *name){
    const char *tmpstr = strrchr(name,'.');
    if(tmpstr == NULL || (name-tmpstr == 0)){
        return name;
    }
    char *cp = (char *)malloc(tmpstr-name);
    cp = strncpy(cp,name,tmpstr-name);
    return cp;
}
static  const char * Basename(const char *name)
{
    const char *cp = strrchr(name, '/');
    return cp != NULL ? cp + 1 : name;
}

/* we don't know what kind of the normal MPEG2 will be.
For ECM inserting, the PMT will be rebuilt.  After Injecting CA descriptos,
the size of the new PMT, in terms of number of TS packets, may change. Therefore,
It is hard to tell the inserting positions of ECMs  with respect to the old TS, 
at least for the current software infrastruture we have. 

Option1:
To generate an accurate meta data file, a ECM insertion is performed first,
but without encryption, then the output is passed to the generate_IOnly_stream program.
while creating the ES, we can get an accurate meta data file, indication the offset of
each frame for the new bitstream that has ECMs and new PMTs. 
Then we do the acutall encryption.

Option2
we do know the exact offset of each ECM in the new file, if we assume the PMT is not changed
after injecting CA descriptor, we can write a program to update the meta data file.
step 1: Create the ES bitstream from the old bitstream 
        --> get meta data file A
step 2: Do the ECM inserting and encryption for the old bitsteam 
        --> generate the ECM offset file B
step 3: Have another program to update A with B

We use option1 here
*/

static void Help(const char *program)
{
	printf("Generating Encrypted Bitstream Steps: \n");	
	printf("\tStep 1: %s ts ecmFileName emmFileName aesMode(0-2)\n", program);	
	printf("\tStep 2: run MpegDump using *.encrypted as input \n");	
	printf("\tStep 3: %s trickplay_ts ecmFileName emmFileName metaFilename stream_no(>=0) . \n");	
	printf("\tStep 4: you may want to rename the files \n");	
    exit(1);
}


cas_emm_payload * readEMMPayload(const char * emm_pl_fname)
{
    size_t num_bytes;
    FILE *emm_file;
    cas_emm_payload *p_emm = NULL;

    p_emm = (cas_emm_payload *)calloc(1, sizeof(cas_emm_payload));
    if( p_emm == NULL ) {
        perror("Error in readEMMPayload");
    }

    emm_file = fopen(emm_pl_fname, "r+b");
    if(emm_file)
    {
        
        /* 
           we assume that the key_len and the block_size have not changed since
           the payload was created
         */
        num_bytes = fread(p_emm, 1, sizeof(cas_emm_payload), emm_file);

        fclose(emm_file);
        
        if(num_bytes == sizeof(cas_emm_payload)) {
            return p_emm;
        } else {
            perror("emm payload read failed");
            free(p_emm);
        }
        
    }
    
    return NULL;
}

/*
 * This should go into a utils file perhaps
 */
static void hexPrintBuf(char * message, unsigned char * buf, int len)
{
    int i = 0;
    printf("\n---------------------------------");
    printf("\n  %s   len = %d bytes   ", message, len);
    printf("\n---------------------------------\n");
    for ( i = 0; i < len; i ++ ) {
        if ( i != 0 && !(i % 10) ) {
            printf("\n");
        }
        printf("%2x ", buf[i]);
    }
    fflush(stdout);
}

/* encrypts the EMM payload */
static void EMMAsymEncrypt(void *token, uint8_t *emm_buf, int emm_len, 
                            uint8_t *out_buf, int *out_len, long long  position)
{
    FILE  *fp_pukey;
    char  *pukey_fname = (char *)token; 

#ifdef _DEBUG  
    hexPrintBuf("emm_buf", emm_buf, emm_len);
#endif // _DEBUG
         
    /* sanity check */
    if(emm_len > *out_len) {
        fprintf(stderr, "Error, EMMAsymEncrypt output buffer is smaller than input buffer");
        return;
    }
   
    fp_pukey = fopen(pukey_fname, "rb+");
    if(fp_pukey == NULL) {
        perror("Cannot open public key file");
        return;
    }
     
	fprintf(stdout, "calling asym_encrypt\n");
    *out_len = asym_encrypt(fp_pukey, emm_buf, emm_len, out_buf);

#ifdef _DEBUG    
    hexPrintBuf("out_buf", out_buf, *out_len);
#endif // _DEBUG

    fprintf(stdout, "done EmmAsymEncrypt\n");
    fclose(fp_pukey); 
    return; 

}

/*
 * Used to encrypt the EMM payloads. Payloads first inserted into a EMM section
 * data structure, then encapsulated into 1 or more TS packets. For now,
 * irrespective of whether EMM is being sent in the stream or out of band they
 * will be encapsulated in TS packets.
 */
int encryptEMM(char *emm_pl_fname, char *pukey_fname, 
               char *emm_out_fname)
{
    ca_sys_p          p_sys, p_emm_sys;
    cas_emm_payload   *p_emm;
    
    p_emm = readEMMPayload(emm_pl_fname);
    if( p_emm == NULL ) {
        fprintf(stderr, "\nCould not read EMM payload. Exiting!");
        exit(-1);
    }

#ifdef _DEBUG
    printf ("key: ");
    for (i = 0; i<KEY_LENGTH; i++)
    {
        printf("%#x ", p_emm->key[i]);
    }
    printf("\n");
    printf ("iv : ");
    for (i = 0; i<BLOCK_SIZE; i++)
    {
        printf("%#x ", p_emm->iv[i]);
    }
    printf("\n");
#endif

    /* creating a CAS structure with some dummy variables */
    p_emm_sys = createCAS(CA_SYS_ID, FREQUENCY);
    casSetOutputCallback(p_emm_sys, EMMOutputCallBack, emm_out_fname);
    casSetEMSCallBack(p_emm_sys, EMMAsymEncrypt, pukey_fname);
    generateCAMessage(p_emm_sys, 0, 600, (uint8_t *)p_emm, sizeof(*p_emm));
    freeCAS(p_emm_sys);
    return 0;
}

#define NAME_LENGTH 256

int main(int argc, char **argv)
{
    char   *resourceName;
    char   encryted1XName[NAME_LENGTH];
    char   encryted1XTmpName[NAME_LENGTH];
    char   encrytedTrickName[NAME_LENGTH];
    char   metaName[NAME_LENGTH];
    char   emmName[NAME_LENGTH];
    FILE   *in, *out;
    cas_ecm_payload ecm;
    cas_emm_payload emm;
    sys_info_s sys_info;
    ca_sys_p   p_sys;
    ECMfile_info ecmfileinfo;
    uint8_t    aes_mode;
    InputOption option;
    int stream_no = 1;
    const char *progName = Basename(argv[0]);
    FILE *emmFile = NULL;
   
    /* EMM encrypt mode */
    if(argc == 4)
    {
        fprintf(stderr, "calling encryptEMM\n");
        encryptEMM(argv[1], argv[2], argv[3]); /* argv[1] = emm payload file name  
                                                  argv[2] = client public key file
                                                  argv[4] = output emm file */
       
        return 0;
    }
    
    else if(argc == 5) 
    {
        strcpy(sys_info.out_ecm_file_name, argv[2]);
        strcpy(sys_info.emm_file_name, argv[3]);
        aes_mode = atoi(argv[4]);
        if (aes_mode != 0 && aes_mode != 1 && aes_mode != 2)
        {
            Help(progName);
        }
        option = CREATE_1X;
    }
    
    else if (argc == 6)
    {
        strcpy(sys_info.in_ecm_file_name, argv[2]);
        strcpy(sys_info.emm_file_name, argv[3]);
        strcpy(metaName, argv[4]);
        stream_no = atoi(argv[5]);
        option = CREATE_TRICKPLAY;
    }
    else
    {
        Help(progName);	
    }
    
    /* information about the ECM file */
    ecmfileinfo.header.version = 1;
    ecmfileinfo.header.no_of_ECMs = 0;
    ecmfileinfo.header.ecm_len  = sizeof(cas_ecm_payload); 
    //    ecmfileinfo.header.metadata_offset = sizeof(ECMfile_header);
    ecmfileinfo.header.metadata_offset = ECM_FILE_HEADER_SIZE;
    ecmfileinfo.item_len = sizeof(FileOffset) + sizeof(FileOffset) + ecmfileinfo.header.ecm_len;
    
    resourceName = getResourceName(argv[1]);
    
    sys_info.p_ecm = &ecm; /* a ecm buffer */
    sys_info.p_emm = &emm; /* this EMM is used in CAS to encrypt the ECMs */
    
    switch (option)
    {
        case CREATE_1X:
        default:
		/* Step 0: create a EMM file as a license file, which is sent to the client */

	  createEMM(sys_info.p_emm, aes_mode, sys_info.emm_file_name);
	  sprintf(emmName, "%s.Trickplay.key", resourceName);
	  out = fopen(emmName, "wb");
	  if (out != NULL)
          {
	    int size;
	    size = fwrite(sys_info.p_emm, 1, sizeof(cas_emm_payload), out);
          }
            else
            {
                printf("Cannot open emm key file %s!\n", sys_info.emm_file_name);
                exit(1);
            }
          

		    sprintf(encryted1XName,"%s.en.mpg",resourceName);
		    sprintf(encryted1XTmpName,"%s.encrypted",resourceName);
		/* Step 1: generate a temporary file without encryption */
		    out = fopen(sys_info.out_ecm_file_name, "wb");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);
		    
		    p_sys = createCAS(CA_SYS_ID,FREQUENCY);
		    sys_info.ecm_no = 0; // no ECM inserted now
		    sys_info.p_ecm_sys = p_sys;
		    out = fopen(encryted1XTmpName, "wb");
		    GetEncryptedBitstream(p_sys, &sys_info, argv[1], out, TRUE, NULL, ECMCallBack);
		    freeCAS(p_sys);
		    fclose(out);

		    /*update ECMfile header */
		    ecmfileinfo.header.no_of_ECMs = sys_info.ecm_no;
		    out = fopen(sys_info.out_ecm_file_name, "r+b");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);

		/* Step 2: generate encryted bitstream for input TS*/
		    printf("generating encryted bitstream ... \n");
		    out = fopen(sys_info.out_ecm_file_name, "wb");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);

		    out = fopen(encryted1XName, "wb");
		    p_sys = createCAS(CA_SYS_ID,FREQUENCY);
//		    casSetPMTVersionNumber(p_sys, 1);
		    sys_info.ecm_no = 0;
		    sys_info.p_ecm_sys = p_sys;
		    GetEncryptedBitstream(p_sys, &sys_info, argv[1], out, TRUE, ScrambleCallBack, ECMCallBack);
		    freeCAS(p_sys);
		    fclose(out );
		    
		    /*update ECMfile header */
		    ecmfileinfo.header.no_of_ECMs = sys_info.ecm_no;
		    out = fopen(sys_info.out_ecm_file_name, "r+b");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);
		    printf("\n...Done...\n");

        	break;
        case CREATE_TRICKPLAY:
            assert(in = fopen(metaName, "rb"));
        	sys_info.b_reverse = CheckReverse(in, stream_no);
            fclose(in);
		    if (sys_info.b_reverse)
		    {
		    	sprintf(encrytedTrickName,"%s.en.mpg_RWND",resourceName);
		    }
		    else
		    {
		    	sprintf(encrytedTrickName,"%s.en.mpg_FFWD",resourceName);
		    }
            
            emmFile = fopen(sys_info.emm_file_name, "rb");
            if (emmFile != NULL)
            {
                fread(sys_info.p_emm, 1, sizeof(cas_emm_payload), emmFile);
                fclose(emmFile);
            }
            else
            {
                printf("Cannot open emm key file %s!\n", sys_info.emm_file_name);
                exit(1);
            }
    
#ifdef _DEBUG
		    assert(in = fopen(sys_info.in_ecm_file_name, "rb"));
		    assert(out = fopen("ecm_old.ascii","w"));
		    ConvertECMfile(in, out);
		    fclose(in);
		    fclose(out);
#endif		    
		/* Step 3: Update the meta data file */
		    assert(in = fopen(sys_info.in_ecm_file_name, "r+b"));
		    assert(out = fopen(metaName,"r+b"));
		    UpdateECMandMetaFile(in, out, stream_no, 188); /* for the ECM we have, it can fit into one TS */
		    fclose(in);
		    fclose(out);
		    
#ifdef _DEBUG
		/* what happened ? */
		    assert(in = fopen(sys_info.in_ecm_file_name, "rb"));
		    assert(out = fopen("ecm_new.ascii","w"));
		    ConvertECMfile(in, out);
		    fclose(in);
		    fclose(out);
#endif
		/* Step 4: encrypt the trickplay stream */
#ifdef _DEBUG
		    sprintf(sys_info.out_ecm_file_name, "%s.Trickplay.ecm",resourceName);
		    out = fopen(sys_info.out_ecm_file_name, "wb");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);
#endif

		    printf("generating encryted trickplay bitstream ... \n");
		    sys_info.ecm_id  = 500; // currently the pid is 500 
		    out = fopen(encrytedTrickName, "wb");
		    p_sys = createCAS(CA_SYS_ID,FREQUENCY);
		    casSetTsCallBack(p_sys, TSCallBack, &sys_info);
		    sys_info.p_ecm_sys = p_sys;
		    sys_info.ecm_no = 0;
		    sys_info.ecm_no_inserting = 0;
		    GetEncryptedBitstream(p_sys, &sys_info, argv[1], out, FALSE, ScrambleCallBack, ECMIOnlyCallBack);
		    freeCAS(p_sys);
		    fclose(out);
		    
#ifdef _DEBUG
		    /*update ECMfile header */
		    ecmfileinfo.header.no_of_ECMs = sys_info.ecm_no;
		    out = fopen(sys_info.out_ecm_file_name, "r+b");
		    WriteECMFileHeader(out, &ecmfileinfo);
		    fclose(out);
		    printf("\n...Done...\n");
#endif
		        	
        	break;

    }
    free(resourceName);
    return 0;
}

