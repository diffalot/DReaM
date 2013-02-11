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
 * $(@)cas_lib.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*----------------------------------------------------------------
 * Sun project header file (ca_lib.h)
 *
 * Libs, macros, data types common to all modules
 *
 * NOTE: this file depends on libraries MMP and Klingon
 * Created:   August 2, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
 
#ifndef _CA_LIB_H
#define _CA_LIB_H

#define FOR_ANY_PROGRAM -1
#define ANY_STREAM_TYPE -1
#define bool int

typedef void  (*casOutputCallBack) (void *token, unsigned char *buf, int len);
typedef void  (*casEMSCallBack) (void *token, unsigned char *buf, int len, unsigned char *out_buf, int *out_len, 
	                             long long position);
typedef void  (*casScrambleCallBack) (void *token, unsigned char *key_buf, int key_len, 
	                                  unsigned char *pkt, unsigned char *payload_buf, int payload_len,
	                                  long long position);
typedef void  (*casTSCallBack) (void *token, long long position);

typedef struct parser_s *parser_p;
typedef struct ca_sys_s *ca_sys_p;

typedef enum encrypt_type_s 
{
	ENCRYPT_ALL, 
	ENCRYPT_VIDEO_ONLY, 
	ENCRYPT_AUDIO_ONLY,
	ENCRYPT_UNKNOWN
}encrypt_type;

/* Funtions declaration */
#ifdef __cplusplus
extern "C" {
#endif

void       addKey2CAS(ca_sys_p p_sys, int pn, encrypt_type type, unsigned char *p_payload, int i_length);
ca_sys_p createCAS(int sys_id,unsigned int seconds);
void       freeCAS(ca_sys_p p_sys);
int        casEOF(ca_sys_p p_sys);
int        casPutData(ca_sys_p p_sys, unsigned char *buf, int buf_len);

void casSetOutputCallback (ca_sys_p p_sys, casOutputCallBack callBack, void *token);
void casSetEMSCallBack (ca_sys_p p_sys, casEMSCallBack callBack, void *token);
void casSetScrambleCallBack (ca_sys_p p_sys, casScrambleCallBack callBack, void *token);
void casSetPMTVersionNumber(ca_sys_p p_sys, int version_number);
long generateCAMessage(ca_sys_p p_sys, bool ecm, short pid, unsigned char *buf, int buf_len);
void casECMafterPMT(ca_sys_p p_sys, int value);
void casSetTsCallBack (ca_sys_p p_sys, casTSCallBack callBack, void *token);

#ifdef __cplusplus
}
#endif


#endif


