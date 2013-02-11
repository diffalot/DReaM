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
 * $(@)MetricIo.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * MetricIo.c -- Description.
 */

#pragma ident "@(#)MetricIo.c 1.2	99/04/05 SMI"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "cobjs/Macros.h"

#include "cobjs/MetricIo.h"

#define	METRIC_IO_N_BUFS		10
#define	METRIC_IO_FORMAT_LEN		15

char *
doubleToMetric(double val, const char *unit, MetricIoBase base)
{
    static char buf[METRIC_IO_N_BUFS]
	[METRIC_IO_FORMAT_LEN + METRIC_IO_MAX_UNITS_LEN];
    static int rotor = 0;
    char *bp = buf[rotor];
    double absval = fabs(val);
    double Kbase = base;
    double Mbase = Kbase * Kbase;
    double Gbase = Mbase * Kbase;
    double Tbase = Gbase * Kbase;
    double Pbase = Tbase * Kbase;
    double Xbase = Pbase * Kbase;

    if (strlen(unit) >= METRIC_IO_MAX_UNITS_LEN) {
	return "UNITS TOO LONG";
    }
    rotor = (rotor + 1) % METRIC_IO_N_BUFS;
    if (unit == NULL) unit = "";
    if (absval >= Xbase) {
	(void) sprintf(bp, "%8.3g %s ", val, unit);
    } else if (absval >= Pbase) {
	(void) sprintf(bp, "%8.3f P%s", val / Pbase, unit);
    } else if (absval >= Tbase) {
	(void) sprintf(bp, "%8.3f T%s", val / Tbase, unit);
    } else if (absval >= Gbase) {
	(void) sprintf(bp, "%8.3f G%s", val / Gbase, unit);
    } else if (absval >= Mbase) {
	(void) sprintf(bp, "%8.3f M%s", val / Mbase, unit);
    } else if (absval >= Kbase) {
	(void) sprintf(bp, "%8.3f K%s", val / Kbase, unit);
    } else if (absval >= 1.0) {
	(void) sprintf(bp, "%8.3f %s ", val, unit);
    } else if (absval >= 0.001) {
	(void) sprintf(bp, "%8.3f m%s", val * 1000.0, unit);
    } else if (absval >= 0.000001) {
	(void) sprintf(bp, "%8.3f u%s", val * 1000000.0, unit);
    } else if (absval >= 0.000000001) {
	(void) sprintf(bp, "%8.3f n%s", val * 1000000000.0, unit);
    } else if (absval >= 0.000000000001) {
	(void) sprintf(bp, "%8.3f p%s", val * 1000000000000.0, unit);
    } else {
	(void) sprintf(bp, "%8.3g %s ", val, unit);
    }
    return bp;
}

double
metricToDouble(const char *string, MetricIoBase base, char **cpp)
{
    double val;
    const char *cp;

#if	defined(__lint)
    /*
     * For some reason lint doesn't like the next statement w/o this...
     */
    string = "abc";
    cp = "abc";
#endif
    val = strtod(string, (char **) &cp);
    switch (*cp) {
    case 'P':
	val *= base;
	/* FALLTHRU */
    case 'T':
	val *= base;
	/* FALLTHRU */
    case 'G':
	val *= base;
	/* FALLTHRU */
    case 'M':
	val *= base;
	/* FALLTHRU */
    case 'K':
	val *= base;
	cp += 1;
	break;

    case 'p':
	val /= 1000.0;
	/* FALLTHRU */
    case 'n':
	val /= 1000.0;
	/* FALLTHRU */
    case 'u':
	val /= 1000.0;
	/* FALLTHRU */
    case 'm':
	val /= 1000.0;
	cp += 1;
	break;
    }
    if (cpp != NULL) {
	*cpp = (char *) cp;
    }
    return val;
}
