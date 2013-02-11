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
 * $(@)Mpeg2VideoEs.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:39 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/* This file would contain all the parser utilities of a Mpeg2 Video 
Elementary Stream */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Context.h"
#include "mmp/Pipe.h"
#include "mmp/Mpeg2VideoEs.h"

//#include "util/utLog.h"
#include <assert.h>
#include <synch.h>
#include "metadata.h"


static size_t  mpeg2VideoEsMaxChunk(void *instp);

static void    mpeg2VideoEsRecover(void *instp);

static RetCode mpeg2VideoEsParse(void *instp, MmpContextObject *cop, Pipe pipe) ;

static u64  adjustIntraQuantMatrix(u64 *, u64);

RetCode mpeg2VideoSetOutputFile(Mpeg2VideoEs mpeg2VideoEs, int filep);

extern metadata *init_metadata(char *meta_name, int speed);

u64
adjustIntraQuantMatrix(u64 *p , u64 initbit){

  u64 nextbit ;
  int i;
  
  for(i = 0 ;  i < 8 ; i++){
    nextbit = *p & 0x0000000000000001 ;
    *p = *p >> 1 ;
    *p = (*p) | ( initbit << 63) ; 
    initbit = nextbit ;
    if(i < 7) 
      p++ ;
  }
  return nextbit ;
}
    
  
  
Mpeg2VideoEsStartCode *
skipToNextStartCode(Pipe pipe){
  Mpeg2VideoEsStartCode *ppp ;

  while(1){
    ppp = PIPE_PEEK(pipe, Mpeg2VideoEsStartCode) ;
    if(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(ppp))
      break ;
    else 
      PIPE_SKIP(pipe, u8) ;
  }
  return ppp ;
    
}


Mpeg2VideoEsStartCode *
copyTillNextStartCode(Pipe pipe, FILE *file){
  Mpeg2VideoEsStartCode *ppp ;

  while(1){
    ppp = PIPE_PEEK(pipe, Mpeg2VideoEsStartCode) ;
    if(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(ppp))
      break ;
    else{ 
      assert(fwrite(ppp,1,sizeof(u8),file));
      PIPE_SKIP(pipe, u8) ;
    }
  }
  return ppp ;
    
}

/*



Mpeg2VideoEsStartCode *
skipToNextStartCode(Pipe pipe){
  Mpeg2VideoEsStartCode *ppp ;

  while(1){
    ppp = PIPE_PEEK(pipe, Mpeg2VideoEsStartCode) ;
    if(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(ppp))
      break ;
    else 
      PIPE_SKIP(pipe, u8) ;
  }
  return ppp ;
    
}

PipePosition 
copyTillNextStartCode(Pipe pipe,Mpeg2VideoEsStartCode **ppp, FILE *file){

 PipePosition pos =0; 
  while(1){
    *ppp = PIPE_PEEK(pipe, Mpeg2VideoEsStartCode) ;
    if(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(*ppp))
      break ;
    else{ 
      assert(fwrite(*ppp,1,sizeof(u8),file));
      PIPE_SKIP(pipe, u8) ;
      pos++;
    }
  }
  return pos;
    
}


*/

static RetCodeId    retCodeId;
FILE *file;
FILE *metadata_file;
Mpeg2VideoEs
mpeg2VideoEsNew(void){
  
  Mpeg2VideoEs      mpeg2VideoEs ;

/*  if(retCodeId == 0){
     retCodeId = retCodeRegisterWithTable(MPEG2VIDEO_ES_CLASSNAME,
     mpeg2VideoEsErrorTable);
  }
 */
  //UT_LOG_MESG(&logp, (&logp, "Mpeg2VideoEsNew : called \n"));


  mpeg2VideoEs  = NEW_ZEROED(struct _Mpeg2VideoEs, 1);
  mpeg2VideoEs->bufBase = mpeg2VideoEs->bufDataptr = (u8 *)malloc(DEMUX_BUF_LEN * sizeof(u8));
  mpeg2VideoEs->bufLimit = (unsigned long*)malloc(sizeof(u8));
  *(mpeg2VideoEs->bufLimit) = DEMUX_BUF_LEN;
  mpeg2VideoEs->bufDatasize = (unsigned long*)malloc(sizeof(u8));
  *(mpeg2VideoEs->bufDatasize) = 0;

  mpeg2VideoEs->po.maxChunk = mpeg2VideoEsMaxChunk ;
  mpeg2VideoEs->po.recognize = mpeg2VideoEsRecognize ;
  mpeg2VideoEs->po.parse  = mpeg2VideoEsParse ;
  mpeg2VideoEs->po.recover = mpeg2VideoEsRecover ;
  mpeg2VideoEs->po.instp  = mpeg2VideoEs ;
  mpeg2VideoEs->context = contextNew();
  mpeg2VideoEs->cop = contextMmpContextObject(mpeg2VideoEs->context);
  mpeg2VideoEs->resourceName = "";
  mpeg2VideoEs->metaName = "";
 
  return mpeg2VideoEs ;
  
}

MmpParserObject *
mpeg2VideoEsParserObject(Mpeg2VideoEs mpeg2VideoEs){
  return &mpeg2VideoEs->po;
}

RetCode
mpeg2VideoSetOutputFile(Mpeg2VideoEs mpeg2VideoEs, int filep){
  /* This sets filedescriptor for writing Video Elementary Stream */
  /*  mpeg2VideoEs->filep = filep;*/
  return RETCODE_SUCCESS ;
}

void mpeg2VideoEsSetResourceMetaName(Mpeg2VideoEs mpeg2VideoEs, char
*resourceName, char* metaName){
    mpeg2VideoEs->resourceName = resourceName;
    mpeg2VideoEs->metaName = metaName;
}

void
mpeg2VideoEsSetCallBack(Mpeg2VideoEs mpeg2VideoEs, Mpeg2VideoEsCallBack callBack,
			void *token){
  //UT_LOG_MESG(&logp, (&logp, "mpeg2VideoEsSetCallBack : called \n"));
  mpeg2VideoEs->callBack = callBack ;
  mpeg2VideoEs->token = token ;
}

static size_t
mpeg2VideoEsMaxChunk(void *instp){
  return MPEG2_VIDEOES_MAX_CHUNK ;
}

static RetCode
mpeg2VideoEsRecognize(void *instp, Pipe pipe){
  Mpeg2VideoEsStartCode          *ppp =  PIPE_PEEK(pipe, Mpeg2VideoEsStartCode);
  
  
  if(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(ppp)){
    //UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsRecognize : Video Es recognized \n"));
    return RETCODE_SUCCESS;
  }else{
     //UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsRecognize : Video Es not recongized \n"));
     return RETCODE_FAILED;
  }
 
}

void
mpeg2VideoEsFree(Mpeg2VideoEs mpeg2VideoEs){
  free(mpeg2VideoEs);
  //UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsFree : called \n"));
}

static void
mpeg2VideoEsRecover(void *instp){

  Mpeg2VideoEs        mpeg2video = (Mpeg2VideoEs)instp;
  mpeg2video->isDirty = FALSE;
 // UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsRecover : Called \n"));
  /* will be implemented later */
}



RetCode
mpeg2VideoEsParse(void *instp, MmpContextObject *cop, Pipe pipe){

  Mpeg2VideoEs               mpeg2VideoEs = (Mpeg2VideoEs)instp ;
  Mpeg2VideoEsInfo           info ;
  Mpeg2VideoEsInfo          *infop = &info ;
  Mpeg2VideoEsStartCode     *ppp ;
  int                        seqHdrflag = 0, len, getlen;
  u8                        *buf ;
  RetCode                    retcode = RETCODE_SUCCESS ;
  unsigned                   pid ;
  Boolean                    exitwhile = FALSE;
  unsigned                   startcode ;
  PipePosition               pos, pos2;
  u32                        curlen = 0;
  u8	     				 *tmp;
  

   mpeg2VideoEs->isDirty = TRUE ;
   
   pid = MMP_CONTEXT_PIPETOPID(cop, pipe);
   
   //	resource_name="../../../repository/temp";
   



   //UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsParse : video Parser called \n"));
	
	create_1X_trickplay_stream(pipe, mpeg2VideoEs->resourceName,
                                mpeg2VideoEs->metaName );
	
	return RETCODE_SUCCESS;
}


Boolean create_1X_trickplay_stream(Pipe pipe, char *resource_name , char *meta_file_name ){

   FileOffset pos;
   Mpeg2VideoEsStartCode     *ppp;
   u8 startcode;
   Mpeg2VideoPictureHeader *picHeader;
   long frameNumber;
//   char *meta_file_name;
   char *output_stream_name;
   metadata *meta;
   FILE* meta_file;
   FILE* file, *temp_file;
   int err;
   int frametype;
   picture_metadata temp;
   long avg_frame_len;
   
	
   temp_file = fopen("ascii.meta", "w");	
   output_stream_name= (char *)malloc(strlen(resource_name)+ 10);  
   sprintf(output_stream_name, "%s.1X.es", resource_name); 
   file= fopen(output_stream_name,"wb");	
   if (file == NULL)
   		FATAL_ERROR("Cannot open file for stream");
   
   //initialise the metadata file	 
   // meta_file_name= (char *)malloc(strlen(resource_name)+ 10);  
   // sprintf(meta_file_name, "%s.mpg_META", resource_name); 
   meta = init_metadata(meta_file_name,1);
   meta_file = meta->meta_file; 
   assert(meta_file != NULL);

   ppp=skipToNextStartCode(pipe);
   frameNumber = -1;
   while(pipeIsAvail(pipe, 1)){ 

    pos = pipePosition(pipe);
    ppp = PIPE_GET(pipe, Mpeg2VideoEsStartCode) ;
    assert(IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(ppp));  //expecting a startcode
    startcode = ppp->packetStartCodeByte;
    if (startcode == MPEG2_VIDEO_SEQUENCE_END_CODE) {
	fwrite(ppp,1,sizeof(*ppp),file);
	break; // this is slightly fragile as we expect the seqence eddnd always
    }
    
    if(IS_PICTURE_START_CODE(startcode)){ 
	  frameNumber++;
       picHeader = PIPE_GET(pipe,Mpeg2VideoPictureHeader);
	  if (IS_IFRAME(picHeader)) 
	  	  frametype = INTRA_CODED_TYPE;
	   else if(IS_PFRAME(picHeader))
	   	  frametype = PREDICTIVE_CODED_TYPE;
	   else if(IS_BFRAME(picHeader))
	   	   frametype = BIDIRECTIONAL_CODED_TYPE;
	   else {
	   	   frametype = UNKNOWN;
		   fprintf(stderr, "found an unknown picture frame at offset%ullx\
		   	 		and frame number %l",pos, frameNumber);
		}	
		// writing the meta data 
	 temp.frame_number = frameNumber;
	 temp.frametype = frametype;
	 temp.offset = pos;
	 fwrite(&temp,1,sizeof(temp),meta_file);
	 fprintf(temp_file,"%ld  %d  %llx\n", frameNumber, frametype, pos);
	 copy_picture_header(ppp,picHeader,file);
	 copyTillNextStartCode(pipe,file);

   }else {

   	assert(fwrite(ppp,1,sizeof(*ppp),file)== sizeof(*ppp));
   	copyTillNextStartCode(pipe,file);
     }
  }
   // find the average frame length
	avg_frame_len = pos/(frameNumber-1);
	fseek(meta_file,sizeof(metadata_header)+sizeof(int),SEEK_SET);  //sizeof(int) is for stream_metadata->speed
	fwrite(&avg_frame_len,1,sizeof(avg_frame_len),meta_file);
  //meta_cleanup(meta); 
   fclose(file);
   fclose(meta_file);
   return Boolean(true);//FIXME
}



void copy_picture_header(Mpeg2VideoEsStartCode *ppp,Mpeg2VideoPictureHeader *header, FILE *file){
	 
	assert(file);
	assert(ppp);
	assert(header);
	assert(fwrite(ppp,1,sizeof(Mpeg2VideoEsStartCode),file) == sizeof(Mpeg2VideoEsStartCode)); //startcode written
	assert(fwrite(header,1,sizeof(header),file) == sizeof(header)); //writen the heder;	
	
}

void copyIFrame(Pipe pipe, FILE *file){
	
	Mpeg2VideoEsStartCode *ppp;
	u8 startcode;
	Boolean done;
	done = Boolean(false); 
	//copy till the next header is not 
	do {
		assert(ppp = copyTillNextStartCode(pipe,file));
		startcode = ppp->packetStartCodeByte;	
		if ( startcode != MPEG2_VIDEO_EXTENSION_START_CODE &&
	       		startcode != MPEG2_VIDEO_USER_DATA_START_CODE &&
			(startcode >= MPEG2_VIDEO_SLICE_START_CODE_MIN && 
		 	startcode <= MPEG2_VIDEO_SLICE_START_CODE_MAX)
	       	){ 
			ppp = PIPE_GET(pipe, Mpeg2VideoEsStartCode) ;
			fwrite(ppp,1,sizeof(Mpeg2VideoEsStartCode),file);
		  }
		else
		 	done = Boolean(true);


	 }while(done == false); 
	return;
}

Mpeg2VideoEsStartCode* skipPictureFrame(Pipe pipe){
	
	Mpeg2VideoEsStartCode *ppp;
	u8 startcode;
	Boolean done;
	done = Boolean(false); 
	//copy till the next header is not 
	do {
		assert(ppp = skipToNextStartCode(pipe));
		startcode = ppp->packetStartCodeByte;	

	}while( startcode != MPEG2_VIDEO_EXTENSION_START_CODE &&
	       		startcode != MPEG2_VIDEO_USER_DATA_START_CODE &&
			(startcode >= MPEG2_VIDEO_SLICE_START_CODE_MIN && 
		 	startcode <= MPEG2_VIDEO_SLICE_START_CODE_MAX)
	       	); 
	return ppp;
}





/*
PipePosition copy_picture_header(Mpeg2VideoEsStartCode *ppp,Mpeg2VideoPictureHeader *header, FILE *file){
	 
	PipePosition pos = 0;
	assert(file);
	assert(ppp);
	assert(header);
	assert(fwrite(ppp,1,sizeof(Mpeg2VideoEsStartCode),file) == sizeof(Mpeg2VideoEsStartCode)); //startcode written
	assert(fwrite(header,1,sizeof(*header),file) == sizeof(*header)); //writen the heder;	
	pos += sizeof(Mpeg2VideoEsStartCode) + sizeof(*header);
	return pos;	
}

PipePosition copyIFrame(Pipe pipe, FILE *file){
	
	Mpeg2VideoEsStartCode *ppp;
	u8 startcode;
	Boolean done;
	PipePosition pos =0;
	done = false; 
	ppp = malloc(sizeof(Mpeg2VideoEsStartCode));
	do {
		assert(pos+=copyTillNextStartCode(pipe,&ppp,file));
		startcode = ppp->packetStartCodeByte;	
		if ( startcode != MPEG2_VIDEO_EXTENSION_START_CODE &&
	       		startcode != MPEG2_VIDEO_USER_DATA_START_CODE &&
			(startcode >= MPEG2_VIDEO_SLICE_START_CODE_MIN && 
		 	startcode <= MPEG2_VIDEO_SLICE_START_CODE_MAX)
	       	){ 
			ppp = PIPE_GET(pipe, Mpeg2VideoEsStartCode) ;
			fwrite(ppp,1,sizeof(Mpeg2VideoEsStartCode),file);
			pos += sizeof(Mpeg2VideoEsStartCode);
		  }
		else
		 	done = true;


	 }while(done == false); 
	return pos ;
}

Mpeg2VideoEsStartCode* skipPictureFrame(Pipe pipe){
	
	Mpeg2VideoEsStartCode *ppp;
	u8 startcode;
	Boolean done;
	done = false; 
	//copy till the next header is not 
	do {
		assert(ppp = skipToNextStartCode(pipe));
		startcode = ppp->packetStartCodeByte;	

	}while( startcode != MPEG2_VIDEO_EXTENSION_START_CODE &&
	       		startcode != MPEG2_VIDEO_USER_DATA_START_CODE &&
			(startcode >= MPEG2_VIDEO_SLICE_START_CODE_MIN && 
		 	startcode <= MPEG2_VIDEO_SLICE_START_CODE_MAX)
	       	); 
	return ppp;
}

*/

//static RetCode
//mpeg2VideoEsParse(void *instp, MmpContextObject *cop, Pipe pipe){
//
//  Mpeg2VideoEs               mpeg2VideoEs = (Mpeg2VideoEs)instp ;
//  Mpeg2VideoEsInfo           info ;
//  Mpeg2VideoEsInfo          *infop = &info ;
//  Mpeg2VideoEsStartCode     *ppp ;
//  int                        seqHdrflag = 0, len, getlen;
//  u8                        *buf ;
//  RetCode                    retcode = RETCODE_SUCCESS ;
//  unsigned                   pid ;
//  Boolean                    exitwhile = FALSE;
//  unsigned                   startcode ;
//  PipePosition               pos, pos2;
//  u32                        curlen = 0;
//  u8	      *tmp;
//  tmp = malloc(sizeof(int)*40);
//
//   mpeg2VideoEs->isDirty = TRUE ;
//   
//   pid = MMP_CONTEXT_PIPETOPID(cop, pipe);
//
//   /* if(pid != MMP_PID_NULL){
//      pipeFindMark(pipe) ;
//      }
//      */
// file12 = fopen ("output.es","w");
//
//   findStartCodeprefix(pipe);
//
//   //UT_LOG_MESG(&logp, (&logp, " mpeg2VideoEsParse : video Parser called \n"));
// 
//   mpeg2VideoEs->curptr = mpeg2VideoEs->bufDataptr;
//
//   *(&(mpeg2VideoEs->noVideo)) = FALSE;
//
//   while(pipeIsAvail(pipe, 1)){
//       if(pipeIsAtMark(pipe)){
//         buf = pipeGet(pipe, 1) ;
//         len = 1;
//         memcpy(mpeg2VideoEs->curptr, buf, len);
//         (*(mpeg2VideoEs->bufDatasize))++;
//         mpeg2VideoEs->curptr++;
//	   printf("Position in the stream is %x", pos);
//         /*mpeg2VideoEs->bufDataptr++;*/
//
//         /*if(len != write(mpeg2VideoEs->filep, buf, len)){
//	   UT_LOG_MESG(&logp, (&logp, "mpeg2VideoEsparse : Write failed \n")) ;
//  	   return RETCODE_FAILED ;
//         }*/
//       }else{
//         len = pipeAvailUnmarked(pipe) ;
//         /* Get only chunks of data,  so only a small buffer needs to be allocated */
//         while(len > 0){
//	   if(len > MPEG2_VIDEOES_MAX_CHUNK){
//	     getlen = MPEG2_VIDEOES_MAX_CHUNK ;
//	     len -= MPEG2_VIDEOES_MAX_CHUNK ;
//	   }else{
//	     getlen = len ;
//	     len = 0;
//	   }
//	   pos = pipePosition(pipe);
//	   buf = pipeGet(pipe, getlen) ;
// 	   
//           if((*(mpeg2VideoEs->bufDatasize))+getlen < 
//              (*(mpeg2VideoEs->bufLimit)) ){
//             memcpy(mpeg2VideoEs->curptr, buf, getlen);
//             (*(mpeg2VideoEs->bufDatasize)) += getlen;
//             mpeg2VideoEs->curptr += getlen;
//             /*mpeg2VideoEs->bufDataptr += getlen;*/
//           }
//           else{
//	     /* the output buf is full, copy some data into buf */
//             curlen = (*(mpeg2VideoEs->bufLimit)) - (*(mpeg2VideoEs->bufDatasize));
//             memcpy(mpeg2VideoEs->curptr, buf, curlen);
//             (*(mpeg2VideoEs->bufDatasize)) += curlen;
//             mpeg2VideoEs->curptr += curlen;
//	     fwrite(mpeg2VideoEs->bufBase,1,*(mpeg2VideoEs->bufDatasize),file12 );
//             /*mpeg2VideoEs->bufDataptr += curlen;*/
//
//             /* wait for video decoder to read filledBuf*/
//	     fprintf( stderr, "video_es: in While: posting sema \n");
//            //sema_post(&(mpeg2VideoEs->semaBufFilled));
//            //sema_wait(&(mpeg2VideoEs->semaBufReleased));
//
//             /* copy the remaining data into buf */
//	     mpeg2VideoEs->curptr = mpeg2VideoEs->bufDataptr; //remove this just for testing
//	     *(mpeg2VideoEs->bufDatasize) = 0;
//             memcpy(mpeg2VideoEs->curptr, buf+curlen, getlen-curlen);
//             (*(mpeg2VideoEs->bufDatasize)) += getlen-curlen;
//             mpeg2VideoEs->curptr += getlen-curlen;
//
//             /*mpeg2VideoEs->bufDataptr += getlen-curlen;*/
//	   }
//	 }
//       }
//   }
//fwrite(mpeg2VideoEs->bufBase,1,*(mpeg2VideoEs->bufDatasize),file12 );
//fclose(file12);
////fprintf( stderr, " video_es : posting sema outside while \n");
//  // sema_post(&(mpeg2VideoEs->semaBufFilled));
//
//   /*   if(mpeg2VideoEs->parseDone){
//     sema_post(&(mpeg2VideoEs->semaBufFilled));
//     sema_wait(&(mpeg2VideoEs->semaBufReleased));
//   }*/
//
//   /*sema_wait(&(mpeg2VideoEs->semaBufReleased));*/
//
//   return RETCODE_SUCCESS ;
//}
 

