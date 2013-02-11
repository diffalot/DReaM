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
 * $(@)Option.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Option.h -- Description
 */

#ifndef	_COBJS_OPTION_H
#define	_COBJS_OPTION_H

#pragma ident "@(#)Option.h 1.2	99/07/29 SMI"

/***********************************************************************
 * Includes
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
 * Instance Types
 ***********************************************************************/

typedef struct _Option *Option;

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef enum OptionType {
    OPTION_TYPE_FLAG,			/* -a */
    OPTION_TYPE_BOOLEAN,		/* -a => TRUE, -A => FALSE */
    OPTION_TYPE_STRING,			/* -a abc */
    OPTION_TYPE_INT,			/* -a 1 */
    OPTION_TYPE_DBL,			/* -a 1.0 */
    OPTION_TYPE_USAGE			/* -h => show usage string */
} OptionType;

typedef struct OptionEntry {
    int		flag;
    OptionType	type;
    void	*ptr;
    char	*desc;
} OptionEntry;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(OPTION_BODY)
#define	OPTION_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(OPTION_BODY) */
#define	OPTION_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(OPTION_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern Option optionNew(int argc, char **argv, OptionEntry table[]);

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

OPTION_INLINE int optionFlag(Option option);
OPTION_INLINE int optionArg(Option option);
OPTION_INLINE char *optionValue(Option option);
OPTION_INLINE OptionEntry *optionEntry(Option option);

/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

extern Boolean optionScan(Option option, const char *progName,
	    const char *extraArgs);
extern char optionNext(Option option);
extern void optionUsage(Option option, const char *progname,
	    const char *cmdargs);
extern void optionFree(Option option);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(OPTION_BODY)
#define	OPTION_HEADER
#include "cobjs/Option.c"
#undef	OPTION_HEADER
#endif		   /* defined(DO_INLINING) && !defined(OPTION_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_OPTION_H */
