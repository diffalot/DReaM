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
 * $(@)CheckSum.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * CheckSum.h -- Description
 */

#ifndef	_COBJS_CHECKSUM_H
#define	_COBJS_CHECKSUM_H

#pragma ident "@(#)CheckSum.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _CheckSum *CheckSum;

/***********************************************************************
 * Public Types
 ***********************************************************************/

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(CHECKSUM_BODY)
#define	CHECKSUM_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(CHECKSUM_BODY) */
#define	CHECKSUM_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(CHECKSUM_BODY) */

/***********************************************************************
 * Function Interface
 ***********************************************************************/

extern u32 checkSum32(const void *blockp, size_t len);

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern CheckSum     checkSumNew(void);

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

/*
 * CHECKSUM_INLINE void checkSumInline(CheckSum checkSum);
 */

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

extern u32 checkSum(CheckSum checkSum, const void *blockp, size_t len);
extern void checkSumClear(CheckSum checkSum);
extern void         checkSumFree(CheckSum checkSum);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(CHECKSUM_BODY)
#define	CHECKSUM_HEADER
#include "cobjs/CheckSum.c"
#undef	CHECKSUM_HEADER
#endif		   /* defined(DO_INLINING) && !defined(CHECKSUM_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_CHECKSUM_H */
