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
 * $(@)CheckSum.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * CheckSum.c -- Description.
 */

#pragma ident "@(#)CheckSum.c 1.1	98/10/22 SMI"

#if	!defined(CHECKSUM_HEADER)
#define	CHECKSUM_BODY
#define	CHECKSUM_INLINE		extern
#include "cobjs/CheckSum.h"
#endif					   /* !defined(CHECKSUM_HEADER) */

#include <stdlib.h>

#include "cobjs/Macros.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _CheckSum {
    u32		partial;
    size_t	offset;		
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void checkSumInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * CHECKSUM_INLINE void
 * checkSumInlineRoutine(void)
 * {
 * }
 */

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(CHECKSUM_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

static u32 checkSumBlock(const void *blockp, size_t len, size_t offset);

/*************************************************************************
 * Private class data
 *************************************************************************/

/*************************************************************************
 * Function Interface
 *************************************************************************/

u32
checkSum32(const void *blockp, size_t len)
{
    return checkSumBlock(blockp, len, 0);
}

/*************************************************************************
 * Class Methods
 *************************************************************************/

CheckSum
checkSumNew(void)
{
    CheckSum checkSum = NEW_ZEROED(struct _CheckSum, 1);
    return checkSum;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

u32
checkSum(CheckSum checkSum, const void *blockp, size_t len)
{
    checkSum->partial += checkSumBlock(blockp, len, checkSum->offset);
    checkSum->offset += len;
    return checkSum->partial;
}

void
checkSumClear(CheckSum checkSum)
{
    checkSum->partial = 0;
    checkSum->offset = 0;
}

void
checkSumFree(CheckSum checkSum)
{
    free(checkSum);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 * INLINE_PRIVATE void
 * checkSumInlinePrivate(void)
 * {
 * }
 * 
 * static void
 * checkSumPrivate(void)
 * {
 * }
 */

/*************************************************************************
 * Private Functions
 *************************************************************************/

static u32
checkSumBlock(const void *blockp, size_t len, size_t offset)
{
    u32 sum = 0;
    const u8 *u8p = (const u8 *)blockp;

    /*
     * Performs a big-endian 2's complement checksum
     */
    if ((offset & (sizeof(u32) - 1)) == 0
	&& ((long) blockp & (sizeof(u32) - 1)) == 0) {
	const u32 *u32p = (const u32 *)blockp;
	while (len >= sizeof(u32)) {
	    sum += *u32p++;
	    len -= sizeof(u32);
	}
	u8p += len;
    }
    /*
     * do it the hard way
     */
    while (len-- > 0) {
	sum += *u8p++ << (24 - (offset++ & (sizeof(u32)) - 1) * 8);
    }
    return sum;
}

#endif					   /* !defined(CHECKSUM_HEADER) */
