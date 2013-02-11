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
 * $(@)BinarySearch.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:33 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)BinarySearch.c 1.1	98/10/22 SMI"

/*
 * BinarySearch.c -- A generalized binary search algorithm.
 */
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

#include "cobjs/BinarySearch.h"

#define	PTR(base, width, i) ((void *)(((char *)base) + ((width) * (i))))

BinarySearchResult
binarySearch(const void *base, size_t nel, size_t width, const void *targetp,
	     int (*compar) (const void *el1p, const void *el2p),
	     BinarySearchOption opt, int *indexp)
{
    int                 low = 0;
    int                 high = nel - 1;
    int                 probe;
    int                 cmp;
    Boolean             isReverse;
    BinarySearchResult  result;

    /*
     * isReverse is TRUE if data is ordered with decreasing values as index
     * increases
     */
    if (nel == 0) {
	probe = 0;
	switch (opt) {
	case BINARY_SEARCH_LOWER:
	case BINARY_SEARCH_FLOOR:
	    result = BINARY_SEARCH_BEFORE;
	    break;
	case BINARY_SEARCH_HIGHER:
	case BINARY_SEARCH_CEIL:
	    result = BINARY_SEARCH_AFTER;
	    break;
	default:
	    result = BINARY_SEARCH_AFTER;
	    ABORT("illegal binary search option");
	    break;
	}
	goto done;
    }
    isReverse = Boolean((*compar) (PTR(base, width, 0),
                        PTR(base, width, nel - 1)) > 0);
    while (high - low > 1) {
	int                 cmp;

	probe = (low + high) / 2;
	cmp = (*compar) (targetp, PTR(base, width, probe));

	if (cmp == 0) {
	    result = BINARY_SEARCH_MATCH;
	    goto done;
	}
	if (isReverse ^ (cmp < 0)) {
	    high = probe;
	} else {
	    low = probe;
	}
    }
    /*
     * At this point, high - low <= 1 (0 is only possible if nel == 1).
     * Target can be anywhere in relationship to high and low (because it
     * could be outside the range of all values in the table).
     */
    cmp = (*compar) (targetp, PTR(base, width, high));
    if (cmp == 0 || (isReverse ^ (cmp > 0))) {
	probe = high;
	result = (cmp == 0) ? BINARY_SEARCH_MATCH : BINARY_SEARCH_AFTER;
	ASSERT(cmp == 0 || high == nel - 1);
	goto done;
    }
    cmp = (*compar) (targetp, PTR(base, width, low));
    if (cmp == 0 || (isReverse ^ (cmp < 0))) {
	probe = low;
	result = (cmp == 0) ? BINARY_SEARCH_MATCH : BINARY_SEARCH_BEFORE;
	ASSERT(cmp == 0 || low == 0);
	goto done;
    }
    /*
     * Target is between low and high
     */
    result = BINARY_SEARCH_BETWEEN;
    switch (opt) {
    case BINARY_SEARCH_LOWER:
	probe = low;
	break;
    case BINARY_SEARCH_HIGHER:
	probe = high;
	break;
    case BINARY_SEARCH_FLOOR:
	probe = isReverse ? high : low;
	break;
    case BINARY_SEARCH_CEIL:
	probe = isReverse ? low : high;
	break;
    default:
	probe = 0;
	ABORT("illegal binary search option");
	break;
    }
done:
    if (indexp != NULL) {
	*indexp = probe;
    }
    return result;
}
