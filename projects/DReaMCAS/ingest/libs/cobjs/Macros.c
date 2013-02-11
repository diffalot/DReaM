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
 * $(@)Macros.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

#pragma	ident "@(#)Macros.c 1.2	99/06/11 SMI"

/*
 * Macros.c -- macro assist routines
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

#ifdef	__lint
int                 __FALSE__ = 0;
#define	_mallocHandler	mallocHandler
#endif					   /* __lint */

void
_mallocHandler(const char *type, size_t num,
	       size_t elsize, const char *file,
	       int line);

#pragma weak mallocHandler = _mallocHandler

void               *
ckMalloc(const char *type, const char *file, int line,
	 size_t num, size_t elsize)
{
    void               *p;

    if ((p = malloc(num * elsize)) == NULL) {
	_mallocHandler(type, num, elsize, file, line);
	if ((p = malloc(num * elsize)) == NULL) {
	    logLibPrint(cobjs_TEXT_DOMAIN,
		   "malloc: out of memory @ %s,%d: alloc %s[%d] (%d:%d)\n",
		   file, line, type, num, elsize, elsize * num);
	    abort();
	}
    }
    return p;
}

void               *
ckCalloc(const char *type, const char *file, int line,
	 size_t num, size_t elsize)
{
    void               *p;

    if ((p = calloc(num, elsize)) == NULL) {
	_mallocHandler(type, num, elsize, file, line);
	if ((p = calloc(num, elsize)) == NULL) {
	    logLibPrint(cobjs_TEXT_DOMAIN,
		   "calloc: out of memory @ %s,%d: alloc %s[%d] (%d:%d)\n",
		   file, line, type, num, elsize, elsize * num);
	    abort();
	}
    }
    return p;
}

void               *
ckRealloc(const char *type, const char *file, int line,
	  void *ptr, size_t num, size_t elsize)
{
    void               *p;

    if ((p = realloc(ptr, num * elsize)) == NULL) {
	_mallocHandler(type, num, elsize, file, line);
	if ((p = realloc(ptr, num * elsize)) == NULL) {
	    logLibPrint(cobjs_TEXT_DOMAIN,
		   "realloc: out of memory @ %s,%d: alloc %s[%d] (%d:%d)\n",
		   file, line, type, num, elsize, elsize * num);
	    abort();
	}
    }
    return p;
}

/* ARGSUSED */
void
_mallocHandler(const char *type, size_t num, size_t elsize, const char *file,
	       int line)
{
    /*
     * Do nothing by default
     */
}
