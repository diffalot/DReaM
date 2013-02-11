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
 * $(@)Mpeg1Pes.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg1Pes.h
 * 
 * Mpeg1Pes.h is the interface to information gleaned by parsing MPEG1 PES
 * streams.
 */

#ifndef	_MMP_MPEG1PES_H
#define	_MMP_MPEG1PES_H

#pragma ident "@(#)Mpeg1Pes.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Mpeg1 PES Types
 ***********************************************************************/

typedef struct Mpeg1PesPacket {
    BIT1(packetStartCodePrefix23_16:8);
    BIT1(packetStartCodePrefix15_8:8);
    BIT1(packetStartCodePrefix7_0:8);
    BIT1(streamId:8);
    BIT1(packetLength15_8:8);
    BIT1(packetLength7_0:8);
    /*
     * This is optionally followed by: stuffing bytes stdBuffer info
     * presentation time stamp decoding time stamp And the followed by packet
     * data bytes.
     */
} Mpeg1PesPacket;

#define	MPEG1_PES_PACKET_START_CODE_PREFIX(p)				\
	(   ( ((p)->packetStartCodePrefix23_16) << 16 )			\
	  | ( ((p)->packetStartCodePrefix15_8) << 8 )			\
	  | ( (p)->packetStartCodePrefix7_0)				\
	)

#define	IS_MPEG1_PES_PACKET_START_CODE_PREFIX(p)			\
	(MPEG1_PES_PACKET_START_CODE_PREFIX(p) == 0x000001)

#define	MPEG1_PES_STREAM_ID_MIN				0xbc
#define	MPEG1_PES_STREAM_ID_PRIVATE_STREAM_1		0xbd
#define	MPEG1_PES_STREAM_ID_PADDING_STREAM		0xbe
#define	MPEG1_PES_STREAM_ID_PRIVATE_STREAM_2		0xbf
#define	MPEG1_PES_STREAM_ID_MAX				0xff

#define	MPEG1_PES_STREAM_ID_TYPE(s)			((s) & 0xf0)

#define	MPEG1_PES_STREAM_ID_TYPE_SPECIAL		0xb0
#define	MPEG1_PES_STREAM_ID_TYPE_AUDIO_1		0xc0
#define	MPEG1_PES_STREAM_ID_TYPE_AUDIO_2		0xd0
#define	MPEG1_PES_STREAM_ID_TYPE_VIDEO			0xe0
#define	MPEG1_PES_STREAM_ID_TYPE_RESERVED_DATA_STREAM	0xf0

#define	MPEG1_PES_PACKET_LENGTH(p)				\
	( 							\
	    ((p)->packetLength15_8 << 8)			\
	  |  (p)->packetLength7_0				\
	)

#define	MPEG1_PES_STUFF_BYTE		0xff
#define	MPEG1_PES_END_BYTE		0x0f

typedef struct Mpeg1PesStdBuffer {
    BIT3(b01:2,
	 stdBufferScale:1,
	 stdBufferSize12_8:5);
    BIT1(stdBufferSize7_0:8);
} Mpeg1PesStdBuffer;

#define	MPEG1_PES_STD_BUFFER_SIZE(p)				\
	(							\
	    ((p)->stdBufferSize12_8 << 8)			\
	  |  (p)->stdBufferSize7_0				\
	)

typedef struct Mpeg1PesTimeStamp {
    BIT3(bflag:4,			   /* '0010' or '0011' or '0001' */
	 timeStamp32_30:3,
	 markerBit_1:1);
    BIT1(timeStamp29_22:8);
    BIT2(timeStamp21_15:7,
	 markerBit_2:1);
    BIT1(timeStamp14_7:8);
    BIT2(timeStamp6_0:7,
	 markerBit_3:1);
} Mpeg1PesTimeStamp;

#define	MPEG1_PES_TIME_STAMP(p)					\
	(   ((u64)((p)->timeStamp32_30) << 30) 			\
	  | ((p)->timeStamp29_22 << 22) 			\
	  | ((p)->timeStamp21_15 << 15) 			\
	  | ((p)->timeStamp14_7 << 7) 				\
	  |  (p)->timeStamp6_0					\
	)

/***********************************************************************
 * Mpeg1 PES Callback Info Types
 ***********************************************************************/

typedef struct Mpeg1PesInfo {
    PipePosition        position;

    /* packet */
    Mpeg1PesPacket      packet;

    Boolean             hasStdBuffer;
    Mpeg1PesStdBuffer   stdBuffer;

    Boolean             hasPresentationTimeStamp;
    Mpeg1PesTimeStamp   presentationTimeStamp;

    Boolean             hasDecodingTimeStamp;
    Mpeg1PesTimeStamp   decodingTimeStamp;

} Mpeg1PesInfo;

typedef RetCode     (*Mpeg1PesCallBack) (void *token, Mpeg1PesInfo *infop,
				                           RetCode retCode);

/***********************************************************************
 * Mpeg1 PES Error Codes
 *
 * NOTE: Use retCodeFor(MPEG1PES_CLASSNAME, Mpeg1PesError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG1PES_CLASSNAME	"Mpeg1Pes"

typedef enum Mpeg1PesError {
    MPEG1_PES_ERROR_SYNTAX = 1,		   /* packet syntax */
    MPEG1_PES_ERROR_LEN			   /* packet length mismatch */
} Mpeg1PesError;

/***********************************************************************
 * Mpeg1 PES Instance Type
 ***********************************************************************/

typedef struct _Mpeg1Pes *Mpeg1Pes;

/***********************************************************************
 * Mpeg1 PES Class Methods
 ***********************************************************************/

extern Mpeg1Pes     mpeg1PesNew(void);

/***********************************************************************
 * Mpeg1 PES Instance Methods
 ***********************************************************************/

/*
 * Declare elementary stream parsers for each stream id
 */
extern void
mpeg1PesSetEsParsers(Mpeg1Pes mpeg1Pes,
	       MmpParserObject *streamIdToPop[MPEG1_PES_STREAM_ID_MAX + 1]);

extern MmpParserObject *mpeg1PesParserObject(Mpeg1Pes mpeg1Pes);

extern void
mpeg1PesSetCallBack(Mpeg1Pes mpeg1Pes, Mpeg1PesCallBack callBack,
	            void *token);

/*
 * doFlush == TRUE => parse payload "synchronously"
 */
extern void         mpeg1PesSetFlush(Mpeg1Pes mpeg1Pes, Boolean doFlush);

extern void         mpeg1PesFree(Mpeg1Pes mpeg1Pes);

_FOREIGN_END

#endif					   /* _MMP_MPEG1PES_H */
