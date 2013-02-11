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
 * $(@)Package.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 */

/*
 * Package.h -- Description of Package
 *
 * This file generated from Tmplate version: 1.3 98/11/18
 */

#ifndef	_COBJS_PACKAGE_H
#define	_COBJS_PACKAGE_H

#pragma ident "@(#)Package.h 1.1	99/07/07 SMI"

/***********************************************************************
 * Global Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

/***********************************************************************
 * Local Includes
 ***********************************************************************/

_FOREIGN_START
/* local includes go here */
_FOREIGN_END

/***********************************************************************
 * Public Types
 ***********************************************************************/

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PACKAGE_BODY)
#define	PACKAGE_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(PACKAGE_BODY) */
#define	PACKAGE_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(PACKAGE_BODY) */


/*
 * packageHome(component, relPathToHome, pathBuf)
 *
 * Finds path to packageHome of given component.
 *
 * If component is NULL, main program is assumed; else
 *  component name should be library name like "libc.so.1"
 *
 * relPathToHome gives the relative path from the directory that
 * holds the component to the package home.
 *
 * pathBuf will hold packageHome path, it must be at least MAXPATHLEN bytes.
 */
extern char *
packageHome(const char *component, const char *relPathToHome, char *pathBuf);

/*
 * Resolve argv0 to true path to command
 */
extern char *
packageCommand(const char *argv0, char *pathBuf);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(PACKAGE_BODY)
#define	PACKAGE_HEADER
#include "cobjs/Package.c"
#undef	PACKAGE_HEADER
#endif		   /* defined(DO_INLINING) && !defined(PACKAGE_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_PACKAGE_H */
