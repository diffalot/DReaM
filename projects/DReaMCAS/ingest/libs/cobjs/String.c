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
 * $(@)String.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:35 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

#pragma	ident "@(#)String.c 1.3	99/07/29 SMI"

/*
 * String.c -- Description
 */
#include <stdlib.h>
#include <string.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

#include "cobjs/String.h"

/*
 * MUST be non-zero
 */
#define	STRING_PAD	80

/*
 * OBJECT String Instance Variables
 */
struct _String {
    char               *strBuf;
    size_t              strBufLen;
    size_t              strLen;
    int                 rotor;
    char               *charBuf[STRING_BUFFERS];
    size_t              charBufLen[STRING_BUFFERS];
};

static char        *stringBuf(String string, size_t subLen);

/*
 * OBJECT String Class Methods
 */
String
stringNew(const char *s)
{
    String              string;

    string = NEW_ZEROED(struct _String, 1);
    if (s != NULL) {
	string->strLen = strlen(s);
	string->strBufLen = string->strLen + STRING_PAD;
	string->strBuf = NEW(char, string->strBufLen);
	(void) memcpy(string->strBuf, s, string->strLen);
    } else {
	string->strLen = 0;
	string->strBufLen = STRING_PAD;
	string->strBuf = NEW_ZEROED(char, string->strBufLen);
    }
    return string;
}

String
stringDup(String oldString)
{
    String              string;

    string = NEW_ZEROED(struct _String, 1);
    string->strLen = oldString->strLen;
    string->strBufLen = string->strLen + STRING_PAD;
    string->strBuf = NEW(char, string->strBufLen);
    (void) memcpy(string->strBuf, oldString->strBuf, string->strLen);
    return string;
}

/*
 * OBJECT String Instance Methods
 */

void
stringInsert(String string, const char *s, unsigned insertPos)
{
    size_t              insLen = s != NULL ? strlen(s) : 0;
    size_t              newStrLen = string->strLen + insLen;

    if (insertPos > string->strLen) {
	insertPos = string->strLen;
    }
    if (newStrLen > string->strBufLen) {
	size_t              newBufLen = newStrLen + STRING_PAD;
	char               *newBuf = RENEW(char, string->strBuf, newBufLen);

	string->strBufLen = newBufLen;
	string->strBuf = newBuf;
    }
    if (insertPos < string->strLen) {
	(void) memmove(&string->strBuf[insertPos + insLen],
		       &string->strBuf[insertPos], string->strLen - insertPos);
    }
    (void) memcpy(&string->strBuf[insertPos], s, insLen);
    string->strLen = newStrLen;
}

void
stringPrepend(String string, const char *s)
{
    stringInsert(string, s, STRING_POS_BEGINNING);
}

void
stringAppend(String string, const char *s)
{
    stringInsert(string, s, STRING_POS_END);
}

void
stringReplace(String string, const char *s, unsigned startPos, unsigned endPos)
{
    size_t              insLen = s != NULL ? strlen(s) : 0;
    size_t              newStrLen;
    int                 lenDiff;

    if (startPos > string->strLen) {
	startPos = string->strLen;
    }
    if (endPos >= string->strLen) {
	endPos = string->strLen;
    } else {
	endPos += 1;
    }
    if (endPos < startPos) {
	endPos = startPos;
    }
    newStrLen = string->strLen + insLen - (endPos - startPos);
    if (newStrLen >= string->strBufLen) {
	size_t              newBufLen = newStrLen + STRING_PAD;
	char               *newBuf = RENEW(char, string->strBuf, newBufLen);

	string->strBufLen = newBufLen;
	string->strBuf = newBuf;
    }
    /*
     * lenDiff is > 0 if growing, < 0 if shrinking
     */
    lenDiff = newStrLen - string->strLen;
    if (lenDiff != 0) {
	(void) memmove(&string->strBuf[endPos + lenDiff],
		       &string->strBuf[endPos], string->strLen - endPos);
    }
    (void) memcpy(&string->strBuf[startPos], s, insLen);
    string->strLen = newStrLen;
}

void
stringDelete(String string, unsigned startPos, unsigned endPos)
{
    if (startPos > string->strLen) {
	startPos = string->strLen;
    }
    if (endPos >= string->strLen) {
	endPos = string->strLen;
    } else {
	endPos += 1;
    }
    if (startPos < endPos) {
	(void) memmove(&string->strBuf[startPos], &string->strBuf[endPos],
		       string->strLen - endPos);
	string->strLen -= endPos - startPos;
    }
}

/***
 * Returns specified substring in a string-specific buffer. NOTE: There are
 * actually STRING_BUFFERS of these, so things like this will work:
 *	someRoutine(stringSubstring(aString, 0, 3),
 *		    stringSubstring(aString, 4, 6));
 */
char               *
stringSubstring(String string, unsigned startPos, unsigned endPos)
{
    char               *buf;
    size_t              subLen;

    if (startPos > string->strLen) {
	startPos = string->strLen;
    }
    if (endPos >= string->strLen) {
	endPos = string->strLen;
    } else {
	endPos += 1;
    }
    if (endPos < startPos) {
	endPos = startPos;
    }
    subLen = endPos - startPos;
    buf = stringBuf(string, subLen);
    if (subLen != 0) {
	(void) memcpy(buf, &string->strBuf[startPos], subLen);
    }
    buf[subLen] = '\0';
    return buf;
}

char               *
stringValue(String string)
{
    char *buf = stringBuf(string, string->strLen);

    if (string->strLen != 0) {
	(void) memcpy(buf, string->strBuf, string->strLen);
    }
    buf[string->strLen] = '\0';
    return buf;
}

unsigned
stringFindLeft(String string, const char c, unsigned startPos)
{
    char               *cp;
    char               *endCp = &string->strBuf[string->strLen];

    if (startPos < string->strLen) {
	for (cp = &string->strBuf[startPos]; cp < endCp; cp++) {
	    if (*cp == c) {
		return cp - string->strBuf;
	    }
	}
    }
    return STRING_NO_MATCH;
}

unsigned
stringFindRight(String string, char c, unsigned startPos)
{
    char               *cp;

    if (startPos >= string->strLen) {
	startPos = string->strLen - 1;
    }
    for (cp = &string->strBuf[startPos]; cp >= string->strBuf; cp--) {
	if (*cp == c) {
	    return cp - string->strBuf;
	}
    }
    return STRING_NO_MATCH;
}

unsigned
stringMatchLeft(String string, const char *s, unsigned matchLen,
	unsigned startPos)
{
    size_t              patternLen = strlen(s);

    if (matchLen > patternLen) {
	matchLen = patternLen;
    }
    if (startPos < string->strLen - matchLen + 1) {
	char               *cp;
	char               *endCp
		= &string->strBuf[string->strLen - matchLen + 1];

	for (cp = &string->strBuf[startPos]; cp < endCp; cp++) {
	    if (strncmp(cp, s, matchLen) == 0) {
		return cp - string->strBuf;
	    }
	}
    }
    return STRING_NO_MATCH;
}

unsigned
stringMatchRight(String string, const char *s, unsigned matchLen,
	unsigned startPos)
{
    size_t              patternLen = strlen(s);
    char               *cp;

    if (matchLen > patternLen) {
	matchLen = patternLen;
    }
    if (startPos >= string->strLen) {
	startPos = string->strLen - 1;
    }
    if (startPos + matchLen > string->strLen) {
	startPos = string->strLen - matchLen;
    }
    for (cp = &string->strBuf[startPos]; cp >= string->strBuf; cp--) {
	if (strncmp(cp, s, matchLen) == 0) {
	    return cp - string->strBuf;
	}
    }
    return STRING_NO_MATCH;
}

unsigned
stringScan(String string, const char *s, unsigned startPos)
{
    char               *cp;
    char               *endCp = &string->strBuf[string->strLen];

    if (startPos < string->strLen) {
	for (cp = &string->strBuf[startPos]; cp < endCp; cp++) {
	    if (strchr(s, *(unsigned char *)cp) != NULL) {
		return cp - string->strBuf;
	    }
	}
    }
    return STRING_POS_END;
}

unsigned
stringSpan(String string, const char *s, unsigned startPos)
{
    char               *cp;
    char               *endCp = &string->strBuf[string->strLen];

    if (startPos < string->strLen) {
	for (cp = &string->strBuf[startPos]; cp < endCp; cp++) {
	    if (strchr(s, *(unsigned char *)cp) == NULL) {
		return cp - string->strBuf;
	    }
	}
    }
    return STRING_POS_END;
}

Boolean
stringHasPrefix(String string, const char *s)
{
    size_t              patLen = strlen(s);

    return Boolean(patLen <= string->strLen
	&& strncmp(string->strBuf, s, patLen) == 0);
}

Boolean
stringHasSuffix(String string, const char *s)
{
    size_t              patLen = strlen(s);

    return Boolean(patLen <= string->strLen
      && strncmp(&string->strBuf[string->strLen - patLen], s, patLen) == 0);
}

char               *
stringDirectoryPart(String string)
{
    unsigned epos = stringFindRight(string, '/', STRING_POS_END);
    return (char*)((epos == STRING_NO_MATCH) ? "" : stringSubstring(string, 0, epos));
}

char               *
stringFilePart(String string)
{
    return stringSubstring(string,
      stringFindRight(string, '/', STRING_POS_END) + 1, STRING_POS_END);
}

char		   *
stringNextComponent(String string, const char *delimiters, unsigned startPos,
	unsigned *nextPosp)
{
    unsigned spos;
    unsigned epos;
    char *component;

    if ((spos = stringSpan(string, delimiters, startPos)) != STRING_POS_END) {
	epos = stringScan(string, delimiters, spos);
	component = stringSubstring(string, spos, epos - 1);
    } else {
	epos = STRING_POS_END;
	component = NULL;
    }
    if (nextPosp != NULL) {
	*nextPosp = epos;
    }
    return component;
}

/* PRINTFLIKE2 */
void
stringFormat(String string, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    while ((string->strLen = vsnprintf(string->strBuf, string->strBufLen,
		    format, ap)) >= string->strBufLen) {
	string->strBufLen = string->strLen + STRING_PAD;
	string->strBuf = RENEW(char, string->strBuf, string->strBufLen);
    }
    va_end(ap);
}

Boolean
stringGets(String string, FILE *stream)
{
    char *cp = string->strBuf;
    int tailLen = string->strBufLen;

    string->strLen = 0;
    while (fgets(cp, tailLen, stream) != NULL) {
	string->strLen = strlen(string->strBuf);
	if (string->strBuf[string->strLen - 1] == '\n') {
	    /* newline terminated */
	    break;
	}
	if (string->strLen + STRING_PAD >= string->strBufLen) {
	    string->strBufLen = string->strLen + STRING_PAD;
	    string->strBuf = RENEW(char, string->strBuf, string->strBufLen);
	}
	cp = string->strBuf + string->strLen;
	tailLen = string->strBufLen - string->strLen;
    }
    return Boolean(string->strLen > 0);
}

char               *
stringBasePart(String string)
{
    return stringSubstring(string, 0,
		      stringFindRight(string, '.', STRING_POS_END) - 1);
}

char               *
stringSuffixPart(String string)
{
    return stringSubstring(string,
	  stringFindRight(string, '.', STRING_POS_END), STRING_POS_END);
}

size_t
stringLength(String string)
{
    return string->strLen;
}

void
stringFree(String string)
{
    int                 i;

    free(string->strBuf);
    for (i = 0; i < STRING_BUFFERS; i++) {
	if (string->charBuf[i] != NULL) {
	    free(string->charBuf[i]);
	}
    }
    free(string);
}

static char        *
stringBuf(String string, size_t subLen)
{
    char               *buf;

    if (string->charBufLen[string->rotor] < subLen + 1) {
	if (string->charBuf[string->rotor] != NULL) {
	    free(string->charBuf[string->rotor]);
	}
	string->charBuf[string->rotor] = NEW(char, subLen + 1);
	string->charBufLen[string->rotor] = subLen + 1;
    }
    buf = string->charBuf[string->rotor];
    string->rotor = (string->rotor + 1) % STRING_BUFFERS;
    return buf;
}
