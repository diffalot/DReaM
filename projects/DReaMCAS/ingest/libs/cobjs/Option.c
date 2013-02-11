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
 * $(@)Option.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Option.c -- Description.
 */

#pragma ident "@(#)Option.c 1.4	99/08/27 SMI"

#if	!defined(OPTION_HEADER)
#define	OPTION_BODY
#define	OPTION_INLINE		extern
#include "cobjs/Option.h"
#endif					   /* !defined(OPTION_HEADER) */

#include <stdlib.h>
#include <ctype.h>
#include <libintl.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*************************************************************************
 * Instance Variables
 *************************************************************************/

struct _Option {
    int			argc;
    const char * const	*argv;
    OptionEntry		*table;
    OptionEntry		*entryp;
    int			flag;		   /* option character that failed */
    int			arg;		   /* next option to parse */
    const char		*argp;		   /* position in arg */
    const char		*argval;
};

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void optionInlinePrivate(void);
 */

/*************************************************************************
 * Private class data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

OPTION_INLINE int
optionFlag(Option option)
{
    return option->flag;
}

OPTION_INLINE int
optionArg(Option option)
{
    return option->arg;
}

OPTION_INLINE char *
optionValue(Option option)
{
    return (char *)option->argval;
}

OPTION_INLINE OptionEntry *
optionEntry(Option option)
{
    return option->entryp;
}

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(OPTION_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private method prototypes
 *************************************************************************/

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

/*************************************************************************
 * Private class data
 *************************************************************************/

/*************************************************************************
 * Class Methods
 *************************************************************************/

/*
 * lint won't let you pass a "char **argv" to "const char * const *argv"
 * So, we give up and declare this char ** and then cast on the
 * instance var assignment.
 */
Option
optionNew(int argc, char **argv, OptionEntry table[])
{
    Option option = NEW_ZEROED(struct _Option, 1);

    option->argc = argc;
    option->argv = (const char * const *)argv;
    option->table = table;
    option->entryp = NULL;
    option->flag = 'a';
    option->arg = 1;
    option->argp = "";
    option->argval = NULL;
    return option;
}

/*************************************************************************
 * Instance Methods
 *************************************************************************/

Boolean
optionScan(Option option, const char *progName, const char *extraArgs)
{
    char flag = '\0';	/* lint */
    Boolean isSuccess = TRUE;

    while (isSuccess && (flag = optionNext(option)) != '\0') {
	switch (flag) {
	case '#':
	    switch (optionEntry(option)->type) {
	    case OPTION_TYPE_INT:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: %s not legal integer value for flag %c\n",
		    optionValue(option), optionFlag(option));
		break;
	    case OPTION_TYPE_DBL:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: %s not legal floating point value for flag %c\n",
		    optionValue(option), optionFlag(option));
		break;
	    default:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: %s not legal value for flag %c\n",
		    optionValue(option), optionFlag(option));
		break;
	    }
	    optionUsage(option, progName, extraArgs);
	    isSuccess = FALSE;
	    break;
	case '?':
	    logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: flag %c not legal\n", optionFlag(option));
	    optionUsage(option, progName, extraArgs);
	    isSuccess = FALSE;
	    break;
	case '!':
	    switch (optionEntry(option)->type) {
	    case OPTION_TYPE_INT:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: flag %c requires integer value\n",
		    optionFlag(option));
		break;
	    case OPTION_TYPE_DBL:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: flag %c requires floating point value\n",
		    optionFlag(option));
		break;
	    default:
		logLibPrint(cobjs_TEXT_DOMAIN,
		    "FATAL: flag %c requires value\n",
		    optionFlag(option));
		break;
	    }
	    optionUsage(option, progName, extraArgs);
	    isSuccess = FALSE;
	    break;
	case '@':
	    optionUsage(option, progName, extraArgs);
	    isSuccess = FALSE;
	    break;
	}
    }
    return isSuccess;
}

char
optionNext(Option option)
{
    OptionEntry *entryp;
    double dval;
    int ival;
    char *cp;

    option->argval = NULL;
    option->entryp = NULL;

    ABORT_IF_FALSE(option->flag != '\0');
    if ((option->flag = *(const u8 *)option->argp++) == '\0') {
	if (option->arg >= option->argc
		|| option->argv[option->arg] == NULL
		|| option->argv[option->arg][0] != '-') {
	    return '\0';
	}
	option->argp = &option->argv[option->arg++][1];
	if ((option->flag = *(const u8 *)option->argp++) == '\0') {
	    return '\0';
	}
	if (option->flag == '-' && *((const u8 *)option->argp) == '\0') {
	    option->flag = '\0';
	    option->arg += 1;
	    return '\0';
	}
    }

    for (entryp = option->table; entryp->flag != '\0'; entryp++) {
	if (entryp->flag == option->flag
	    || (entryp->type == OPTION_TYPE_BOOLEAN
		&& entryp->flag == tolower(option->flag))) {
	    break;
	}
    }
    if (entryp->flag == '\0') {
	return '?';
    }
    option->entryp = entryp;
    switch (entryp->type) {
    case OPTION_TYPE_FLAG:
	*(Boolean *) entryp->ptr = TRUE;
	break;
    case OPTION_TYPE_BOOLEAN:
	*(Boolean *) entryp->ptr = (Boolean)islower(option->flag);
	break;
    case OPTION_TYPE_STRING:
    case OPTION_TYPE_INT:
    case OPTION_TYPE_DBL:
	if (option->arg >= option->argc) {
	    return '!';
	}
	option->argval = option->argv[option->arg++];
	switch (entryp->type) {
	case OPTION_TYPE_STRING:
	    *(char **) entryp->ptr = (char *)option->argval;
	    break;
	case OPTION_TYPE_INT:
	    ival = strtol(option->argval, &cp, 0);
	    if (*cp != '\0') {
		return '#';
	    }
	    *(int *) entryp->ptr = ival;
	    break;
	case OPTION_TYPE_DBL:
	    dval = strtod(option->argval, &cp);
	    if (*cp != '\0') {
		return '#';
	    }
	    *(double *) entryp->ptr = dval;
	    break;
	}
	break;
    case OPTION_TYPE_USAGE:
	return '@';
    default:
	ABORT("illegal OptionTable type");
    }
    return option->flag;
}

void
optionUsage(Option option, const char *progname, const char *cmdargs)
{
    OptionEntry *entryp;

    if (progname == NULL || progname[0] == '\0') {
	if (option->argv[0] == NULL) {
	    progname = "<Unknown>";
	} else {
	    progname = strrchr(option->argv[0], '/');
	    if (progname == NULL) {
		progname = option->argv[0];
	    } else {
		progname += 1;
	    }
	}
    }
    logLibPrint(cobjs_TEXT_DOMAIN, "Usage: %s [<flags>] %s", progname,
	    gettext(cmdargs));
    logLibPrint(cobjs_TEXT_DOMAIN, "   Flags are:");
    for (entryp = option->table; entryp->flag != '\0'; entryp++) {
	switch (entryp->type) {
	case OPTION_TYPE_FLAG:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c]  {%s}"),
		entryp->flag, gettext(entryp->desc));
	    break;
	case OPTION_TYPE_USAGE:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c]  {%s}"),
		entryp->flag, gettext(entryp->desc));
	    break;
	case OPTION_TYPE_BOOLEAN:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c%c] {%s: %c}"),
		entryp->flag, toupper(entryp->flag), gettext(entryp->desc),
		*(Boolean *)entryp->ptr ? entryp->flag : toupper(entryp->flag));
	    break;
	case OPTION_TYPE_STRING:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c]  {%s: \"%s\"}"),
		entryp->flag, gettext(entryp->desc),
		*(char **)entryp->ptr != NULL ? *(char **) entryp->ptr
		    : dgettext(cobjs_TEXT_DOMAIN, "<NULL>"));
	    break;
	case OPTION_TYPE_INT:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c]  {%s: %d}"),
		entryp->flag, gettext(entryp->desc), *(int *) entryp->ptr);
	    break;
	case OPTION_TYPE_DBL:
	    logLibPrint(cobjs_TEXT_DOMAIN,
		NO_LOCALE("       [-%c]  {%s: %.3f}"),
		entryp->flag, gettext(entryp->desc), *(double *) entryp->ptr);
	    break;
	default:
	    ABORT("illegal OptionTable type");
	}
    }
}

void
optionFree(Option option)
{
    free(option);
}

/*************************************************************************
 * Private Methods
 *************************************************************************/

/*
 * Non-inlinable called from inline
 *
 * INLINE_PRIVATE void
 * optionInlinePrivate(void)
 * {
 * }
 *
 * static void
 * optionPrivate(void)
 * {
 * }
 */

/*************************************************************************
 * Private Functions
 *************************************************************************/

/* 
 * static void
 * optionFunc()
 * {
 * }
 */

#endif					   /* !defined(OPTION_HEADER) */
