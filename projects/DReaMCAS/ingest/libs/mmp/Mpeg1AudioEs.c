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
 * $(@)Mpeg1AudioEs.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/* This has the parsing engine for the Mpeg1 Audio */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"


#include "mmp/Mmp.h"
#include "mmp/Context.h"
#include "mmp/Pipe.h"

#include "mmp/Mpeg1AudioEs.h"
//#include "util/utLog.h"

#include <synch.h>

/* Table for the bitrates(in kbits/sec) for the various layers */
static u32 bitRate[3][15] = 
{ 
  {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
  {0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384},
  {0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320}
};

/* Table for the sampling frequencies */
static float sampFrequency[4] = {44.1, 48.0, 32.0, 0};


static size_t  mpeg1AudioEsMaxChunk(void *instp);
static RetCode mpeg1AudioEsRecognize(void *instp, Pipe pipe) ;

static RetCode mpeg1AudioEsParse(void *instp, MmpContextObject *cop, Pipe pipe) ;

static void mpeg1AudioEsRecover(void *instp);

static Mpeg1AudioEsHeader *findMpeg1AudioSyncWord(Pipe pipe);

RetCode mpeg1AudioSetoutputfile(Mpeg1AudioEs mpeg1AudioEs, int filep);

/*RetCode mpeg1AudioSetBuf(Mpeg1AudioEs mpeg1AudioEs, u8 *audioDfBuffer);*/

/* find the syncword */
Mpeg1AudioEsHeader *
findMpeg1AudioSyncWord(Pipe pipe){
  Mpeg1AudioEsHeader *ppp;
  /* searches continuously for the sycnword till it finds one */

  while(1){
    ppp = PIPE_PEEK(pipe, Mpeg1AudioEsHeader);
    if(IS_MPEG1_AUDIO_ES_SYNCWORD(ppp))
      break;
    else
      PIPE_SKIP(pipe, u8);
  }
  return ppp;
}


Mpeg1AudioEs
mpeg1AudioEsNew(void){

  Mpeg1AudioEs      mpeg1AudioEs;

  /* if(retCodeId == 0){
     retCodeId = retCodeRegisterWithTable(MPEG1AUDIO_ES_CLASSNAME,
     mpeg1AudioEsErrorTable);
     }
  */
  //UT_LOG_MESG(&logp, (&logp, "mpeg1AudioEsNew : called \n"));
  mpeg1AudioEs = NEW_ZEROED(struct _Mpeg1AudioEs, 1);

  mpeg1AudioEs->po.maxChunk = mpeg1AudioEsMaxChunk ;
  mpeg1AudioEs->po.recognize = mpeg1AudioEsRecognize ;
  mpeg1AudioEs->po.parse  = mpeg1AudioEsParse ;
  mpeg1AudioEs->po.recover = mpeg1AudioEsRecover ;
  mpeg1AudioEs->po.instp  = mpeg1AudioEs ;

  return mpeg1AudioEs ;
}

MmpParserObject *
mpeg1AudioEsParserObject(Mpeg1AudioEs mpeg1AudioEs){
  return &mpeg1AudioEs->po;
}

void
mpeg1AudioEsSetCallBack(Mpeg1AudioEs mpeg1AudioEs, Mpeg1AudioEsCallBack callBack,
			void *token){
  mpeg1AudioEs->callBack = callBack;
  mpeg1AudioEs->token = token;
}


RetCode 
mpeg1AudioSetOutputFile(Mpeg1AudioEs mpeg1AudioEs, int filep){
  /*  mpeg1AudioEs->filep = filep;*/
  return RETCODE_SUCCESS;
}

/*RetCode
mpeg1AudioSetBuf(Mpeg1AudioEs mpeg1AudioEs, u8 *audioDfBuffer){
  mpeg1AudioEs->buf = audioDfBuffer;

  mpeg1AudioEs->buf->datasize = 0;

  return RETCODE_SUCCESS ;
}*/

static RetCode
mpeg1AudioEsRecognize(void *instp, Pipe pipe){
  Mpeg1AudioEsHeader  *ppp = PIPE_PEEK(pipe, Mpeg1AudioEsHeader);
  
  if(MPEG1_AUDIO_ES_SYNCWORD(ppp)){
    //UT_LOG_MESG(&logp, (&logp, "mpeg1AudioEsRecognize : Audio syncword recognized \n"));
    return RETCODE_SUCCESS;
  }else{
    //UT_LOG_MESG(&logp, (&logp, "mpeg1AudioEsRecognize : Audio syncword not found \n"));
    return RETCODE_FAILED;
  }
}

static size_t
mpeg1AudioEsMaxChunk(void *instp){
  return MPEG1_AUDIOES_MAX_CHUNK;
}

static void
mpeg1AudioEsRecover(void *instp){
  Mpeg1AudioEs         mpeg1audio = (Mpeg1AudioEs)instp;
  
  mpeg1audio->isDirty = FALSE;
  //UT_LOG_MESG(&logp, (&logp, " mpeg1AudioEsRecover : called \n"));
  /* will be implemented later */
}

void 
mpeg1AudioEsFree(Mpeg1AudioEs mpeg1AudioEs){
  /* free the allocated audio parser */
  //UT_LOG_MESG(&logp, (&logp, " mpeg1AudioEsFree : called \n"));
  free(mpeg1AudioEs);
}


/* The first syncword can be found at the start of a PES stream; but the later syncwords
   should be seeked by computing the  distance between two consecutive syncwods
   (ISO:3-11171 rev 1 Pg 38).  */

static RetCode
mpeg1AudioEsParse(void *instp, MmpContextObject *cop, Pipe pipe){
  
  Mpeg1AudioEs              mpeg1AudioEs = (Mpeg1AudioEs)instp;
  Mpeg1AudioEsInfo          info;
  Mpeg1AudioEsInfo         *infop = &info;
  Mpeg1AudioEsHeader       *ppp;
  RetCode                   retCode = RETCODE_SUCCESS;
  u8                        *buf ;
  u32                       len, getlen ;
  unsigned                  pid;
  unsigned                  skiplen = 0, slotcount;
  float                     numslots, frac;
  u32                       curlen = 0;

  //UT_LOG_MESG(&logp, (&logp, "mpeg1AudioEsParse : called \n"));

  mpeg1AudioEs->isDirty = TRUE;
  pid = MMP_CONTEXT_PIPETOPID(cop, pipe);

  if(pid != MMP_PID_NULL){
    pipeFindMark(pipe);
  }

  ppp = findMpeg1AudioSyncWord(pipe);

 mpeg1AudioEs->curptr = mpeg1AudioEs->bufDataptr;

  *(&(mpeg1AudioEs->noAudio)) = FALSE;

 while(pipeIsAvail(pipe, 1)){
    if(pipeIsAtMark(pipe)){
      buf = pipeGet(pipe, 1) ;
    }else{
      len = pipeAvailUnmarked(pipe) ;
       pipeSkip(pipe,len);
      }
    }
 fprintf( stderr, " audio_es: posting outside while \n");
  //sema_post(&(mpeg1AudioEs->semaBufFilled));

 /*if(mpeg1AudioEs->parseDone){
    sema_post(&(mpeg1AudioEs->semaBufFilled));
    sema_wait(&(mpeg1AudioEs->semaBufReleased));
  }*/
  
  //UT_LOG_MESG(&logp, (&logp, " Returned from the Audio parser (pipeIsAvail) \n"));
  return retCode;
}
