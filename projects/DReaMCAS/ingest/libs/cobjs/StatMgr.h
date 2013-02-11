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
 * $(@)StatMgr.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * StatMgr.h -- Description
 */

#ifndef	_COBJS_STATMGR_H
#define	_COBJS_STATMGR_H

#pragma ident "@(#)StatMgr.h 1.2	99/08/27 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/ArrayOf.h"
#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _StatMgr *StatMgr;

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef struct StatMgrStats {
    int		cycles;
    double	minValue;
    double	maxValue;
    double	sumValue;
    double	sumValue2;
    double	avgValue;
    double	stdDev;
    double	bucketSize;
    double	baseBucket;
    int		nBuckets;
    int		histogramUnder;
    int		histogramOver;
    ArrayOf	histogram;
} StatMgrStats;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(STATMGR_BODY)
#define	STATMGR_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(STATMGR_BODY) */
#define	STATMGR_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(STATMGR_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern StatMgr     statMgrNew(void);

/*
 * Keep a histogram of samples.
 * Histogram starts at baseBucket with nBuckets of bucketSize.
 * Under/over range samples are counted in histogramUnder/histogramOver.
 */
extern StatMgr	   statMgrNewWithHistogram(double bucketSize,
			double baseBucket, int nBuckets);

/***********************************************************************
 * Instance Interface
 ***********************************************************************/

/*
 * Clear the statMgr and all stats
 */
extern void statMgrClear(StatMgr statMgr);

/*
 * Record a value
 */
extern void statMgrRecord(StatMgr statMgr, double value);

/*
 * Get complete statMgr statistics
 *
 * BEWARE: the histogram goes away when the statMgr is freed.
 */
extern StatMgrStats statMgrStats(StatMgr statMgr);

/*
 * Free the statMgr
 */
extern void         statMgrFree(StatMgr statMgr);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(STATMGR_BODY)
#define	STATMGR_HEADER
#include "cobjs/StatMgr.c"
#undef	STATMGR_HEADER
#endif		   /* defined(DO_INLINING) && !defined(STATMGR_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_STATMGR_H */
