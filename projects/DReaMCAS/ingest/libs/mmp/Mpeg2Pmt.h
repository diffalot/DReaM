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
 * $(@)Mpeg2Pmt.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2Pmt.h -- Interface to mpeg2 program map table parser.
 */
#ifndef	_MMP_MPEG2PMT_H
#define	_MMP_MPEG2PMT_H

#pragma ident "@(#)Mpeg2Pmt.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/HashTable.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/RetCode.h"

#include "mmp/Mmp.h"
#include "mmp/Mpeg2Psi.h"
#include "mmp/Mpeg2.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Mpeg2 Program Map Table Types
 ***********************************************************************/

#define	MPEG2_PMT_TABLE_ID		2
#define	MPEG2_PMT_VN_INVALID		255
#define MPEG2_PMT_STREAM_TYPE_MAX	255

typedef enum Mpeg2PmtCurrentNextIndicator {
    MPEG2_PMT_NEXT = 0,
    MPEG2_PMT_CURRENT = 1
} Mpeg2PmtCurrentNextIndicator;

typedef struct Mpeg2PmtHeader {
    BIT2(reserved_1:3,			   /* reserved */
	 pcrPid12_8:5);			   /* pcr pid */
    BIT1(pcrPid7_0:8);
    BIT2(reserved_2:4,			   /* reserved */
	 programInfoLength11_8:4);
    BIT1(programInfoLength7_0:8);
} Mpeg2PmtHeader;

#define MPEG2_PMT_HEADER_PCR_PID(p)					\
	(   ((p)->pcrPid12_8 << 8)					\
	  |  (p)->pcrPid7_0						\
 	)

#define MPEG2_PMT_HEADER_PROGRAM_INFO_LENGTH(p)				\
	(   ((p)->programInfoLength11_8 << 8)				\
	  |  (p)->programInfoLength7_0					\
 	)

typedef struct Mpeg2PmtStreamHeader {
    BIT1(streamType:8);
    BIT2(reserved_1:3,			   /* reserved */
	 elementaryPid12_8:5);
    BIT1(elementaryPid7_0:8);
    BIT2(reserved_2:4,			   /* reserved */
	 esInfoLength11_8:4);
    BIT1(esInfoLength7_0:8);
} Mpeg2PmtStreamHeader;


#define MPEG2_PMT_STREAM_HEADER_ELEMENTARY_PID(p)			\
	(   ((p)->elementaryPid12_8 << 8)				\
	  |  (p)->elementaryPid7_0					\
 	)

#define MPEG2_PMT_STREAM_HEADER_ES_INFO_LENGTH(p) 			\
	(   ((p)->esInfoLength11_8 << 8)				\
	  |  (p)->esInfoLength7_0					\
 	)

typedef struct Mpeg2PmtDescriptor Mpeg2PmtDescriptor;
struct Mpeg2PmtDescriptor {
    Mpeg2PmtDescriptor *next;
    Mpeg2Descriptor     descriptor;
};

typedef struct Mpeg2PmtStream Mpeg2PmtStream;
struct Mpeg2PmtStream {
    Mpeg2PmtStream     *next;
    Mpeg2PmtStreamHeader streamHeader;
    Mpeg2PmtDescriptor *descList;
    MmpContextObject   *cop;
};

#define	MPEG2_PMT_PROGRAM_NUMBER(p)				\
	MPEG2_PSI_EXTENSION_TABLE_ID_EXTENSION(p)

typedef struct Mpeg2PmtTsProgramMapSection {
    Mpeg2PsiHeader      psiHeader;
    Mpeg2PsiExtension   psiExtension;
    Mpeg2PmtHeader      pmtHeader;
    Mpeg2PmtDescriptor *descList;
    Mpeg2PmtStream     *streamList;
    u32                 crc;
}                   Mpeg2PmtTsProgramMapSection;

/***********************************************************************
 * Mpeg2 Program Map Table Callback Info Types
 ***********************************************************************/

typedef struct Mpeg2PmtInfo {
    PipePosition        position;

    Mpeg2PmtTsProgramMapSection tsProgramMapSection;
} Mpeg2PmtInfo;

typedef RetCode     (*Mpeg2PmtCallBack) (void *token, Mpeg2PmtInfo *infop,
				                           RetCode retCode);


/***********************************************************************
 * Mpeg2 PMT Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2PMT_CLASSNAME, Mpeg2PmtError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2PMT_CLASSNAME		"Mpeg2Pmt"

typedef enum Mpeg2PmtError {
    MPEG2_PMT_ERROR_SSI = 1,		   /* section syntax indicator not
					    * set */
    MPEG2_PMT_ERROR_SN,			   /* (last) section number != 0 */
    MPEG2_PMT_ERROR_PIL_LEN,		   /* program info length error */
    MPEG2_PMT_ERROR_LEN,		   /* pmt length error */
    MPEG2_PMT_ERROR_CRC,		   /* crc error */
    MPEG2_PMT_ERROR_STOP,		   /* stop on pmt found */
    MPEG2_PMT_ERROR_PID_USE,		   /* pid use conflict */
    MPEG2_PMT_ERROR_EOF 		   /* unexpected eof */
} Mpeg2PmtError;

/***********************************************************************
 * Mpeg2Pmt Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Pmt *Mpeg2Pmt;

/***********************************************************************
 * Mpeg2Pmt Class Interface
 ***********************************************************************/

extern Mpeg2Pmt     mpeg2PmtNew(void);

/***********************************************************************
 * Mpeg2Pmt Instance Interface
 ***********************************************************************/

extern void
mpeg2PmtSetPesParsers(Mpeg2Pmt mpeg2Pmt,
	   MmpParserObject *streamTypeToPop[MPEG2_PMT_STREAM_TYPE_MAX + 1]);

extern MmpParserObject *mpeg2PmtParserObject(Mpeg2Pmt mpeg2Pmt);

extern void
mpeg2PmtSetCallBack(Mpeg2Pmt mpeg2Pmt, Mpeg2PmtCallBack callBack,
                    void *token);

extern void         mpeg2PmtSetStrict(Mpeg2Pmt mpeg2Pmt, Boolean isStrict);

extern Boolean      mpeg2PmtGetStrict(Mpeg2Pmt mpeg2Pmt);

extern void
mpeg2PmtSetPmtStop(Mpeg2Pmt mpeg2Pmt,
                   Boolean shouldPmtStop);

extern Boolean      mpeg2PmtGetPmtStop(Mpeg2Pmt mpeg2Pmt);

typedef struct Mpeg2PmtStatus {
    /*
     * pmtTable[0] is "next" pat. pmtTable[1] is "current" pat.
     * 
     * HashTable maps from programNumber to Mpeg2PmtTsProgramMapSection *
     */
    HashTable           pmtTable[2];
} Mpeg2PmtStatus;

extern Mpeg2PmtStatus mpeg2PmtStatus(Mpeg2Pmt mpeg2Pmt);

extern void         mpeg2PmtFree(Mpeg2Pmt mpeg2Pmt);

_FOREIGN_END

#endif					   /* _MMP_MPEG2PMT_H */
