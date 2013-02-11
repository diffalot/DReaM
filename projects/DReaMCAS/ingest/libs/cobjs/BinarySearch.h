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
 * $(@)BinarySearch.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * BinarySearch.h
 * 
 * A generalized binary search algorithm.
 */

#ifndef	_COBJS_BINARYSEARCH_H
#define	_COBJS_BINARYSEARCH_H

#pragma ident "@(#)BinarySearch.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <stddef.h>

#include "cobjs/Foreign.h"

_FOREIGN_START

/***********************************************************************
 * BinarySearch Public Types
 ***********************************************************************/

typedef enum BinarySearchOption {
    BINARY_SEARCH_LOWER,		   /* if not found, return lower
					    * index */
    BINARY_SEARCH_HIGHER,		   /* if not found, return higher
					    * index */
    BINARY_SEARCH_FLOOR,		   /* if not found, return lower
					    * value */
    BINARY_SEARCH_CEIL,			   /* if not found, return higher
					    * value */
    BINARY_SEARCH_FAIL			   /* if not found, return -1 */
} BinarySearchOption;

typedef enum BinarySearchResult {
    BINARY_SEARCH_MATCH,		   /* found item */
    BINARY_SEARCH_BETWEEN,		   /* not found, but between existing
					    * items */
    BINARY_SEARCH_BEFORE,		   /* not found, before all items */
    BINARY_SEARCH_AFTER			   /* not found, after all items */
} BinarySearchResult;

/***********************************************************************
 * OBJECT Template Instance Interface
 ***********************************************************************/

/*
 * Perform binary search on data array pointed to by base.
 * 
 * Number of elements in array is given by nel. Size of each element is given by
 * width. Pointer to target of search is given by targetp.
 * 
 * Comparison function, compar, should return <0, 0, >0 if element pointed to by
 * el1p is less than, equal, or greater than element pointed to by el2p.
 * 
 * If target not found, search resolves as indicated by BinarySearchOption opt.
 * 
 * Index of element resolved by search is returned via pointer indexp.
 * 
 * Returns TRUE if target found, FALSE otherwise.
 */
extern BinarySearchResult
binarySearch(const void *base, size_t nel, size_t width, const void *targetp,
	     int (*compar) (const void *el1p, const void *el2p),
	     BinarySearchOption opt, int *indexp);

_FOREIGN_END

#endif					   /* _COBJS_BINARYSEARCH_H */
