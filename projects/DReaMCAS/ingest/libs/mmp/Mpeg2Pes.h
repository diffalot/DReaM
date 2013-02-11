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
 * $(@)Mpeg2Pes.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2Pes.h
 * 
 * Mpeg2Pes.h is the interface to information gleaned by parsing MPEG2 PES
 * streams.
 */

#ifndef	_MMP_MPEG2PES_H
#define	_MMP_MPEG2PES_H

#pragma ident "@(#)Mpeg2Pes.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <values.h>

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Pipe.h"
#include "mmp/Mpeg2Ps.h"

_FOREIGN_START

/***********************************************************************
 * Mpeg2 PES Types
 ***********************************************************************/

typedef struct Mpeg2PesPacket {
    BIT1(packetStartCodePrefix23_16:8);
    BIT1(packetStartCodePrefix15_8:8);
    BIT1(packetStartCodePrefix7_0:8);
    BIT1(streamId:8);
    BIT1(pesPacketLength15_8:8);
    BIT1(pesPacketLength7_0:8);
    /*
     * streamId indicates which of the following structs are included in this
     * packet
     */
} Mpeg2PesPacket;

#define	MPEG2_PES_PACKET_START_CODE_PREFIX(p)				\
	(   ( ((p)->packetStartCodePrefix23_16) << 16 )			\
	  | ( ((p)->packetStartCodePrefix15_8) << 8 )			\
	  | ( (p)->packetStartCodePrefix7_0)				\
	)

#define	IS_MPEG2_PES_PACKET_START_CODE_PREFIX(p)			\
	(MPEG2_PES_PACKET_START_CODE_PREFIX(p) == 0x000001)

#define	MPEG2_PES_STREAM_ID_MIN				0xbc
#define	MPEG2_PES_STREAM_ID_MAX				0xff

#define	MPEG2_PES_STREAM_ID_TYPE(sid)			((sid) & 0xf0)

#define	MPEG2_PES_STREAM_ID_TYPE_SPECIAL_1		0xb0
#define	MPEG2_PES_STREAM_ID_TYPE_AUDIO_1		0xc0
#define	MPEG2_PES_STREAM_ID_TYPE_AUDIO_2		0xd0
#define	MPEG2_PES_STREAM_ID_TYPE_VIDEO			0xe0
#define	MPEG2_PES_STREAM_ID_TYPE_SPECIAL_2		0xf0

/*
 * MPEG2_PES_STREAM_ID_TYPE_SPEC_1 subtypes
 */
#define	MPEG2_PES_STREAM_ID_PROGRAM_STREAM_MAP		0xbc
#define	MPEG2_PES_STREAM_ID_PRIVATE_STREAM_1		0xbd
#define	MPEG2_PES_STREAM_ID_PADDING_STREAM		0xbe
#define	MPEG2_PES_STREAM_ID_PRIVATE_STREAM_2		0xbf

/*
 * MPEG2_PES_STREAM_ID_TYPE_SPEC_2 subtypes
 */
#define	MPEG2_PES_STREAM_ID_ECM_STREAM			0xf0
#define	MPEG2_PES_STREAM_ID_EMM_STREAM			0xf1
#define	MPEG2_PES_STREAM_ID_DSM_CC_STREAM		0xf2
#define	MPEG2_PES_STREAM_ID_ISO_IEC_13522_STREAM	0xf3
#define	MPEG2_PES_STREAM_ID_PROGRAM_STREAM_DIRECTORY	0xff

#define	MPEG2_PES_PACKET_LENGTH(p)				\
	(   ((p)->pesPacketLength15_8 << 8)			\
	  |  (p)->pesPacketLength7_0				\
	)

typedef struct Mpeg2PesPacketFlags {
    BIT6(b10:2,				   /* '10' */
	 pesScramblingControl:2,
	 pesPriority:1,
	 dataAlignmentIndicator:1,
	 copyright:1,
	 originalOrCopy:1);
    BIT7(ptsDtsFlags:2,
	 escrFlag:1,
	 esRateFlag:1,
	 dsmTrickModeFlag:1,
	 additionalCopyInfoFlag:1,
	 pesCrcFlag:1,
	 pesExtensionFlag:1);
    BIT1(pesHeaderDataLength:8);
} Mpeg2PesPacketFlags;

typedef struct Mpeg2PesTs {
    BIT3(tsType:4,			   /* '0010' or '0011' */
	 ts32_30:3,
	 markerBit_1:1);
    BIT1(ts29_22:8);
    BIT2(ts21_15:7,
	 markerBit_2:1);
    BIT1(ts14_7:8);
    BIT2(ts6_0:7,
	 markerBit_3:1);
} Mpeg2PesTs;

#define	MPEG2_PES_TS(p)						\
	(   ((u64)((p)->ts32_30) << 30) 			\
	  |       ((p)->ts29_22 << 22)	 			\
	  |       ((p)->ts21_15 << 15) 				\
	  |       ((p)->ts14_7 << 7) 				\
	  |        (p)->ts6_0					\
	)

typedef struct Mpeg2PesEscr {
    BIT4(reserved:2,
	 escrBase32_30:3,
	 markerBit_1:1,
	 escrBase29_28:2);
    BIT1(escrBase27_20:8);
    BIT3(escrBase19_15:5,
	 markerBit_2:1,
	 escrBase14_13:2);
    BIT1(escrBase12_5:8);
    BIT3(escrBase4_0:5,
	 markerBit_3:1,
	 escrExtension8_7:2);
    BIT2(escrExtension6_0:7,
	 markerBit_4:1);
} Mpeg2PesEscr;

#define MPEG2_PES_ESCR(p) 					\
	(   (  ((u64)((p)->escrBase32_30) << 30)		\
              |      ((p)->escrBase29_28 << 28)			\
              |      ((p)->escrBase27_20 << 20)			\
              |      ((p)->escrBase19_15 << 15)			\
              |      ((p)->escrBase14_13 << 13)			\
              |      ((p)->escrBase12_5 << 5)			\
              |       (p)->escrBase4_0				\
            ) * 300						\
          + (        ((p)->escrExtension8_7 << 7)		\
              |       (p)->escrExtension6_0			\
            )							\
	)

typedef struct Mpeg2PesEsRate {
    BIT2(markerBit_1:1,
	 esRate21_15:7);
    BIT1(esRate14_7:8);
    BIT2(esRate6_0:7,
	 markerBit_2:1);
} Mpeg2PesEsRate;

#define	MPEG2_PES_ES_RATE(p)					\
	(   ((p)->esRate21_15 << 15)				\
	  | ((p)->esRate14_7 << 7)				\
	  |  (p)->esRate6_0					\
	)

typedef struct Mpeg2PesDsmTrickMode {
    union {
	struct Mpeg2PesTrickModeControl {
	    BIT2(trickModeControl:3,
		 reserved:5);
	}                   modeControl;
	struct Mpeg2PesTrickMode000 {
	    BIT4(trickModeControl:3,
		 fieldId:2,
		 intraSliceRefresh:1,
		 frequencyTruncation:2);
	}                   mode000;
	struct Mpeg2PesTrickMode001 {
	    BIT2(trickModeControl:3,
		 fieldRepCntrl:5);
	}                   mode001;
	struct Mpeg2PesTrickMode010 {
	    BIT3(trickModeControl:3,
		 fieldId:2,
		 reserved:3);
	}                   mode010;
	struct Mpeg2PesTrickMode011 {
	    BIT4(trickModeControl:3,
		 fieldId:2,
		 intraSliceRefresh:1,
		 frequencyTruncation:2);
	}                   mode011;
	struct Mpeg2PesTrickMode100 {
	    BIT2(trickModeControl:3,
		 fieldRepCntrl:5);
	}                   mode100;
    }                   un;
} Mpeg2PesDsmTrickMode;

typedef struct Mpeg2PesAdditionalCopyInfo {
    BIT2(markerBit:1,
	 additionalCopyInfo:7);
} Mpeg2PesAdditionalCopyInfo;

typedef struct Mpeg2PesCrc {
    BIT1(previousPesPacketCrc15_8:8);
    BIT1(previousPesPacketCrc7_0:8);
} Mpeg2PesPesCrc;

#define	MPEG2_PES_PREVIOUS_PES_PACKET_CRC(p) 			\
	(   ((p)->previousPesPacketCrc15_8 << 8)		\
	  |  (p)->previousPesPacketCrc7_0			\
	)

typedef struct Mpeg2PesExtension {
    BIT6(pesPrivateDataFlag:1,
	 packHeaderFieldFlag:1,
	 programPacketSequenceCounterFlag:1,
	 pStdBufferFlag:1,
	 reserved:3,
	 pesExtensionFlag2:1);
} Mpeg2PesPesExtension;

typedef struct Mpeg2PesPrivateData {
    u8                  pesPrivateDate[128 / BITSPERBYTE];
} Mpeg2PesPrivateData;

typedef struct Mpeg2PesPackHeaderField {
    BIT1(packFieldLength:8);
    Mpeg2PsPackHeader   packHeader;
} Mpeg2PesPackHeaderField;

typedef struct Mpeg2PesProgramPacketSequenceCounter {
    BIT2(markerBit_1:1,
	 programPacketSequenceCounter:7);
    BIT3(markerBit_2:1,
	 mpeg1Mpeg2Identifier:1,
	 originalStuffLength:6);
} Mpeg2PesProgramPacketSequenceCounter;

typedef struct Mpeg2PesPStdBuffer {
    BIT3(b01:2,
	 pStdBufferScale:1,
	 pStdBufferSize12_8:5);
    BIT1(pStdBufferSize7_0:8);
} Mpeg2PesPStdBuffer;

#define	MPEG2_PES_STDBUFFER_SIZE(p) 					\
	(   ((p)->pStdBufferSize12_8 << 8)				\
	  |  (p)->pStdBufferSize7_0					\
	)

typedef struct Mpeg2PesExtension2 {
    BIT2(markerBit_1:1,
	 pesExtensionFieldLength:7);
} Mpeg2PesExtension2;

#define	MPEG2_PES_PACKET_STUFF_BYTE	0xff
#define	MPEG2_PES_PACKET_PADDING_BYTE	0xff

/***********************************************************************
 * Mpeg2 PES Callback Info Types
 ***********************************************************************/

typedef struct Mpeg2PesInfo {
    PipePosition        position;

    /* packet */
    Mpeg2PesPacket      packet;
    Mpeg2PesPacketFlags packetFlags;

    /*
     * Fields below are only valid if flagged in packetFlags
     */
    Mpeg2PesTs          pts;
    Mpeg2PesTs          dts;
    Mpeg2PesEscr        escr;
    Mpeg2PesEsRate      esRate;
    Mpeg2PesDsmTrickMode dsmTrickMode;
    Mpeg2PesAdditionalCopyInfo additionalCopyInfo;
    Mpeg2PesPesCrc      pesCrc;
    Mpeg2PesPesExtension pesExtension;

    /*
     * Fields below are only valid if flagged in pesExtension
     */
    Mpeg2PesPrivateData pesPrivateData;
    Mpeg2PesPackHeaderField packHeaderField;
    Mpeg2PesProgramPacketSequenceCounter programPacketSequenceCounter;
    Mpeg2PesPStdBuffer  pStdBuffer;
    Mpeg2PesExtension2  pesExtension2;

    /*
     * Fields below are only valid if pesExtension2 has non-zero field length
     */
    u8                  pesExtension2Data[128];
} Mpeg2PesInfo;

typedef RetCode     (*Mpeg2PesCallBack) (void *token, Mpeg2PesInfo *infop,
				                           RetCode retCode);

/***********************************************************************
 * Mpeg2 PES Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2PES_CLASSNAME, Mpeg2PesError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2PES_CLASSNAME		"Mpeg2Pes"

typedef enum Mpeg2PesError {
    MPEG2_PES_ERROR_SYNTAX = 1,		   /* packet syntax error */
    MPEG2_PES_ERROR_PTS,		   /* pts flag error */
    MPEG2_PES_ERROR_DTS,		   /* dts flag error */
    MPEG2_PES_ERROR_HDR_LEN,		   /* header length error */
    MPEG2_PES_ERROR_LEN			   /* packet length error */
} Mpeg2PesError;

/***********************************************************************
 * Mpeg2 PES Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Pes *Mpeg2Pes;

/***********************************************************************
 * Mpeg2 PES Class Interface
 ***********************************************************************/

extern Mpeg2Pes     mpeg2PesNew(void);

/***********************************************************************
 * Mpeg2 PES Instance Interface
 ***********************************************************************/

extern void
mpeg2PesSetEsParsers(Mpeg2Pes mpeg2Pes,
	       MmpParserObject *streamIdToPop[MPEG2_PES_STREAM_ID_MAX + 1]);

extern MmpParserObject *mpeg2PesParserObject(Mpeg2Pes mpeg2Pes);

extern void
mpeg2PesSetCallBack(Mpeg2Pes mpeg2Pes, Mpeg2PesCallBack callBack,
                    void *token);

/*
 * doFlush == TRUE => parse payload "synchronously"
 */
extern void mpeg2PesSetFlush(Mpeg2Pes mpeg2Pes, Boolean doFlush);

extern void         mpeg2PesFree(Mpeg2Pes mpeg2Pes);

_FOREIGN_END

#endif					   /* _MMP_MPEG2PES_H */
