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
 * $(@)Mpeg1AudioEs.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/* This defines the Audio Elementary stream structures */

#ifndef _MMP_MPEG1AUDIOES_H
#define _MMP_MPEG1AUDIOES_H

#include <values.h>

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Pipe.h"
#include "mmp/Mpeg2Pes.h"

#include <synch.h>

_FOREIGN_START

/* The Mpeg1 Audio Elementary Stream Header */

#define MPEG1_AUDIOES_MAX_CHUNK 512
#define MPEG1_AUDIO_PRE_SCAN_DISTANCE 8

typedef struct _Mpeg1AudioEs *Mpeg1AudioEs;

typedef struct Mpeg1AudioEsHeader{
  BIT1(syncword11_4:8);
  BIT4(syncword3_0:4,
       id:1,
       layer:2,
       protectionBit:1);
  BIT4(bitrateIndex:4,
       samplingFrequency:2,
       paddingBit:1,
       privateBit:1);  
  BIT5(mode:2,
       modeExtension:2,
       copyright:1,
       original:1,
       emphasis:2);
}Mpeg1AudioEsHeader;


#define MPEG1_AUDIO_ES_SYNCWORD(ppp)   ((ppp->syncword11_4 << 4) | (ppp->syncword3_0))

#define IS_MPEG1_AUDIO_ES_SYNCWORD(ppp)   (MPEG1_AUDIO_ES_SYNCWORD(ppp) == 0x0FFF) 

typedef struct Mpeg1AudioEsInfo{
  u16                    pid;
  double                 samplingRate ;
  u32                    bitRate ;
  PipePosition           position;
  Mpeg1AudioEsHeader     mpeg1AudioEsHeader;
}Mpeg1AudioEsInfo;


typedef RetCode  (*Mpeg1AudioEsCallBack)(void *token, Mpeg1AudioEsInfo *infop, RetCode);

struct _Mpeg1AudioEs{
  Mpeg1AudioEsCallBack   callBack;
  void                   *token;
  MmpParserObject        po;
  Boolean                isDirty;
  Boolean                doFlush ;
  /*int                    filep;*/
  u8                     *bufBase;
  u8                     *bufDataptr;
  u32                    *bufLimit;
  u32                    *bufDatasize;
  u8                     *curptr;
  sema_t                 semaBufFilled;
  sema_t                 semaBufReleased;
  Boolean                parseDone;
  Boolean                noAudio;
};


extern MmpParserObject *mpeg1AudioEsParserObject(Mpeg1AudioEs mpeg1AudioEs);

extern Mpeg1AudioEs mpeg1AudioEsNew(void );

extern void   mpeg1AudioEsFree(Mpeg1AudioEs);

extern void mpeg1AudioEsSetCallBack(Mpeg1AudioEs, Mpeg1AudioEsCallBack, void *);

_FOREIGN_END

#endif
