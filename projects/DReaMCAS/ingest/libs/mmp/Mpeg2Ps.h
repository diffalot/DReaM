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
 * $(@)Mpeg2Ps.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * OBJECT Mpeg2Ps.h -- Interface to mpeg2 program stream parser.
 */
#ifndef	_MMP_MPEG2PS_H
#define	_MMP_MPEG2PS_H

#pragma ident "@(#)Mpeg2Ps.h 1.1	96/09/18 SMI"

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
 * Mpeg2 Program Stream Types
 ***********************************************************************/

typedef struct Mpeg2PsPackHeader {
    BIT1(packStartCode31_24:8);
    BIT1(packStartCode23_16:8);
    BIT1(packStartCode15_8:8);
    BIT1(packStartCode7_0:8);
    BIT4(b01:2,
	 systemClockReferenceBase32_30:3,
	 markerBit_1:1,
	 systemClockReferenceBase29_28:2);
    BIT1(systemClockReferenceBase27_20:8);
    BIT3(systemClockReferenceBase19_15:5,
	 markerBit_2:1,
	 systemClockReferenceBase14_13:2);
    BIT1(systemClockReferenceBase12_5:8);
    BIT3(systemClockReferenceBase4_0:5,
	 markerBit_3:1,
	 systemClockReferenceExtension8_7:2);
    BIT2(systemClockReferenceExtension6_0:7,
	 markerBit_4:1);
    BIT1(programMuxRate21_14:8);
    BIT1(programMuxRate13_6:8);
    BIT3(programMuxRate5_0:6,
	 markerBit_5:1,
	 markerBit_6:1);
    BIT2(reserved:5,
	 packStuffingLength:3);
    /*
     * This is followed by packStuffingLength bytes of stuffing followed by
     * an optional systemHeader.
     */
} Mpeg2PsPackHeader;

typedef enum Mpeg2PsStartCodes {
    MPEG2_PS_START_CODE_END = 0x1b9,
    MPEG2_PS_START_CODE_PACK = 0x1ba,
    MPEG2_PS_START_CODE_SYSTEM = 0x1bb
} Mpeg2PsStartCodes;

#define	MPEG2_PS_PACK_HEADER_START_CODE(p)				\
	(   ((p)->packStartCode31_24 << 24)				\
	  | ((p)->packStartCode23_16 << 16)				\
	  | ((p)->packStartCode15_8 << 8)				\
	  |  (p)->packStartCode7_0					\
	)

#define MPEG2_PS_PACK_HEADER_SYSTEM_CLOCK_REFERENCE(p) 			\
	(  (  ((u64)((p)->systemClockReferenceBase32_30) << 30)		\
             |      ((p)->systemClockReferenceBase29_28 << 28)		\
             |      ((p)->systemClockReferenceBase27_20 << 20)		\
             |      ((p)->systemClockReferenceBase19_15 << 15)		\
             |      ((p)->systemClockReferenceBase14_13 << 13)		\
             |      ((p)->systemClockReferenceBase12_5 << 5)		\
             |      ((p)->systemClockReferenceBase4_0)			\
          ) * 300							\
        + (         ((p)->systemClockReferenceExtension8_7 << 7)	\
             |       (p)->systemClockReferenceExtension6_0		\
          )								\
       )

#define MPEG2_PS_PACK_HEADER_PROGRAM_MUX_RATE(p)   			\
	(   ((p)->programMuxRate21_14 << 14)				\
	  | ((p)->programMuxRate13_6 << 6)				\
	  |  (p)->programMuxRate5_0					\
 	)

#define	MPEG2_PS_STUFFING_BYTE		0xff

typedef struct Mpeg2PsSystemHeader {
    BIT1(systemHeaderStartCode31_24:8);
    BIT1(systemHeaderStartCode23_16:8);
    BIT1(systemHeaderStartCode15_8:8);
    BIT1(systemHeaderStartCode7_0:8);
    BIT1(headerLength15_8:8);
    BIT1(headerLength7_0:8);
    BIT2(markerBit_1:1,
	 rateBound21_15:7);
    BIT1(rateBound14_7:8);
    BIT2(rateBound6_0:7,
	 markerBit_2:1);
    BIT3(audioBound:6,
	 fixedFlag:1,
	 cspsFlag:1);
    BIT4(systemAudioLockFlag:1,
	 systemVideoLockFlag:1,
	 markerBit_3:1,
	 videoBound:5);
    BIT1(reservedByte:8);
} Mpeg2PsSystemHeader;

#define	MPEG2_PS_SYSTEM_HEADER_LENGTH(p)				\
	(   ((p)->headerLength15_8 << 8)				\
	  |  (p)->headerLength7_0					\
	)

#define	MPEG2_PS_SYSTEM_HEADER_RATE_BOUND(p)				\
	(   ((p)->rateBound21_15 << 15)					\
	  | ((p)->rateBound14_7 << 7) 					\
	  |  (p)->rateBound6_0						\
	)

typedef struct Mpeg2PsPStdBuffer {
    BIT1(streamId:8);
    BIT3(b11:2,
	 pStdBufferBoundScale:1,
	 pStdBufferSizeBound12_8:5);
    BIT1(pStdBufferSizeBound7_0:8);
} Mpeg2PsPStdBuffer;

#define	MPEG2_PS_P_STD_BUFFER_BUFFER_SIZE_BOUND(p)			\
	(   ((p)->pStdBufferSizeBound12_8 << 8)				\
	  |  (p)->pStdBufferSizeBound7_0				\
	)

/***********************************************************************
 * Mpeg2 Program Stream Callback Info Types
 ***********************************************************************/

#define	MPEG2_PS_MAX_P_STD_BUFFERS		16

typedef struct Mpeg2PsInfo {
    PipePosition        position;

    Mpeg2PsPackHeader   packHeader;

    Boolean             hasSystemHeader;
    Mpeg2PsSystemHeader systemHeader;

    u8                  nPStdBuffer;	   /* valid pStdBuffer entries */
    Mpeg2PsPStdBuffer   pStdBuffer[MPEG2_PS_MAX_P_STD_BUFFERS];
} Mpeg2PsInfo;

typedef RetCode     (*Mpeg2PsCallBack) (void *token, Mpeg2PsInfo *infop,
					                    RetCode retCode);


/***********************************************************************
 * Mpeg2 Program Stream Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2PS_CLASSNAME, Mpeg2PsError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2PS_CLASSNAME		"Mpeg2Ps"

typedef enum Mpeg2PsError {
    MPEG2_PS_ERROR_PACK_SYNTAX = 1,	   /* pack syntax error */
    MPEG2_PS_ERROR_END_CODE,		   /* missing or invalid end code */
    MPEG2_PS_ERROR_SYSTEM_HDR_LEN	   /* system header length error */
} Mpeg2PsError;

/***********************************************************************
 * Mpeg2Ps Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Ps *Mpeg2Ps;

/***********************************************************************
 * Mpeg2Ps Class Interface
 ***********************************************************************/

extern Mpeg2Ps      mpeg2PsNew(void);

/***********************************************************************
 * Mpeg2Ps Instance Interface
 ***********************************************************************/

extern void         mpeg2PsSetPesParser(Mpeg2Ps mpeg2Ps, MmpParserObject *pesp);

extern MmpParserObject *mpeg2PsParserObject(Mpeg2Ps mpeg2Ps);

extern void
mpeg2PsSetCallBack(Mpeg2Ps mpeg2Ps, Mpeg2PsCallBack callBack,
		   void *token);

extern void         mpeg2PsFree(Mpeg2Ps mpeg2Ps);

_FOREIGN_END

#endif					   /* _MMP_MPEG2PS_H */
