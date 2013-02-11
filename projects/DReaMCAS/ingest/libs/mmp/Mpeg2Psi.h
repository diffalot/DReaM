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
 * $(@)Mpeg2Psi.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2Psi.h -- Interface to generalize mpeg2 psi parser.
 * 
 * Mpeg2Psi.h recognizes any type of mpeg2 psi stream and invokes the
 * appropriate parser
 */
#ifndef	_MMP_MPEG2PSI_H
#define	_MMP_MPEG2PSI_H

#pragma ident "@(#)Mpeg2Psi.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

#include "mmp/Mmp.h"
#include "mmp/Pipe.h"

_FOREIGN_START

/***********************************************************************
 * Mpeg2 Program Specific Information Types
 ***********************************************************************/

#define	MPEG2_PSI_TABLE_ID_STUFF	0xff

typedef struct Mpeg2PsiHeader {
    BIT1(tableId:8);
    BIT4(sectionSyntaxIndicator:1,
	 privateIndicator:1,
	 reserved:2,
	 sectionLength11_8:4);
    BIT1(sectionLength7_0:8);
} Mpeg2PsiHeader;

#define	MPEG2_PSI_HEADER_SECTION_LENGTH(p)				\
	(								\
	    ((p)->sectionLength11_8 << 8)				\
	  |  (p)->sectionLength7_0					\
	)

typedef struct Mpeg2PsiExtension {
    BIT1(tableIdExtension15_8:8);
    BIT1(tableIdExtension7_0:8);
    BIT3(reserved:2,
	 versionNumber:5,
	 currentNextIndicator:1);	   /* TRUE => current, FALSE => next */
    BIT1(sectionNumber:8);
    BIT1(lastSectionNumber:8);
} Mpeg2PsiExtension;

#define	MPEG2_PSI_EXTENSION_TABLE_ID_EXTENSION(p)			\
	(								\
	    ((p)->tableIdExtension15_8 << 8)				\
	  |  (p)->tableIdExtension7_0					\
	)

/***********************************************************************
 * Mpeg2Psi Error Codes
 *
 * NOTE: Use retCodeFor(MPEG2PSI_CLASSNAME, Mpeg2PsiError) to map to
 * RetCode.
 ***********************************************************************/

#define	MPEG2PSI_CLASSNAME		"Mpeg2Psi"

typedef enum Mpeg2PsiError {
    MPEG2_PSI_ERROR_SL = 1		   /* section length error */
} Mpeg2PsiError;

/***********************************************************************
 * Mpeg2Psi Instance Type
 ***********************************************************************/

typedef struct _Mpeg2Psi *Mpeg2Psi;

/***********************************************************************
 * Mpeg2Psi Class Interface
 ***********************************************************************/

extern Mpeg2Psi     mpeg2PsiNew(void);

/***********************************************************************
 * Mpeg2Psi Instance Interface
 ***********************************************************************/

extern void
mpeg2PsiSetTableParsers(Mpeg2Psi mpeg2Psi,
		   MmpParserObject *tableIdToPop[MPEG2_PSI_TABLE_ID_STUFF]);

extern MmpParserObject *mpeg2PsiParserObject(Mpeg2Psi mpeg2Psi);

extern void         mpeg2PsiSetRecover(Mpeg2Psi mpeg2Psi, Boolean doRecover);

extern Boolean      mpeg2PsiGetRecover(Mpeg2Psi mpeg2Psi);

extern void         mpeg2PsiFree(Mpeg2Psi mpeg2Psi);

_FOREIGN_END

#endif					   /* _MMP_MPEG2PSI_H */
