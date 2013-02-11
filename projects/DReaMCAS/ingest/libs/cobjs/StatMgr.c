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
 * $(@)StatMgr.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * StatMgr.c -- Description.
 */

#pragma ident "@(#)StatMgr.c 1.3	99/12/10 SMI"

#if	!defined(STATMGR_HEADER)
#define	STATMGR_BODY
#define	STATMGR_INLINE		extern
#include "cobjs/StatMgr.h"
#endif					   /* !defined(STATMGR_HEADER) */

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "cobjs/Macros.h"

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _StatMgr {
    StatMgrStats	stats;
};

#if	!defined(STATMGR_HEADER)

/*************************************************************************
 * Class Methods
 *************************************************************************/

StatMgr
statMgrNew(void)
{
    StatMgr aStatMgr = NEW_ZEROED(struct _StatMgr, 1);
    aStatMgr->stats.histogram = NULL;
    statMgrClear(aStatMgr);
    return aStatMgr;
}

StatMgr
statMgrNewWithHistogram(double bucketSize, double baseBucket, int nBuckets)
{
    StatMgr aStatMgr = NEW_ZEROED(struct _StatMgr, 1);
    aStatMgr->stats.histogram = NEW_ARRAY(s32);
    aStatMgr->stats.bucketSize = bucketSize;
    aStatMgr->stats.baseBucket= baseBucket;
    aStatMgr->stats.nBuckets= nBuckets;
    arrayOfItemAt(aStatMgr->stats.histogram, s32, nBuckets - 1) = 0;
    statMgrClear(aStatMgr);
    return aStatMgr;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

/*
 * Set the statMgr to a specific value
 */
void
statMgrClear(StatMgr statMgr)
{
    statMgr->stats.cycles = 0;
    statMgr->stats.minValue = DBL_MAX;
    statMgr->stats.maxValue = 0.0;
    statMgr->stats.sumValue = 0.0;
    statMgr->stats.sumValue2 = 0.0;
    statMgr->stats.avgValue = 0.0;
    statMgr->stats.stdDev = 0.0;
    statMgr->stats.histogramUnder = 0;
    statMgr->stats.histogramOver = 0;
    if (statMgr->stats.histogram != NULL) {
	arrayOfSetLength(statMgr->stats.histogram, 0);
	arrayOfItemAt(statMgr->stats.histogram, s32,
		statMgr->stats.nBuckets - 1) = 0;
    }
}

/*
 * Stop and immediately restart the statMgr and return the elapsed time
 */
void
statMgrRecord(StatMgr statMgr, double value)
{
    statMgr->stats.cycles += 1;
    statMgr->stats.minValue = MIN(value, statMgr->stats.minValue);
    statMgr->stats.maxValue = MAX(value, statMgr->stats.maxValue);
    statMgr->stats.sumValue += value;
    statMgr->stats.sumValue2 += (value * value);
    if (statMgr->stats.histogram != NULL) {
	int bucket = (value - statMgr->stats.baseBucket)
	    / statMgr->stats.bucketSize;
	if (bucket < 0) {
	    statMgr->stats.histogramUnder += 1;
	} else if (bucket >= statMgr->stats.nBuckets) {
	    statMgr->stats.histogramOver += 1;
	} else {
	    arrayOfItemAt(statMgr->stats.histogram, s32, bucket) += 1;
	}
    }
}

/*
 * Get complete statMgr statistics
 */
StatMgrStats
statMgrStats(StatMgr statMgr)
{
    if (statMgr->stats.cycles > 0) {
	statMgr->stats.avgValue = statMgr->stats.sumValue
		/ statMgr->stats.cycles;
    }
    if (statMgr->stats.cycles > 1) {
	statMgr->stats.stdDev = sqrt(
		(statMgr->stats.cycles * statMgr->stats.sumValue2
		    - statMgr->stats.sumValue * statMgr->stats.sumValue)
		/ ((double) statMgr->stats.cycles * (statMgr->stats.cycles - 1))
	    );
    }
    return statMgr->stats;
}

/*
 * Free the statMgr
 */
void
statMgrFree(StatMgr statMgr)
{
    if (statMgr->stats.histogram != NULL) {
	arrayOfFree(statMgr->stats.histogram);
    }
    free(statMgr);
}

#endif					   /* !defined(STATMGR_HEADER) */
