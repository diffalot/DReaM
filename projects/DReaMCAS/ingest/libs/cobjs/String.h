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
 * $(@)String.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * String.h
 * 
 * A string is ...
 */

#ifndef	_COBJS_STRING_H
#define	_COBJS_STRING_H

#pragma ident "@(#)String.h 1.3	99/07/29 SMI"

/*
 * Includes
 */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

#include "cobjs/Foreign.h"
#include "cobjs/Types.h"

_FOREIGN_START

/*
 * Defines
 */
#define	STRING_POS_BEGINNING	0
#define	STRING_POS_END		UINT_MAX

#define	STRING_MATCH_ALL	UINT_MAX	/* match entire pattern */

#define	STRING_NO_MATCH		UINT_MAX

#define	STRING_BUFFERS		64

/*
 * OBJECT String Instance Type
 */
typedef struct _String *String;

/*
 * OBJECT String Public Types
 */

/*
 * OBJECT String Class Interface
 */
extern String       stringNew(const char *s);
extern String       stringDup(String string);

/*
 * OBJECT String Instance Interface
 */

/*
 * String Editing Functions. These are the only functions that change the
 * strings value.
 *
 * NOTE 1: positions are 0 based, i.e. the character at position 0
 * is the first character of the string.
 *
 * NOTE 2: endPos indicates the last character that is a part of the
 * operation.  I.e stringDelete(s, 3, 6) deletes 4 characters: char 3,
 * char 4, char 5, and char 6.
 *
 * NOTE 3: if endPos is less than startPos no characters a specified for
 * deletion or replacement.  Replacement will result in s inserted at
 * startPos. stringSubstring will return ""
 *
 * NOTE 4: NULL as insertion or replace string is legal and equivalent to ""
 */

/*
 * stringInsert -- insert s into string before character at pos
 *
 * If pos is >= string length, s is appended to end of string
 */
extern void      stringInsert(String string, const char *s, unsigned pos);

/*
 * stringPrepend() and stringAppend()
 *
 * Calls to stringInsert() with appropriate parameters.
 */
extern void	 stringPrepend(String string, const char *s);
extern void	 stringAppend(String string, const char *s);

extern void
stringReplace(String string, const char *s, unsigned startPos,
	      unsigned endPos);
extern void
stringDelete(String string, unsigned startPos,
	     unsigned endPos);

/***
 * Returns specified substring in a thread-specific buffer.
 *
 * NOTE: There are actually STRING_BUFFERS of these, so things like this
 * will work:
 *	someRoutine(stringSubstring(aString, 0, 3),
 *		    stringSubstring(aString, 4, 6));
 */
extern char        *
stringSubstring(String string, unsigned startPos,
		unsigned endPos);

/*
 * Returns string value as C string.
 */
extern char               *
stringValue(String string);

/*
 * String search functions. These functions return the position in the string
 * at which the specified character is found.
 *
 * Searches start with character specified by startPos.
 * 
 * RETURNS: Position of match or STRING_NO_MATCH.
 */
extern unsigned
stringFindLeft(String string, const char c,
		unsigned startPos);

extern unsigned
stringFindRight(String string, const char c,
		unsigned startPos);

/*
 * Find substring within String 
 *
 * Searches start with character specified by startPos.
 * 
 * matchLen of STRING_MATCH_ALL matches entire pattern.
 */
extern unsigned
stringMatchLeft(String string, const char *s, unsigned matchLen,
		unsigned startPos);
extern unsigned
stringMatchRight(String string, const char *s, unsigned matchLen,
		 unsigned startPos);

extern Boolean      stringHasPrefix(String string, const char *s);
extern Boolean      stringHasSuffix(String string, const char *s);

/*
 * If string is "a/b/c.d":
 *	stringDirectoryPart() returns "a/b/"
 *	stringFilePart() returns "c.d"
 *	stringBasePart() returns "a/b/c"
 *	stringSuffixPart() returns ".d"
 *	stringNextComponent(,"/",,) returns "a", "b", "c.d", and then NULL
 *
 * If string is "/.a":
 *	stringDirectoryPart() returns "/"
 *	stringFilePart() returns ".a"
 *	stringBasePart() returns ""
 *	stringSuffixPart() returns ".a"
 *	stringNextComponent(,"/",,) returns ".a" and then NULL
 *
 * If string is "a":
 *	stringDirectoryPart() returns ""
 *	stringFilePart() returns "a"
 *	stringBasePart() returns "a"
 *	stringSuffixPart() returns ""
 *	stringNextComponent(,"/",,) returns "a" and then NULL
 */
extern char        *stringDirectoryPart(String string);
extern char        *stringFilePart(String string);
extern char        *stringBasePart(String string);
extern char        *stringSuffixPart(String string);

/*
 * Search for next component delimited by any character in delimiters string.
 */
extern char	   *stringNextComponent(String string, const char *delimiters,
			unsigned startPos, unsigned *nextPosp);

/* PRINTFLIKE2 */
extern void	    stringFormat(String string, const char *format, ...);

extern Boolean	    stringGets(String string, FILE *file);

/*
 * Scan and span string.  This functions scan (span) characters in the string
 * looking for any character in the string that is contained in s.  Scan
 * returns the position for the first character in s starting with (and
 * including) startPos. Span returns the position of the first character not
 * in s.
 * 
 * RETURNS: Position of char in (out) of set or STRING_NO_MATCH if end of
 * string.
 */
extern unsigned
stringScan(String string, const char *s,
	unsigned startPos);

extern unsigned
stringSpan(String string, const char *s,
	unsigned startPos);

extern size_t       stringLength(String string);

extern void         stringFree(String string);

_FOREIGN_END

#endif					   /* _COBJS_STRING_H */
