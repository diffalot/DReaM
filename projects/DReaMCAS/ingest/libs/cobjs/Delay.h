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
 * $(@)Delay.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Delay.h -- Description
 */

#ifndef	_COBJS_DELAY_H
#define	_COBJS_DELAY_H

#pragma ident "@(#)Delay.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Time.h"

#ifdef	__lint
#define	DELAY_FOO	1
#endif	/* __lint */

_FOREIGN_START

/***********************************************************************
 * Function Instance Interface
 ***********************************************************************/

extern void delayTil(Time dayTime);

extern void delayFor(Time delayTime);

extern void delaySecs(double delaySecs);

_FOREIGN_END

#endif					   /* _COBJS_DELAY_H */
