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
 * $(@)Package.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 */

/*
 * Package.c -- Description of Package
 *
 * This file generated from Tmplate version: 1.3 98/11/18
 */


#pragma ident "@(#)Package.c 1.2	99/09/02 SMI"

#if	!defined(PACKAGE_HEADER)
#define	PACKAGE_BODY
#define	PACKAGE_INLINE		extern
#include "cobjs/Package.h"
#endif					   /* !defined(PACKAGE_HEADER) */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/link.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cobjs/Macros.h"
#include "cobjs/Types.h"
#include "cobjs/Log.h"
#include "cobjs/String.h"

/*************************************************************************
 * Defines
 *************************************************************************/

/*************************************************************************
 * Private types and prototypes referenced from inlines
 *************************************************************************/

/*
 * Use INLINE_PRIVATE if non-inline-able, define in Non-inlinable section
 * Use static if inline-able, define in Private Inline-able section
 *
 * INLINE_PRIVATE void packageInlinePrivate(void);
 */

/*************************************************************************
 * Private data referenced from inlines
 *************************************************************************/

/*************************************************************************
 * Inline Methods
 *************************************************************************/

/*
 * PACKAGE_INLINE void *
 * packageInlineRoutine(void)
 * {
 * }
 */

/*************************************************************************
 * Private Inlineable Methods and Functions Called From Inlines
 *************************************************************************/

#if	!defined(PACKAGE_HEADER)

/*************************************************************************
 * Private types
 *************************************************************************/

/*************************************************************************
 * Private function prototypes
 *************************************************************************/

/*************************************************************************
 * Private class data
 *************************************************************************/

/*************************************************************************
 * Public Functions
 *************************************************************************/

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
char *
packageHome(const char *component, const char *relPathToHome, char *pathBuf)
{
    String comp = stringNew(NULL);
    struct stat st;
    char *hdp = NULL;
    void *handle;
    Link_map *lmp;

    /*
     * Use dlinfo() to find path to component
     */
    if ((handle = dlopen(NULL, RTLD_LAZY)) == NULL) {
	logLibPrint(cobjs_TEXT_DOMAIN, "dlopen: %s", dlerror());
	goto done;
    }
    if (dlinfo(handle, RTLD_DI_LINKMAP, &lmp) != 0) {
	logLibPrint(cobjs_TEXT_DOMAIN, "dlinfo: %s", dlerror());
	goto done;
    }

    if (component != NULL) {
	/*
	 * Locate library in dlinfo
	 */
	while (lmp != NULL) {
	    stringFormat(comp, "%s", lmp->l_name);
	    if (strcmp(stringFilePart(comp), component) == 0) {
		break;
	    }
	    lmp = lmp->l_next;
	}
	if (lmp == NULL) {
	    goto done;
	}
    } else {
	/*
	 * Main program (always first element returned by dlinfo).
	 * Search path if necessary
	 */
	stringFormat(comp, "%s", lmp->l_name);
	/*
	 * If comp doesn't include a slash, it was located via the PATH.
	 * Find the appropriate path element and prepend it.
	 */
	if (*stringDirectoryPart(comp) == '\0') {
	    char *pdir;
	    unsigned sPos;
	    String path = stringNew(getenv("PATH"));

	    /*
	     * component must have been found via path
	     * figure out which path element points at directory
	     * containing component
	     */
	    sPos = STRING_POS_BEGINNING;
	    while ((pdir = stringNextComponent(path, ":", sPos, &sPos))
		    != NULL) {
		char *file = stringFilePart(comp);

		stringFormat(comp, "%s/%s", pdir, file);
		if (access(stringValue(comp), X_OK) == 0
			&& stat(stringValue(comp), &st) == 0
			&& S_ISREG(st.st_mode)) {
		    break;
		}
	    }
	    stringFree(path);
	    if (pdir == NULL) {
		stringFormat(comp, "%s", stringFilePart(comp));
		logLibPrint(cobjs_TEXT_DOMAIN,
			"packageHome: %s: couldn't determine exec dir",
			stringValue(comp));
		goto done;
	    }
	}
    }

    /*
     * Resolve any symlinks.
     */
    if (realpath(stringValue(comp), pathBuf) == NULL) {
	logLibErrno(cobjs_TEXT_DOMAIN, "realPath: %s", stringValue(comp));
    }
    stringFormat(comp, "%s", pathBuf);

    /*
     * Append relPathToHome to directory part.
     * Resolve path once more.  Ignore errors, since sometimes
     * relPathToHome may not name a real directory.
     */
    stringFormat(comp, "%s/%s", stringDirectoryPart(comp), relPathToHome);
    (void) realpath(stringValue(comp), pathBuf);
    stringFormat(comp, "%s", pathBuf);

    hdp = strcpy(pathBuf, stringValue(comp));
done:
    if (handle != NULL && dlclose(handle) != 0) {
	logLibPrint(cobjs_TEXT_DOMAIN, "dlclose: %s", dlerror());
    }
    stringFree(comp);
    return hdp;
}

char *
packageCommand(const char *_argv0, char *pathBuf)
{
    String argv0 = stringNew(_argv0);
    struct stat st;
    char *hdp = NULL;

    if (stringLength(argv0) == 0) {
	void *handle;
	Link_map *lmp;
	/*
	 * Use current program
	 */
	if ((handle = dlopen(NULL, RTLD_LAZY)) == NULL) {
	    logLibPrint(cobjs_TEXT_DOMAIN, "dlopen: %s", dlerror());
	    goto done;
	}
	if (dlinfo(handle, RTLD_DI_LINKMAP, &lmp) != 0) {
	    logLibPrint(cobjs_TEXT_DOMAIN, "dlinfo: %s", dlerror());
	    goto done;
	}
	stringFormat(argv0, "%s", lmp->l_name);
	if (handle != NULL && dlclose(handle) != 0) {
	    logLibPrint(cobjs_TEXT_DOMAIN, "dlclose: %s", dlerror());
	}
    }

    /*
     * If argv0 doesn't include a slash, it was located via the PATH.
     * Find the appropriate path element and prepend it.
     */
    if (*stringDirectoryPart(argv0) == '\0') {
	char *pdir;
	unsigned sPos;
	String path = stringNew(getenv("PATH"));

	/*
	 * program must have been found via path
	 * figure out which path element points at directory
	 * containing program
	 */
	sPos = STRING_POS_BEGINNING;
	while ((pdir = stringNextComponent(path, ":", sPos, &sPos))
		!= NULL) {
	    char *file = stringFilePart(argv0);

	    stringFormat(argv0, "%s/%s", pdir, file);
	    if (access(stringValue(argv0), X_OK) == 0
		    && stat(stringValue(argv0), &st) == 0
		    && S_ISREG(st.st_mode)) {
		break;
	    }
	}
	stringFree(path);
	if (pdir == NULL) {
	    stringFormat(argv0, "%s", stringFilePart(argv0));
	    logLibPrint(cobjs_TEXT_DOMAIN,
		    "packageHome: %s: couldn't determine exec dir",
		    stringValue(argv0));
	    goto done;
	}
    }

    /*
     * Resolve any symlinks.
     */
    if (realpath(stringValue(argv0), pathBuf) == NULL) {
	logLibErrno(cobjs_TEXT_DOMAIN, "realPath: %s", stringValue(argv0));
    }
    stringFormat(argv0, "%s", pathBuf);

    hdp = strcpy(pathBuf, stringValue(argv0));
done:
    stringFree(argv0);
    return hdp;
}

/*************************************************************************
 * Private Functions
 *************************************************************************/

#endif					   /* !defined(PACKAGE_HEADER) */
