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
 * $(@)Mpeg2VideoEs.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/* This file would define the structures required for the Mpeg2 Video
Elementary Stream and also the declarations for the parser routines */

#ifndef _MMP_MPEG2VIDEOES_H
#define _MMP_MPEG2VIDEOES_H

#include <values.h>

#include <cobjs/Foreign.h>
#include <cobjs/Macros.h>
#include <cobjs/Types.h>
#include <cobjs/RetCode.h>
#include "mmp/Context.h"
#include <mmp/Pipe.h>
#include <mmp/Mpeg2Pes.h>
#include <stdio.h>
#include <synch.h>

_FOREIGN_START

#define MPEG2_VIDEOES_MAX_CHUNK (4 * 1024)

#define MPEG2_VIDEO_ES_CLK    90000
#define DEMUX_BUF_LEN    (1024 * 256)

typedef struct _Mpeg2VideoEs *Mpeg2VideoEs;

typedef struct Mpeg2VideoEsStartCode{
  BIT1(packetStartCodePrefix23_16:8) ;
  BIT1(packetStartCodePrefix15_8:8) ;
  BIT1(packetStartCodePrefix7_0:8) ;
  BIT1(packetStartCodeByte:8) ;
}Mpeg2VideoEsStartCode ;


#define MPEG2_VIDEO_ES_START_CODE_PREFIX(p)          \
      (  ( ((p)->packetStartCodePrefix23_16) << 16)  \
       | ( ((p)->packetStartCodePrefix15_8) << 8)    \
       | ( (p)->packetStartCodePrefix7_0)           \
      )

#define IS_MPEG2_VIDEO_ES_START_CODE_PREFIX(p)       \
     (MPEG2_VIDEO_ES_START_CODE_PREFIX(p) == 0x000001)

#define IS_MPEG2_VIDEO_I_FRAME(header)	\
	((header)->pictureCodingType == 0x1)

#define IS_PICTURE_START_CODE(p)	\
	((p) == MPEG2_VIDEO_PICTURE_START_CODE )

#define IS_IFRAME(p)	\
	((p)->pictureCodingType == INTRA_CODED_TYPE)

#define IS_PFRAME(p)	\
	((p)->pictureCodingType == PREDICTIVE_CODED_TYPE)

#define IS_BFRAME(p)	\
	((p)->pictureCodingType ==BIDIRECTIONAL_CODED_TYPE )
	
/* start code values */

#define MPEG2_VIDEO_PICTURE_START_CODE     0x00
#define MPEG2_VIDEO_USER_DATA_START_CODE   0xb2
#define MPEG2_VIDEO_SEQUENCE_HEADER_CODE   0xb3
#define MPEG2_VIDEO_SEQUENCE_ERROR_CODE    0xb4
#define MPEG2_VIDEO_EXTENSION_START_CODE   0xb5
#define MPEG2_VIDEO_SEQUENCE_END_CODE      0xb7
#define MPEG2_VIDEO_GROUP_START_CODE       0xb8
#define MPEG2_VIDEO_SLICE_START_CODE_MIN   0x01
#define MPEG2_VIDEO_SLICE_START_CODE_MAX   0xaf
/* extension start code identifiers */

#define MPEG2_VIDEO_SEQUENCE_EXTENSION_ID                   0x01
#define MPEG2_VIDEO_SEQUENCE_DISPLAY_EXTENSION_ID           0x02
#define MPEG2_VIDEO_QUANT_MATRIX_EXTENSION_ID               0x03
#define MPEG2_VIDEO_COPYRIGHT_EXTENSION_ID                  0x04
#define MPEG2_VIDEO_SEQUENCE_SCALABLE_EXTENSION_ID          0x05
#define MPEG2_VIDEO_PICTURE_DISPLAY_EXTENSION_ID            0x07
#define MPEG2_VIDEO_PICTURE_CODING_EXTENSION_ID             0x08
#define MPEG2_VIDEO_PICTURE_SPATIAL_SCALABLE_EXTENSION_ID   0x09
#define MPEG2_VIDEO_PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID  0x0a

/* picture type macros */
#define INTRA_CODED_TYPE           0x01
#define PREDICTIVE_CODED_TYPE      0x02
#define BIDIRECTIONAL_CODED_TYPE   0x03
#define UNKNOWN		           0x04   /*error codition */

/* picture structure type */
#define TOP_FIELD           1
#define BOTTOM_FIELD        2
#define FRAME_PICTURE       3
#define MPEG2VIDEO_ES_CLASSNAME  "MPEG2VideoEs"

#define true	1
#define false	0
/* Definition of the Mpeg2 Video sequence Header as given in 13818-2 */
typedef struct Mpeg2VideoSequenceHdr{
  BIT1(horizontalSizeValue11_4:8) ;
  BIT2(horizontalSizeValue3_0:4,
       verticalSizeValue11_8:4) ;
  BIT1(verticalSizeValue7_0:8) ;
  BIT2(apectRatioInformation:4,
       frameRateCode:4);
  BIT1(bitRateValue17_10:8);
  BIT1(bitRateValue9_2:8);
  BIT3(bitRateValue1_0:2,
       markerBit:1,
       vbvBufferSizeValue9_5:5);
  BIT4(vbvBufferSizeValue4_0:5,
       constrainedParametersFlag:1,
       loadIntraQuantiserMatrix:1,
       optLoadNonIntraQuantiserMatrix:1);   /* This bit could be a intra 
                                              Quantizer bit or non intra flag*/
}Mpeg2VideoSequenceHdr ;


/* Definition of the loadable Quantiser Matrix */

typedef struct Mpeg2VideoIntraQuantiserMatrix{
  u8 intraQuantiserMatrix[64] ;
} Mpeg2VideoIntraQuantiserMatrix;


/* Definition of the non intra Quantiser Matrix */
typedef struct Mpeg2VideoNonIntraQuantiserMatrix{
  u8 nonIntraQuantizerMatrix[64] ;
} Mpeg2VideoNonIntraQuantiserMatrix;


/* Definition of the Sequence Extension Header */
typedef struct Mpeg2VideoSequenceExtHdr{
  BIT2(extStartCodeIdentifer:4,
      profileAndLevelIndication7_4:4);
  BIT4(profileAndLevelIndication3_0:4,
       progressiveSequence:1,
       chromaFormat:2,
       horizontalSizeExtension1:1) ;
  BIT3(horizontalSizeExtension0:1,
       verticalSizeExtension:2,
       bitRateExtension11_7:5) ;
  BIT2(bitRateExtension6_0:7,
       markerBit:1);
  BIT1(vbvBufferSizeExtension:8) ;
  BIT3(lowDelay:1,
       frameRateExtensionN:2,
       frameRateExtensionD:5);
}Mpeg2VideoSequenceExtHdr ;


#define MPEG2_VIDEO_SEQUENCE_HEADER_LEN  \
                      (sizeof(Mpeg2VideoSequenceExtHdr) + sizeof(Mpeg2VideoSequenceHdr))

#define MPEG2_VIDEO_BIT_RATE(p)       \
     (((p)->bitRateValue17_10 << 10) | ((p)->bitRateValue9_2 << 2) \
      | ((p)->bitRateValue1_0))

#define MPEG2_VIDEO_BIT_RATE_EXT(p)       \
     (((p)->bitRateExtension11_7 << 7) | ((p)->bitRateExtension6_0))

#define MPEG2_VIDEO_PROFILE(p) ((p)->profileAndLevelIndication7_4 & 0x7) 

#define MPEG2_VIDEO_LEVEL(p) (p)->profileAndLevelIndication3_0

#define BIT_RATE_MULT    400.0

/* Definition of the GOP Header */
typedef struct Mpeg2VideoGroupHeader{
  BIT1(timeCode24_17:8) ;
  BIT1(timeCode16_9:8) ;
  BIT1(timeCode8_1:8) ;
  BIT4(timeCode0:1,
       closedGop:1,
       brokenLink:1,
       byteAlignStuffingBits:5) ;   /* The next startcode would not start 
				       till the next byte */
}Mpeg2VideoGroupHeader ;

#define MPEG2_GOP_HEADER_LEN  sizeof(Mpeg2VideoGroupHeader)


/* Definition of the Partial Picture Header */
typedef struct Mpeg2VideoPictureHeader{
  BIT1(temporalReference9_2:8) ;
  BIT3(temporalReference1_0:2,
       pictureCodingType:3,
       vbvDelay15_13:3);
  BIT1(vbvDelay12_5:8) ;
  BIT2(vbvDelay4_0:5,
       other:3);
}Mpeg2VideoPictureHeader ;


#define MPEG2_VIDEO_PIC_TEMP_REF(p)   \
        (((p)->temporalReference9_2 << 2) |    \
	 ((p)->temporalReference1_0 ))

#define MPEG2_VIDEO_PIC_VBV_DELAY(p)            \
        (((p)->vbvDelay15_13 << 13) | ((p)->vbvDelay12_5  << 5) | \
	 ((p)->vbvDelay4_0))


/* Definition of partial Picture Coding Extension header */
typedef struct Mpeg2VideoPictureExtHeader{
  BIT2(extStartCodeIdentifier:4,
  fcode00:4) ;
  BIT2(fcode01:4,
      fcode10:4);
  BIT3(fcode11:4,
       intraDcPrecision:2,
       picStructure:2);
  BIT8(topFieldFirst:1,
       framePredFrameDct:1,
       concealmentMotionVectors:1,
       qScaleType:1,
       intraVlcFormat:1,
       alternateScan:1,
       repeatFirstField:1,
       chroma420Type:1) ;
}Mpeg2VideoPictureExtHeader ;


typedef struct Mpeg2VideoEsInfo{
  unsigned                           pid;
  double                             frameRate ;
  Boolean                            seqHdrFlag;
  Boolean                            gopHdrFlag;
  Boolean                            picHdrFlag;
  PipePosition                       position;
  u32                                bufferFullness ;
  Mpeg2VideoSequenceHdr              mpeg2VideoEsSeqHdr;
  Mpeg2VideoSequenceExtHdr           mpeg2VideoEsSeqExtHdr;
  Mpeg2VideoIntraQuantiserMatrix     mpeg2VideoIntraQuantiserMatrix;
  Mpeg2VideoNonIntraQuantiserMatrix  mpeg2VideoNonIntraQuantiserMatrix;
  Mpeg2VideoGroupHeader              mpeg2VideoGroupHdr;
  Mpeg2VideoPictureHeader            mpeg2VideoPictureHeader;
  Mpeg2VideoPictureExtHeader         mpeg2VideoPictureExtHeader ;
}Mpeg2VideoEsInfo;


#define SET_SEQUENCE_HEADER_FLAG(infop)  infop->seqHdrFlag = TRUE
#define SET_GROUP_HEADER_FLAG(infop)     infop->gopHdrFlag = TRUE
#define SET_PICTURE_HEADER_FLAG(infop)   infop->picHdrFlag = TRUE
#define IS_SEQUENCE_HEADER_FLAG(infop)   infop->seqHdrFlag
#define IS_GOP_HEADER_FLAG(infop)        infop->gopHdrFlag
#define IS_PICTURE_HEADER_FLAG(infop)    infop->picHdrFlag

typedef RetCode (*Mpeg2VideoEsCallBack)(void *token, Mpeg2VideoEsInfo *infop, RetCode retcode);

struct _Mpeg2VideoEs{
  Mpeg2VideoEsCallBack   callBack;
  void              *token;
  MmpParserObject    po;
  Boolean            isDirty;
  Boolean            doFlush ; 
  int                filep ;
  u8                 *bufBase;
  u8                 *bufDataptr;
  u32                *bufLimit;
  u32                *bufDatasize;
  u8                 *curptr;
  sema_t             semaBufFilled;
  sema_t             semaBufReleased;
  Boolean            parseDone;
  Boolean            noVideo ;
  Context            context;
  MmpContextObject	   *cop;	
  char                 *resourceName;
  char                 *metaName;
};

Mpeg2VideoEs mpeg2VideoEsNew(void);

void mpeg2VideoEsFree(Mpeg2VideoEs);

void mpeg2VideoEsSetCallBack(Mpeg2VideoEs,  Mpeg2VideoEsCallBack, void *);

extern MmpParserObject *mpeg2VideoEsParserObject(Mpeg2VideoEs mpeg2VideoEs);

void copyIFrame(Pipe pipe, FILE *file);

Boolean create_1X_trickplay_stream(Pipe pipe, char *resource_name, char*
meta_file_name );

Mpeg2VideoEsStartCode * copyTillNextStartCode(Pipe pipe, FILE *file);

Mpeg2VideoEsStartCode *skipToNextStartCode(Pipe pipe);

Mpeg2VideoEsStartCode* skipPictureFrame(Pipe pipe);

void copy_picture_header(Mpeg2VideoEsStartCode *ppp,Mpeg2VideoPictureHeader *header, FILE *file);

static RetCode mpeg2VideoEsRecognize(void *instp, Pipe pipe) ;


_FOREIGN_END

#endif /* _MMP_MPEG2VIDEOES_H */
