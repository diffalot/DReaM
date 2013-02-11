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
 * $(@)Inline.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Inline.h
 * 
 * Inline.h -- inlining helper macros.
 */

#ifndef	_COJBS_INLINE_H
#define	_COJBS_INLINE_H

#pragma ident "@(#)Inline.h 1.3	99/12/10 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include "Foreign.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

#if defined(__lint) && ! defined(DO_INLINING)
#define	INLINE_PRIVATE	static
#else	/* __lint */
#define	INLINE_PRIVATE	extern
#endif	/* __lint */

#if	defined(DO_INLINING)
#define	INLINE_DEFIN
#else					   /* defined(DO_INLINING) */
#define	INLINE_DEFIN	static
#endif					   /* defined(DO_INLINING) */

_FOREIGN_END

#endif					   /* _COJBS_INLINE_H */
