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
 * $(@)Log.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

#ifndef	_COBJS_LOG_H
#define _COBJS_LOG_H

#pragma ident "@(#)Log.h 1.6	99/10/27 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/
#ifdef	DEBUG
#define	ASSERTS	    1
#endif	/* DEBUG */

typedef	void        (*LogPrintFunc)(const char *fmt, va_list ap);

typedef	enum	LogErrorType {
    LOG_ERROR_TYPE_NONE,			/* no error code */
    LOG_ERROR_TYPE_ERRNO			/* error code is errno */
} LogErrorType;

extern void	    logSetDebugMask(char *mask);
extern void         logSetPrintFunc(LogPrintFunc func);
extern void	    logSetTextDomain(const char *textDomain);
extern void	    logLibSetTextDomain(const char *textDomain,
			const char *component);

/* PRINTFLIKE1 */
extern void         logPrint(const char *fmt,...);
/* PRINTFLIKE2 */
extern void         logLibPrint(const char *textDomain, const char *fmt,...);

/* PRINTFLIKE1 */
extern void         logPrintn(const char *fmt,...);
/* PRINTFLIKE2 */
extern void         logLibPrintn(const char *textDomain, const char *fmt,...);

/* PRINTFLIKE1 */
extern void         logErrno(const char *fmt,...);
/* PRINTFLIKE2 */
extern void         logLibErrno(const char *textDomain, const char *fmt,...);

/* PRINTFLIKE3 */
extern void
logError(LogErrorType errorType, int error,
	 const char *fmt,...);
/* PRINTFLIKE4 */
extern void
logLibError(const char *textDomain, LogErrorType errorType, int error,
	 const char *fmt,...);

/* PRINTFLIKE2 */
extern void         logDebug(int flag, const char *fmt,...);

/* PRINTFLIKE3 */
extern void         logDebugC(Boolean doPrint, int flag, const char *fmt,...);


/* PRINTFLIKE2 */
extern void         logStamp(int flag, const char *fmt,...);

extern void
_logAbort(LogErrorType errorType, int error, const char *typeMsg,
	 const char *itemMsg, const char *file, int line);

#ifdef	ASSERTS

#define	ASSERT(cond)							\
	((void) ((cond) 						\
	 || (_logAbort(LOG_ERROR_TYPE_NONE, 0, logAssertMsg,		\
		NO_LOCALE(#cond), NO_LOCALE(__FILE__), __LINE__), 0)))

#define	CHECK_IF_ERRNO(exp)						\
        BEGIN_STMT							\
	int __err = (exp);						\
	if (__err != 0 && __err != EINTR) {				\
	    _logAbort(LOG_ERROR_TYPE_ERRNO, __err, logIfErrnoMsg,	\
		NO_LOCALE(#exp), NO_LOCALE(__FILE__), __LINE__);	\
	}								\
	END_STMT

#else	/* ASSERTS */

#define	ASSERT(cond)

#define	CHECK_IF_ERRNO(exp)						\
        BEGIN_STMT							\
	(void) (exp);							\
	END_STMT

#endif	/* ASSERTS */

#ifdef	DEBUG
/*
 * Call logDebug() or logStamp() directly for tracing/debugging that should
 * be in all versions (optimized, debug, etc).
 * 
 * Use LOG_DBG() or LOG_STMP() for tracing/debugging that should only be
 * in debug version.
 *
 * FIXME: Do flag check here?
 */
#define	LOG_DBG(flag_msg)						\
	BEGIN_STMT							\
	    logDebug flag_msg;						\
	END_STMT

#define	LOG_DBGC(cond_flag_msg)						\
	BEGIN_STMT							\
	    logDebugC cond_flag_msg;					\
	END_STMT

#define	LOG_STMP(flag_msg)						\
	BEGIN_STMT							\
	    logStamp flag_msg;						\
	END_STMT

#else					   /* DEBUG */

#define	LOG_DBG(flag_msg)
#define	LOG_DBGC(flag_msg)
#define	LOG_STMP(flag_msg)

#endif					   /* DEBUG */

/*
 * Macros to implement various ugly aborts
 */
#define	ABORT(msg)							\
	BEGIN_STMT							\
	_logAbort(LOG_ERROR_TYPE_NONE, 0, logAbortMsg,			\
		NO_LOCALE(msg), NO_LOCALE(__FILE__), __LINE__);		\
	END_STMT

#define	ABORT_IF_ERRNO(exp)						\
        BEGIN_STMT							\
	int __err = (exp);						\
	if (__err != 0) {						\
	    _logAbort(LOG_ERROR_TYPE_ERRNO, __err, logIfErrnoMsg,	\
		NO_LOCALE(#exp), NO_LOCALE(__FILE__), __LINE__);	\
	}								\
	END_STMT

#define	ABORT_ON_ERRNO(exp)						\
        BEGIN_STMT							\
	if ((exp) == -1) {						\
	    _logAbort(LOG_ERROR_TYPE_ERRNO, errno, logIfErrnoMsg,	\
		NO_LOCALE(#exp), NO_LOCALE(__FILE__), __LINE__);	\
	}								\
	END_STMT

#define	ABORT_WITH_ERRNO(error, msg)					\
        BEGIN_STMT							\
	_logAbort(LOG_ERROR_TYPE_ERRNO, error, logIfErrnoMsg,		\
	    NO_LOCALE(msg), NO_LOCALE(__FILE__), __LINE__);		\
	END_STMT

#define	ABORT_IF_FALSE(cond)						\
	((void) ((cond) 						\
	 || (_logAbort(LOG_ERROR_TYPE_NONE, 0, logAssertMsg,		\
		NO_LOCALE(#cond), NO_LOCALE(__FILE__), __LINE__), 0)))

extern const char *logAbortMsg;
extern const char *logAssertMsg;
extern const char *logIfErrnoMsg;

_FOREIGN_END

#endif					   /* _COBJS_LOG_H */
