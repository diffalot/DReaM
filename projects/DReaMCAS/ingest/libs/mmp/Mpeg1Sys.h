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
 * $(@)Mpeg1Sys.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg1Sys.h
 * 
 * Mpeg1Sys.h is the interface to information gleaned by parsing MPEG1 System
 * streams.
 */

#ifndef	_MMP_MPEG1SYS_H
#define	_MMP_MPEG1SYS_H

#pragma ident "@(#)Mpeg1Sys.h 1.1	96/09/18 SMI"

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
 * Mpeg1Sys Types
 ***********************************************************************/

typedef struct Mpeg1SysPack {
    BIT1(packStartCode31_24:8);
    BIT1(packStartCode23_16:8);
    BIT1(packStartCode15_8:8);
    BIT1(packStartCode7_0:8);
    BIT3(b0010:4,
	 systemClockReference32_30:3,
	 markerBit_1:1);
    BIT1(systemClockReference29_22:8);
    BIT2(systemClockReference21_15:7,
	 markerBit_2:1);
    BIT1(systemClockReference14_7:8);
    BIT2(systemClockReference6_0:7,
	 markerBit_3:1);
    BIT2(markerBit_4:1,
	 muxRate21_15:7);
    BIT1(muxRate14_7:8);
    BIT2(muxRate6_0:7,
	 markerBit_5:1);
} Mpeg1SysPack;

typedef enum Mpeg1SysStartCodes {
    MPEG1_SYS_START_CODE_END = 0x1b9,
    MPEG1_SYS_START_CODE_PACK = 0x1ba,
    MPEG1_SYS_START_CODE_SYSTEM = 0x1bb
} Mpeg1SysStartCodes;

#define	MPEG1_SYS_PACK_START_CODE(p)					\
	(   ((p)->packStartCode31_24 << 24)				\
	  | ((p)->packStartCode23_16 << 16)				\
	  | ((p)->packStartCode15_8 << 8)				\
	  |  (p)->packStartCode7_0					\
	)

#define	MPEG1_SYS_PACK_SYSTEM_CLOCK_REFERENCE(p)			\
	(   ( (u64)((p)->systemClockReference32_30) << 30) 		\
	  |        ((p)->systemClockReference29_22 << 22) 		\
	  |        ((p)->systemClockReference21_15 << 15) 		\
	  |        ((p)->systemClockReference14_7 << 7) 		\
	  |         (p)->systemClockReference6_0			\
	)

#define	MPEG1_SYS_PACK_MUX_RATE(p)					\
	(   ((p)->muxRate21_15 << 15)					\
	  | ((p)->muxRate14_7 < 7) 					\
	  |  (p)->muxRate6_0						\
	)

typedef struct Mpeg1SysSystemHeader {
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
} Mpeg1SysSystemHeader;

#define	MPEG1_SYS_SYSTEM_HEADER_START_CODE(p)				\
	(   ((p)->systemHeaderStartCode31_24 << 24)			\
	  | ((p)->systemHeaderStartCode23_16 << 16)			\
	  | ((p)->systemHeaderStartCode15_8 << 8)			\
	  |  (p)->systemHeaderStartCode7_0				\
	)

#define	MPEG1_SYS_SYSTEM_HEADER_LENGTH(p)				\
	(   ((p)->headerLength15_8 << 8)				\
	  |  (p)->headerLength7_0					\
	)

#define	MPEG1_SYS_SYSTEM_HEADER_RATE_BOUND(p)				\
	(   ((p)->rateBound21_15 << 15)					\
	  | ((p)->rateBound14_7 << 7)					\
	  |  (p)->rateBound6_0						\
	)

typedef struct Mpeg1SysStdBuffer {
    BIT1(streamId:8);
    BIT3(b11:2,
	 stdBufferBoundScale:1,
	 stdBufferSizeBound12_8:5);
    BIT1(stdBufferSizeBound7_0:8);
} Mpeg1SysStdBuffer;

#define	MPEG1_SYS_STD_BUFFER_SIZE_BOUND(p)				\
	(   ((p)->stdBufferSizeBound12_8 << 8)				\
	  |  (p)->stdBufferSizeBound7_0					\
	)

/***********************************************************************
 * OBJECT Mpeg1Sys Parser CallBack Info Type
 ***********************************************************************/

#define	MPEG1_SYS_MAX_STD_BUFFERS		16

typedef struct Mpeg1SysInfo {
    PipePosition        position;

    /* pack info */
    Mpeg1SysPack        pack;

    /* system header */
    Boolean             hasSystemHeader;   /* systemHeader valid iff TRUE */
    Mpeg1SysSystemHeader systemHeader;

    Boolean             nStdBuffer;	   /* num valid stdBuffer entries */
    Mpeg1SysStdBuffer   stdBuffer[MPEG1_SYS_MAX_STD_BUFFERS];
} Mpeg1SysInfo;

typedef RetCode     (*Mpeg1SysCallBack) (void *token, Mpeg1SysInfo *infop,
				                           RetCode retCode);

/***********************************************************************
 * Mpeg1 Sysytem Error Codes
 *
 * NOTE: Use retCodeFor(MPEG1SYS_CLASSNAME, Mpeg1SysError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG1SYS_CLASSNAME	"Mpeg1Sys"

typedef enum Mpeg1SysError {
    MPEG1_SYS_ERROR_SYNTAX = 1,		   /* pack syntax error */
    MPEG1_SYS_ERROR_END_CODE,		   /* missing end code */
    MPEG1_SYS_ERROR_SYSTEM_HDR_LEN	   /* system header length error */
} Mpeg1SysError;

/***********************************************************************
 * OBJECT Mpeg1Sys Parser Instance Type
 ***********************************************************************/

typedef struct _Mpeg1Sys *Mpeg1Sys;

/***********************************************************************
 * OBJECT Mpeg1Sys Parser Class Interface
 ***********************************************************************/

extern Mpeg1Sys     mpeg1SysNew(void);

/***********************************************************************
 * OBJECT Mpeg1Sys Parser Instance Interface
 ***********************************************************************/

/*
 * Set PES parser
 */
extern void
mpeg1SysSetPesParser(Mpeg1Sys mpeg1Sys,
		     MmpParserObject *pesp);

extern MmpParserObject *mpeg1SysParserObject(Mpeg1Sys mpeg1Sys);

extern void
mpeg1SysSetCallBack(Mpeg1Sys mpeg1Sys, Mpeg1SysCallBack callBack,
		    void *token);

extern void         mpeg1SysFree(Mpeg1Sys mpeg1Sys);

_FOREIGN_END

#endif					   /* _MMP_MPEG1SYS_H */
