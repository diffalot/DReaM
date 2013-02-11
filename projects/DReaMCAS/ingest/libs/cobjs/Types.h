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
 * $(@)Types.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Types -- Misc. common type definitions.
 */

#ifndef	_COBJS_TYPES_H
#define	_COBJS_TYPES_H

#pragma ident "@(#)Types.h 1.2	98/10/26 SMI"

#include "cobjs/Foreign.h"

_FOREIGN_START

#undef	FALSE
#undef	TRUE

typedef enum Boolean {
    FALSE = 0,
    TRUE = 1
} Boolean;

typedef void *Object;
typedef long long Ptr64;
typedef long long ConstPtr64;

#ifndef	FIXED_WIDTH_TYPES
#define	FIXED_WIDTH_TYPES
typedef unsigned char uchar;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;
#endif	/* FIXED_WIDTH_TYPES */

_FOREIGN_END

#endif					   /* _COBJS_TYPES_H */
