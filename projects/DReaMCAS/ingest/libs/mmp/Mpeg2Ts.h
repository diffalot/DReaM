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
 * $(@)Mpeg2Ts.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2Ts -- Interface to mpeg2 transport stream parser.
 */
#ifndef	_MMP_MPEG2TS_H
#define	_MMP_MPEG2TS_H

#pragma ident "@(#)Mpeg2Ts.h 1.1	96/09/18 SMI"

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
 * Defines
 ***********************************************************************/

#define	MPEG2_TS_SYNC_BYTE	0x47
#define	MPEG2_TS_PKT_SIZE	188

#define	MPEG2_TS_PAT_PID	0
#define	MPEG2_TS_CAT_PID	1
#define	MPEG2_TS_NULL_PID	0x1fff

#define	MPEG2_TS_STUFF_BYTE	0xff

/***********************************************************************
 * Mpeg2 Transport Types
 ***********************************************************************/

typedef struct Mpeg2TsTransportPacket {
    BIT1(syncByte:8);
    BIT4(transportErrorIndicator:1,
	 payloadUnitStartIndicator:1,
	 transportPriority:1,
	 pid12_8:5);
    BIT1(pid7_0:8);
    BIT3(transportScramblingControl:2,
	 adaptationFieldControl:2,
	 continuityCounter:4);
} Mpeg2TsTransportPacket;

#define MPEG2_TS_TRANSPORT_PACKET_PID(p)			\
	(   ((p)->pid12_8 << 8)					\
	  |  (p)->pid7_0					\
	)

typedef enum Mpeg2TsAdaptationFieldControl {
    MPEG2_TS_ADAPTATION_FIELD_CONTROL_RESERVED = 0x0,
    MPEG2_TS_ADAPTATION_FIELD_CONTROL_PAYLOAD_ONLY = 0x1,
    MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_ONLY = 0x2,
    MPEG2_TS_ADAPTATION_FIELD_CONTROL_ADAPTATION_PAYLOAD = 0x3
} Mpeg2TsAdaptationFieldControl;

typedef struct Mpeg2TsAdaptationField {
    BIT1(adaptationFieldLength:8);
    BIT8(discontinuityIndicator:1,
	 randomAccessIndicator:1,
	 elementaryStreamPriorityIndicator:1,
	 pcrFlag:1,
	 opcrFlag:1,
	 splicingPointFlag:1,
	 transportPrivateDataFlag:1,
	 adaptationFieldExtensionFlag:1);
} Mpeg2TsAdaptationField;

typedef struct Mpeg2TsProgramClockReference {
    BIT1(programClockReference32_25:8);
    BIT1(programClockReference24_17:8);
    BIT1(programClockReference16_9:8);
    BIT1(programClockReference8_1:8);
    BIT3(programClockReference0:1,
	 reserved:6,
	 programClockReferenceExtension8:1);
    BIT1(programClockReferenceExtension7_0:8);
} Mpeg2TsProgramClockReference;

#define MPEG2_TS_PROGRAM_CLOCK_REFERENCE(p) 				\
	(   (  ((u64)((p)->programClockReference32_25) << 25)		\
	      | ((p)->programClockReference24_17 << 17)			\
	      | ((p)->programClockReference16_9 << 9)			\
	      | ((p)->programClockReference8_1 << 1)			\
	      |  (p)->programClockReference0				\
	    ) * 300							\
	  + (  ((p)->programClockReferenceExtension8 << 8)		\
	      | ((p)->programClockReferenceExtension7_0)		\
	    )								\
	)

typedef struct Mpeg2TsAdaptationFieldExtension {
    BIT1(adaptationFieldExtensionLength:8);
    BIT4(ltwFlag:1,
	 piecewiseRateFlag:1,
	 seamlessSpliceFlag:1,
	 reserved:5);
} Mpeg2TsAdaptationFieldExtension;

typedef struct Mpeg2TsLtw {
    BIT2(ltwValidFlag:1,
	 ltwOffset14_8:7);
    BIT1(ltwOffset7_0:8);
} Mpeg2TsLtw;

#define MPEG2_TS_LTW_LTW_OFFSET(p)				\
	(   ((p)->ltwOffset14_8 << 8)				\
	  |  (p)->ltwOffset7_0					\
	)

typedef struct Mpeg2TsPiecewiseRate {
    BIT2(reserved:2,
	 piecewiseRate21_16:6);
    BIT1(piecewiseRate15_8:8);
    BIT1(piecewiseRate7_0:8);
} Mpeg2TsPiecewiseRate;

#define MPEG2_TS_PIECEWISE_RATE(p)   				\
	(   ((p)->piecewiseRate21_16 << 16)			\
	  | ((p)->piecewiseRate15_8 << 8)			\
	  |  (p)->piecewiseRate7_0				\
	)

typedef struct Mpeg2TsSeamlessSplice {
    BIT3(spliceType:4,
	 dtsNextAu32_30:3,
	 markerBit_1:1);
    BIT1(dtsNextAu29_22:8);
    BIT2(dtsNextAu21_15:7,
	 markerBit_2:1);
    BIT1(dtsNextAu14_7:8);
    BIT2(dtsNextAu6_0:7,
	 markerBit_3:1);
} Mpeg2TsSeamlessSplice;

#define MPEG2_TS_SEAMLESS_SPLICE_DTS_NEXT_AU(p)   			\
	(  ((u64)((p)->dtsNextAu32_30) << 30)	\
	  |      ((p)->dtsNextAu29_22 << 22)	\
	  |      ((p)->dtsNextAu21_15 << 15)	\
	  |      ((p)->dtsNextAu14_7 << 7)	\
	  |       (p)->dtsNextAu6_0		\
	)

/***********************************************************************
 * Mpeg2 Transport Callback Info Type
 ***********************************************************************/

typedef struct Mpeg2TsInfo {
    PipePosition        position;

    Mpeg2TsTransportPacket transportPacket;

    Mpeg2TsAdaptationField adaptationField;
    u8                  adaptationFieldLength;

    Mpeg2TsProgramClockReference pcr;	   /* if adaptationField.pcrf */
    Mpeg2TsProgramClockReference opcr;	   /* if adaptationField.opcrf */

    u8                  spliceCountdown;

    u8                  transportPrivateDataLength;
    u8                  privateDataBytes[MPEG2_TS_PKT_SIZE];

    Mpeg2TsAdaptationFieldExtension adaptationFieldExtension;
    u8                  adaptationFieldExtensionLength;

    Mpeg2TsLtw          ltw;
    Mpeg2TsPiecewiseRate piecewiseRate;
    Mpeg2TsSeamlessSplice seamlessSplice;

    u8			payloadLen;
} Mpeg2TsInfo;

typedef RetCode     (*Mpeg2TsCallBack) (void *token, Mpeg2TsInfo *infop,
					                    RetCode retCode);

/***********************************************************************
 * Mpeg2 Transport Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2TS_CLASSNAME, Mpeg2TsError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2TS_CLASSNAME		"Mpeg2Ts"

typedef enum Mpeg2TsError {
    MPEG2_TS_ERROR_SYNC = 1,		   /* bad sync byte */
    MPEG2_TS_ERROR_TEI,			   /* transport error indicator set */
    MPEG2_TS_ERROR_AFC,			   /* invalid adaptation field code */
    MPEG2_TS_ERROR_LEN,			   /* packet length error */
    MPEG2_TS_ERROR_AFL,			   /* invalid adaptation field length */
    MPEG2_TS_ERROR_AFEL			   /* invalid adaptation field ext
					    * length */
} Mpeg2TsError;

/***********************************************************************
 * Mpeg2 Transport Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Ts *Mpeg2Ts;

/***********************************************************************
 * Mpeg2 Transport Class Methods
 ***********************************************************************/

extern Mpeg2Ts      mpeg2TsNew(void);

/***********************************************************************
 * Mpeg2 Transport Instance Methods
 ***********************************************************************/

extern void         mpeg2TsSetPsiParser(Mpeg2Ts mpeg2Ts, MmpParserObject *psip);

extern MmpParserObject *mpeg2TsParserObject(Mpeg2Ts mpeg2Ts);

extern void
mpeg2TsSetCallBack(Mpeg2Ts mpeg2Ts, Mpeg2TsCallBack callBack,
		   void *token);

/*
 * doFlush == TRUE => parse payload "synchronously"
 */
extern void mpeg2TsSetFlush(Mpeg2Ts mpeg2Ts, Boolean doFlush);

extern void         mpeg2TsFree(Mpeg2Ts mpeg2Ts);

_FOREIGN_END

#endif					   /* _MMP_MPEG2TS_H */
