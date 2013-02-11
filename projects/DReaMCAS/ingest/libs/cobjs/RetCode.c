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
 * $(@)RetCode.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

#pragma	ident "@(#)RetCode.c 1.6	99/08/26 SMI"

/*
 * RetCode.c -- RetCode handling routines
 */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <thread.h>
#include <synch.h>
#include <stdio.h>

#include "cobjs/ArrayOf.h"
#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

#include "cobjs/RetCode.h"

/*
 * id 0 is reserved for syscall errors
 * id 1 is reserved for client side errors
 */
#define	BASE_ID			2

#define	FIELD_SIZE		30

/*************************************************************************
 * Private class data
 *************************************************************************/

typedef struct RetCodeEntry {
    const char         *className;
    RetCodeId           id;
    Boolean             hasTable;
    RetCodeTable       *table;
    RetCodeFunc         func;
} RetCodeEntry;

static ArrayOf      retCodeArray;
static u16          currentId = BASE_ID;
static thread_key_t retCodeBufKey;
static mutex_t      retCodeMutex;
static Boolean	    retCodeIsFrozen = FALSE;
static char fill[] = "                                                      ";
static int fillSize = sizeof(fill) - 1;

static RetCodeTable retCodeGlobal[] = {
    {RETCODE_FAILED, "FAILED", "unspecified failure"},
    {0, NULL}
};

/*************************************************************************
 * OBJECT Template Class Methods
 *************************************************************************/


static void
retCodeInit(void)
{
    RetCodeEntry        entry;

    ABORT_IF_ERRNO(mutex_init(&retCodeMutex, USYNC_THREAD, 0));
    ABORT_IF_ERRNO(thr_keycreate(&retCodeBufKey, free));

    retCodeArray = NEW_ARRAY(RetCodeEntry);

    entry.className = strdup("RetCode Global");
    entry.id = currentId++;
    entry.hasTable = TRUE;
    entry.table = retCodeGlobal;
    entry.func = NULL;

    arrayOfItemAt(retCodeArray, RetCodeEntry, entry.id) = entry;
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma init(retCodeInit)
#elif defined (__GNUC__)
static void retCodeInit(void) __attribute__ ((constructor));
#endif


static void
retCodeFini(void)
{
    int i;
    int len = arrayOfLength(retCodeArray);

    for (i = BASE_ID; i < len; i++) {
	free((void *)arrayOfItemAt(retCodeArray, RetCodeEntry, i).className);
    }
    arrayOfFree(retCodeArray);
}

#if defined(__SUNPRO_C)  || defined(__SUNPRO_CC)
#pragma fini(retCodeFini)
#elif defined(__GNUC__)
static void retCodeFini(void) __attribute__ ((destructor));
#endif


RetCodeId
retCodeRegisterWithTable(const char *className, RetCodeTable *table)
{
    RetCodeEntry        entry;
    RetCodeTable	*rctp;
    int                 i;

    CHECK_IF_ERRNO(mutex_lock(&retCodeMutex));
    if (retCodeIsFrozen) {
	ABORT("retCode Frozen");
    }
    for (i = BASE_ID; i < arrayOfLength(retCodeArray); i++) {
	RetCodeEntry       *entryp = (RetCodeEntry*)arrayOfItemAtByPtr(retCodeArray, i);

	if (strcmp(className, entryp->className) == 0) {
	    if (! entryp->hasTable || table != entryp->table) {
		ABORT("redefinition of retCode");
	    }
	    CHECK_IF_ERRNO(mutex_unlock(&retCodeMutex));
	    return entryp->id;
	}
    }

    entry.className = strdup(className);
    entry.id = currentId++;
    entry.hasTable = TRUE;
    entry.table = table;
    entry.func = NULL;

    for (rctp = table; rctp->msg != NULL; rctp++) {
	if (rctp->localCode == 0 || rctp->localCode > 0xffff) {
	    logLibPrint(cobjs_TEXT_DOMAIN, "invalid RetCodeTable: %s\n",
		    className);
	    ABORT("invalid table");
	}
    }

    arrayOfItemAt(retCodeArray, RetCodeEntry, entry.id) = entry;
    CHECK_IF_ERRNO(mutex_unlock(&retCodeMutex));

    return entry.id;
}

RetCodeId
retCodeRegisterWithFunc(const char *className, RetCodeFunc func)
{
    RetCodeEntry        entry;
    int                 i;

    CHECK_IF_ERRNO(mutex_lock(&retCodeMutex));
    if (retCodeIsFrozen) {
	ABORT("retCode Frozen");
    }
    for (i = BASE_ID; i < arrayOfLength(retCodeArray); i++) {
	RetCodeEntry       *entryp = (RetCodeEntry*)arrayOfItemAtByPtr(retCodeArray, i);

	if (strcmp(className, entryp->className) == 0) {
	    ASSERT(!entryp->hasTable);
	    CHECK_IF_ERRNO(mutex_unlock(&retCodeMutex));
	    return entryp->id;
	}
    }

    entry.className = strdup(className);
    entry.id = currentId++;
    entry.hasTable = FALSE;
    entry.table = NULL;
    entry.func = func;

    arrayOfItemAt(retCodeArray, RetCodeEntry, entry.id) = entry;
    CHECK_IF_ERRNO(mutex_unlock(&retCodeMutex));

    return entry.id;
}

const char         *
retCodeMsg(RetCode retCode, char *buf, size_t buflen)
{
    RetCodeEntry       *entryp;
    RetCodeId           id;
    RetCodeTable       *rctp;
    u16                 localCode;

    if (retCode == RETCODE_SUCCESS) {
	return "Success";
    }
    id = retCode >> 16;
    localCode = retCode & 0xffff;

    if (buf == NULL || buflen < 80) {
	buflen = 80;
	if (thr_getspecific(retCodeBufKey, (void **) &buf) != 0
		|| buf == NULL) {
	    buf = NEW(char, buflen);
	    ABORT_IF_ERRNO(thr_setspecific(retCodeBufKey, buf));
	}
    }
    if (id == 0 || id > (unsigned) arrayOfLength(retCodeArray)) {
	(void) sprintf(buf, "Unregistered Return Code %d:%d", id, localCode);
	return buf;
    }
    entryp = (RetCodeEntry*)arrayOfItemAtByPtr(retCodeArray, id);
    if (!entryp->hasTable) {
	return (*entryp->func) (localCode, buf, buflen);
    }
    for (rctp = entryp->table; rctp->msg != NULL; rctp++) {
	if ((rctp->localCode & 0xffff) == localCode) {
	    (void) sprintf(buf, "%.30s: %s", entryp->className, rctp->msg);
	    return buf;
	}
    }
    (void) sprintf(buf, "%.30s: Unknown return code: %d", entryp->className,
		   localCode);
    return buf;
}

void
retCodeDumpNames(const char *prefix, RetCodeDumpFunc func)
{
    int nClasses = arrayOfLength(retCodeArray);
    char buf[128];
    RetCodeId           id;
    int fieldSize = FIELD_SIZE - strlen(prefix);

    retCodeIsFrozen = TRUE;
    for (id = BASE_ID; id < nClasses; id++) {
	RetCodeEntry *entryp = (RetCodeEntry*) arrayOfItemAtByPtr(retCodeArray, id);
	if (entryp->hasTable) {
	    RetCodeTable       *rctp;
	    (*func)("/*");
	    (void) snprintf(buf, sizeof(buf), " * Errors for class %s",
			    entryp->className);
	    (*func)(buf);
	    (*func)(" */");
	    for (rctp = entryp->table; rctp->msg != NULL; rctp++) {
		int len;
		int fillLen;
		int fillPoint;
		if (rctp->codeName == NULL) {
		    continue;
		}
		fillLen = fieldSize - strlen(rctp->codeName);
		if (fillLen < 0) fillLen = 0;
		fillPoint = fillSize - fillLen;
		if (fillPoint < 0) fillPoint = 0;
		len = snprintf(buf, sizeof(buf) - 1,
			"%s%s%s = 0x%06lx,  /* %s */",
			       prefix, rctp->codeName, &fill[fillPoint],
			       (entryp->id << 16) | (rctp->localCode & 0xffff),
			       rctp->msg);
		if (len > sizeof(buf) - 1) {
		    ABORT("buf overflow");
		}
		(*func)(buf);
	    }
	}
    }
}

void
retCodeDumpMessages(const char *prefix, RetCodeDumpFunc func)
{
    int nClasses = arrayOfLength(retCodeArray);
    char buf[128];
    RetCodeId           id;
    int fieldSize = FIELD_SIZE - strlen(prefix);

    retCodeIsFrozen = TRUE;
    for (id = BASE_ID; id < nClasses; id++) {
	RetCodeEntry *entryp = (RetCodeEntry*)arrayOfItemAtByPtr(retCodeArray, id);
	if (entryp->hasTable) {
	    RetCodeTable       *rctp;
	    (*func)("/*");
	    (void) snprintf(buf, sizeof(buf), " * Errors for class %s",
			    entryp->className);
	    (*func)(buf);
	    (*func)(" */");
	    for (rctp = entryp->table; rctp->msg != NULL; rctp++) {
		int len;
		int fillLen;
		int fillPoint;
		if (rctp->codeName == NULL) {
		    continue;
		}
		fillLen = fieldSize - strlen(rctp->codeName);
		if (fillLen < 0) fillLen = 0;
		fillPoint = fillSize - fillLen;
		if (fillPoint < 0) fillPoint = 0;
		len = snprintf(buf, sizeof(buf) - 1, "{%s%s,%s \"%s\"},",
			       prefix, rctp->codeName, &fill[fillPoint],
			       rctp->msg);
		if (len > sizeof(buf) - 1) {
		    ABORT("buf overflow");
		}
		(*func)(buf);
	    }
	}
    }
}

void
retCodeFreeze(void)
{
    retCodeIsFrozen = TRUE;
}

RetCode
retCodeFor(char *className, u32 localCode)
{
    int                 i;

    ASSERT((localCode >> 16) == 0);
    for (i = BASE_ID; i < arrayOfLength(retCodeArray); i++) {
	RetCodeEntry       *entryp = (RetCodeEntry*)arrayOfItemAtByPtr(retCodeArray, i);

	if (strcmp(className, entryp->className) == 0) {
	    return RetCode(entryp->id << 16 | localCode);
	}
    }
    /*
     * Intentionally blow up
     */
    ASSERT(className == NULL);
    return RetCode(1);
}
