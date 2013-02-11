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
 * $(@)MetricIo.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * MetricIo.h -- Description
 */

#ifndef	_METRICIO_H
#define	_METRICIO_H

#pragma ident "@(#)MetricIo.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

#define	METRIC_IO_MAX_UNITS_LEN		80

typedef enum MetricIoBase {
    METRIC_IO_BASE_DECIMAL	= 1000,
    METRIC_IO_BASE_BINARY	= 1024
} MetricIoBase;

extern char *doubleToMetric(double val, const char *unit, MetricIoBase base);

extern double metricToDouble(const char *string, MetricIoBase base, char **cpp);

_FOREIGN_END

#endif					   /* _METRICIO_H */
