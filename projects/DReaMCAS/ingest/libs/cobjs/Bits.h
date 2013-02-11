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
 * $(@)Bits.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Bits.h -- Description of Bits
 */

#ifndef	_COBJS_BITS_H
#define	_COBJS_BITS_H

#pragma ident "@(#)Bits.h 1.2	99/05/28 SMI"

/***********************************************************************
 * Global Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

/***********************************************************************
 * Local Includes
 ***********************************************************************/

_FOREIGN_START
/* local includes go here */
_FOREIGN_END

/***********************************************************************
 * Public Types
 ***********************************************************************/

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(BITS_BODY)
#define	BITS_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(BITS_BODY) */
#define	BITS_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(BITS_BODY) */

/***********************************************************************
 * Inline-able Function Interface
 ***********************************************************************/

/*
 * BITS_INLINE void *bitsInline(Bits bits);
 */

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/
extern s32 bitsFindFirst0(s32 word);
extern s32 bitsFindFirst1(s32 word);
extern s32 bitsCount1s(s32 word);
extern s32 bitsCount0s(s32 word);

/*
 * Return mask of count 1's in low-order bits
 */
extern u32 bitsLow1s(s32 count);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(BITS_BODY)
#define	BITS_HEADER
#include "cobjs/Bits.c"
#undef	BITS_HEADER
#endif		   /* defined(DO_INLINING) && !defined(BITS_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_BITS_H */
