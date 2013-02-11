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
 * $(@)Log.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)Log.c 1.8	99/12/10 SMI"

#include <ctype.h>
#include <libintl.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synch.h>
#include <time.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>

#include "cobjs/Package.h"

#include "cobjs/Log.h"

#define	LOG_BUF_LEN	256

static void         stampInit(void);
static void	    logPrintFunc(const char *fmt, va_list ap);
static void         logDoPrint(const char *fmt, ...);

static Boolean      isStampInit = FALSE;
static LogPrintFunc printFunc = logPrintFunc;

static mutex_t      stampMutex;
static hrtime_t     baseTime;
static hrtime_t     lastTime;
static u32	    debugMask;
static u32	    stampMask;

const char *logAbortMsg = "ABORT";
const char *logAssertMsg = "ASSERT FAILED";
const char *logIfErrnoMsg = "STMT FAILED";

void
logSetDebugMask(char *mask)
{
    unsigned c;

    debugMask = 0;
    stampMask = 0;

    while ((c = *(unsigned char *)mask++) != 0) {
	unsigned bit;
	if (isupper(c)) {
	    if ((bit = c - 'A') >= 32) {
		logPrint("Illegal stamp mask flag: %c", c);
		continue;
	    }
	    stampMask |= 1 << bit;
	} else {
	    if ((bit = c - 'a') >= 32) {
		logPrint("Illegal debug mask flag: %c", c);
		continue;
	    }
	    debugMask |= 1 << bit;
	}
    }
}

void
logSetPrintFunc(LogPrintFunc func)
{
    printFunc = func;
}

void
logSetTextDomain(const char *textDomain)
{
    char pathBuf[MAXPATHLEN + 1];

    /*
     * Set up internationalization stuff for commands
     */
    (void) setlocale(LC_ALL, "");

    /*
     * Look in <PACKAGEHOME>/lib/locale first (the "correct" place), then
     * look in <PACKAGEHOME>/locale second (the backward compatable place)
     */
    if (packageHome(NULL, "../lib/locale", pathBuf) != NULL
	    || packageHome(NULL, "../locale", pathBuf) != NULL) {
	(void) bindtextdomain(textDomain, pathBuf);
    }
    (void) textdomain(textDomain);
}

void
logLibSetTextDomain(const char *textDomain, const char *component)
{
    char pathBuf[MAXPATHLEN + 1];

    /*
     * Set up internationalization stuff for libs
     * If can't find this component in dlinfo, try using the main
     * program (perhaps the component is an .a archive).
     *
     * Look in <PACKAGEHOME>/lib/locale first (the "correct" place), then
     * look in <PACKAGEHOME>/locale second (the backward compatable place)
     */
    if (packageHome(component, "../lib/locale", pathBuf) != NULL
	    || packageHome(component, "../locale", pathBuf) != NULL
	    || packageHome(NULL, "../lib/locale", pathBuf) != NULL
	    || packageHome(NULL, "../locale", pathBuf) != NULL) {
	(void) bindtextdomain(textDomain, pathBuf);
    }
}

void
logPrint(const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    va_start(ap, fmt);
    (void) vsnprintf(buf, sizeof(buf), gettext(fmt), ap);
    va_end(ap);

    logDoPrint("%s\n", buf);
}

void
logLibPrint(const char *textDomain, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    va_start(ap, fmt);
    (void) vsnprintf(buf, sizeof(buf), dgettext(textDomain, fmt), ap);
    va_end(ap);

    logDoPrint("%s\n", buf);
}

void
logPrintn(const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    va_start(ap, fmt);
    (void) vsnprintf(buf, sizeof(buf), gettext(fmt), ap);
    va_end(ap);

    logDoPrint("%s", buf);
}

void
logLibPrintn(const char *textDomain, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    va_start(ap, fmt);
    (void) vsnprintf(buf, sizeof(buf), dgettext(textDomain, fmt), ap);
    va_end(ap);

    logDoPrint("%s", buf);
}

void
logError(LogErrorType errorType, int error, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), gettext(fmt), ap);
    size = sizeof(buf) - len;
    va_end(ap);

    if (size > 0) {
	switch (errorType) {
	case LOG_ERROR_TYPE_NONE:
	    break;
	case LOG_ERROR_TYPE_ERRNO:
	    (void) snprintf(&buf[len], size, ": %s", strerror(error));
	    break;
	default:
	    (void) snprintf(&buf[len], size, dgettext(cobjs_TEXT_DOMAIN,
						      ": Error %d"), error);
	    break;
	}
    }
    logDoPrint("%s\n", buf);
}

void
logLibError(const char *textDomain, LogErrorType errorType, int error,
	const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), dgettext(textDomain, fmt), ap);
    size = sizeof(buf) - len;
    va_end(ap);

    if (size > 0) {
	switch (errorType) {
	case LOG_ERROR_TYPE_NONE:
	    break;
	case LOG_ERROR_TYPE_ERRNO:
	    (void) snprintf(&buf[len], size, ": %s", strerror(error));
	    break;
	default:
	    (void) snprintf(&buf[len], size,
			    dgettext(cobjs_TEXT_DOMAIN, ": Error %d"), error);
	    break;
	}
    }
    logDoPrint("%s\n", buf);
}

void
logErrno(const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), gettext(fmt), ap);
    size = sizeof(buf) - len;
    va_end(ap);

    if (size > 0) {
	(void) snprintf(&buf[len], size, ": %s", strerror(errno));
    }

    logDoPrint("%s\n", buf);
}

void
logLibErrno(const char *textDomain, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), dgettext(textDomain, fmt), ap);
    size = sizeof(buf) - len;
    va_end(ap);

    if (size > 0) {
	(void) snprintf(&buf[len], size, ": %s", strerror(errno));
    }

    logDoPrint("%s\n", buf);
}

void
_logAbort(LogErrorType errorType, int error, const char *typeMsg,
	 const char *itemMsg, const char *file, int line)
{
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    len = snprintf(buf, sizeof(buf), "%s  %s@%d: %s",
	    dgettext(cobjs_TEXT_DOMAIN, typeMsg), file, line, itemMsg);
    size = sizeof(buf) - len;

    if (size > 0) {
	switch (errorType) {
	case LOG_ERROR_TYPE_NONE:
	    break;
	case LOG_ERROR_TYPE_ERRNO:
	    (void) snprintf(&buf[len], size, ": %s", strerror(error));
	    break;
	default:
	    (void) snprintf(&buf[len], size,
			    dgettext(cobjs_TEXT_DOMAIN, ": Error %d"), error);
	    break;
	}
    }
    logDoPrint("%s\n", buf);
    abort();
}

void
logDebug(int flag, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    if ((debugMask & (1 << (tolower(flag) - 'a'))) != 0) {
	/*
	 * Don't translate debug messages
	 */
	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	logDoPrint("%s\n", buf);
    }
}

void
logDebugC(Boolean doPrint, int flag, const char *fmt,...)
{
    va_list             ap;
    char		buf[LOG_BUF_LEN];

    if (doPrint && (debugMask & (1 << (tolower(flag) - 'a'))) != 0) {
	/*
	 * Don't translate debug messages
	 */
	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	logDoPrint("%s\n", buf);
    }
}

void
logStamp(int flag, const char *fmt,...)
{
    va_list             ap;
    hrtime_t	        thisTime;
    hrtime_t            diffTime;
    char		buf[LOG_BUF_LEN];
    int			len;
    ssize_t		size;

    if ((stampMask & (1 << (tolower(flag) - 'a'))) == 0) {
	return;
    }
    if (!isStampInit) {
	stampInit();
    }
    CHECK_IF_ERRNO(mutex_lock(&stampMutex));
    thisTime = gethrtime() - baseTime;
    diffTime = thisTime - lastTime;
    lastTime = thisTime;

    len = snprintf(buf, sizeof(buf), NO_LOCALE("%12.6f (%10.6f) "),
	thisTime / 1000000000.0, diffTime / 1000000000.0);
    size = sizeof(buf) - len;

    va_start(ap, fmt);
    /*
     * Don't translate stamp messages
     */
    (void) vsnprintf(&buf[len], size, fmt, ap);
    va_end(ap);

    logDoPrint("%s\n", buf);
    CHECK_IF_ERRNO(mutex_unlock(&stampMutex));
}

static void
logDoPrint(const char *fmt, ...)
{
    va_list             ap;

    va_start(ap, fmt);
    (*printFunc)(fmt, ap);
    va_end(ap);
}

static void
logPrintFunc(const char *fmt, va_list ap)
{
    (void) vprintf(fmt, ap);
    (void) fflush(stdout);
}

static void
stampInit(void)
{
    ABORT_IF_ERRNO(mutex_init(&stampMutex, USYNC_THREAD, 0));
    baseTime = gethrtime();
    isStampInit = TRUE;
}
