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
 * $(@)cas_sys.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*----------------------------------------------------------------
 * Sun project header file (ca_sys.h)
 *
 * Libs, macros, data types common to all modules
 *
 * NOTE: this file depends on libraries MMP and Klingon
 * Created:   July 29, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
 
#ifndef _CA_INJECTION_H
#define _CA_INJECTION_H
/* Common Includes */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include "klingon/ts_psi.h"

/* cobjs Includes */
#include "cobjs/ArrayOf.h"
#include "cobjs/HashTable.h"
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"
/*#include "cobjs/libcobjsVersion.h"*/

/* mmp Includes */
#include "mmp/Mmp.h"
#include "mmp/Context.h"
#include "mmp/Mpeg2.h"
#include "mmp/Mpeg2Pat.h"
#include "mmp/Mpeg2Pes.h"
#include "mmp/Mpeg2Pmt.h"
#include "mmp/Mpeg2Ps.h"
#include "mmp/Mpeg2Psi.h"
#include "mmp/Mpeg2Ts.h"
#include "mmp/Pipe.h"
/*#include "mmp/libmmpVersion.h"*/
//#include "vsm/Vsm.h"


#include "cas/cas_lib.h"
typedef unsigned char byte_t;
typedef byte_t * byte_p;

#define TEST_MEMORY(p) if(p==NULL){ fprintf(stderr, "Memory check failed in %s line %d\n", __FILE__, __LINE__);exit(-1);}
#define TP_LENGTH 188

#define getbit(b,n,mask) ((b>>n)&mask)
#define putbit(b,n,val) (b|=((val)<<n))

/* Funtions declaration */
#ifdef __cplusplus
extern "C" {
#endif

/* Data types (enums, structs, typedefs) */


struct _Mpeg2Ts {
    Mpeg2TsCallBack     callBack;
    void               *token;
    MmpParserObject    *psip;		   /* PSI parser */
    MmpParserObject     po;
    Context		context;
    MmpContextObject   *cop;
    Boolean		isDirty;
    Boolean		doFlush;
};

typedef struct 
{
	byte_t data[MPEG2_TS_PKT_SIZE];
} ca_ts_pck_t, *ca_ts_pck_p;

typedef struct parser_s
{
    Mmp                 mmp;
    Mpeg2Ts             mpeg2Ts;
    Mpeg2Psi            mpeg2Psi;
    Mpeg2Pat            mpeg2Pat;
    Mpeg2Pmt            mpeg2Pmt;
} parser_t;

/* encrypt_key_s: the payload of ECM message section  is contained in buf */
typedef struct encrypt_key_s
{
	 int                pn;           /* program number we are encrypting */
	 encrypt_type       e_type;       /* encrypt type for this key */
	 short              ecm_pid;      /* pid of the ecm containing the key */
	 byte_p             key_buf;          /* key payload */
	 int                key_length;       /* payload length */
	 struct encrypt_key_s *next;      /* next key */
} encrypt_key_t,  *encrypt_key_p;
 
/* structure of the ca descriptor,one ca descriptor corresponds to one key only */
typedef struct ts_ca_descriptor_s 
{
  int                        stream_type; /* the ca is used for this stream type,  */
  encrypt_type               en_type;
  void                       *desc;       /*  the PSI descriptor */
  encrypt_key_p              p_key;       /* encryption key this CA from */
  struct ts_ca_descriptor_s  *next;
} ts_ca_descriptor_t;
typedef struct ts_ca_descriptor_s *ts_ca_descriptor_p;

/* pid structure, contains info for this pid */
typedef struct
{
    Boolean       b_pmt_pid; /* is it a pck pid? */
    Boolean       b_seen;    /* being used ? */
    int           pn;        /* program number */
    encrypt_key_p p_key;     /* encryption key for this pid. chosen from the keys in sys */
    byte_t  continuity_counter; /* TS packet counter */
} ts_pid_t;

/* sys info structure */
typedef struct ca_sys_s
{
     int                 b_outputECMafterPMT;
	 ca_ts_pck_t         pkt_buf;        /* current TS pakcet being process */
	 Pipe                outpipe;        /* the pipe where the output TS will be directed to */
	 PipePosition        putPosition;    /* output pipe position of outpile */
	 ts_pid_t            pid[8192];      /* pid arrays  */
	 encrypt_key_p       p_keys;         /* a list of keys of the CAS */
	 int                 ecm_frequency;  /* frequence of sending ECMs, seconds */
	 MmpContextObject    *mpeg2TScop;
	 parser_p            p_parser;       /* TS demuxer */
	 casOutputCallBack   outputCallback; /* output callback funtion pointer */
	 casEMSCallBack      emsCallback;    /* Entitle message sextion callback */
	 casScrambleCallBack scrambleCallback; /* srambling callback function */
	 casTSCallBack       tsCallback;      /* insertion callback function */
	 void                *output_token;
	 void                *ems_token;
	 void                *scramble_token;
	 void                *ts_token;
	 int                 i_ca_system_id;
	 int                 i_pmt_version_number;
} ca_sys_t;





parser_p   setupNewParser(ca_sys_p p_sys);
void       freeParser(parser_p parser);
ts_ca_descriptor_p ts_ca_descriptor_create(encrypt_type en_type, int ca_system_id,
	                    int ca_pid, byte_p p_private_data, int i_private_length);
ts_ca_descriptor_p ts_ca_descriptor_append(ts_ca_descriptor_p list, 
                        ts_ca_descriptor_p );
void               ts_ca_descriptor_delete(ts_ca_descriptor_p list);

void injectCA2PMT(ca_sys_p p_sys, Mpeg2PmtInfo *info, short program_map_pid, ts_ca_descriptor_p ca_list);
void ouputECMs(ca_sys_p p_sys, int program_number);
encrypt_type getStreamType(int i_stream_type );
void scrambleTsPacket(ca_sys_p p_sys, unsigned   pid, byte_p pkt, byte_p payload, int length, long long curTSpos);

#ifdef __cplusplus
}
#endif

#define TRACE

#ifdef TRACE
/*void msg_TRACE(const char *psz_format, ...);*/
#define msg_TRACE printf
#else
#define msg_TRACE(a)
#define msg_TRACE(a,b)
#define msg_TRACE(a,b,c)
#define msg_TRACE(a,b,c,d,e)
#define msg_TRACE(a,b,c,d,e,f)
#define msg_TRACE(a,b,c,d,e,f,g)
#define msg_TRACE(a,b,c,d,e,f,g,h)
#endif

#endif


