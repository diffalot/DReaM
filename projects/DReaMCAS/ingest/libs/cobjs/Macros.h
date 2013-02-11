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
 * $(@)Macros.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Assorted "helper" macros
 */

#ifndef	_COBJS_MACROS_H
#define	_COBJS_MACROS_H

#pragma ident "@(#)Macros.h 1.4	99/07/29 SMI"

#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/isa_defs.h>

/*
 * Lint helper
 */
#ifdef	__lint
extern int                 __FALSE__;
#else					   /* __lint */
#define	__FALSE__	FALSE
#endif					   /* __lint */

/*
 * Macro statement helper
 */
#define	BEGIN_STMT		do {
#define	END_STMT		} while (__FALSE__)
#define	STRINGIFY(x)		#x

/*
 * Array helpers
 */
#define	NELEM(array)		(sizeof(array)/sizeof((array)[0]))
#define	OFFSET_OF(type, field)	((size_t) (&(((type *) 0)->field)))

/*
 * C String helpers
 */
#define	STR_EQ(a, b)		(strcmp(a, b) == 0)

#define	STR_ASSIGN(dst, src)			\
	((void)(strncpy((dst), (src) == NULL ? "" : (src), sizeof(dst))), \
	 (dst)[sizeof(dst) - 1] = '\0')

/*
 * Simple math functions
 */
#define	DIV_UP(val, div)	(((val) + (div) - 1) / (div))
#define	DIV_ROUND(val, div)	(((val) + ((div)/2) ) / (div))

#define	ROUND_DOWN(val, mult)	(((val) / (mult)) * (mult))
#define	ROUND_UP(val, mult)	ROUND_DOWN((val) + (mult) - 1, (mult))

#define	MIN(a, b)		((a) < (b) ? (a) : (b))
#define	MAX(a, b)		((a) > (b) ? (a) : (b))

/*
 * Bit-field helpers
 */
#if defined(_BIT_FIELDS_HTOL) && !defined(_BIT_FIELDS_LTOH)
#define	BIT1(f)					\
	u8	f
#define	BIT2(f1, f2)				\
	u8	f1, f2
#define	BIT3(f1, f2, f3)			\
	u8	f1, f2, f3
#define	BIT4(f1, f2, f3, f4)			\
	u8	f1, f2, f3, f4
#define	BIT5(f1, f2, f3, f4, f5)		\
	u8	f1, f2, f3, f4, f5
#define	BIT6(f1, f2, f3, f4, f5, f6)		\
	u8	f1, f2, f3, f4, f5, f6
#define	BIT7(f1, f2, f3, f4, f5, f6, f7)	\
	u8	f1, f2, f3, f4, f5, f6, f7
#define	BIT8(f1, f2, f3, f4, f5, f6, f7, f8)	\
	u8	f1, f2, f3, f4, f5, f6, f7, f8
#elif !defined(_BIT_FIELDS_HTOL) && defined(_BIT_FIELDS_LTOH)
#define	BIT1(f)					\
	u8	f
#define	BIT2(f1, f2)				\
	u8	f2, f1
#define	BIT3(f1, f2, f3)			\
	u8	f3, f2, f1
#define	BIT4(f1, f2, f3, f4)			\
	u8	f4, f3, f2, f1
#define	BIT5(f1, f2, f3, f4, f5)		\
	u8	f5, f4, f3, f2, f1
#define	BIT6(f1, f2, f3, f4, f5, f6)		\
	u8	f6, f5, f4, f3, f2, f1
#define	BIT7(f1, f2, f3, f4, f5, f6, f7)	\
	u8	f7, f6, f5, f4, f3, f2, f1
#define	BIT8(f1, f2, f3, f4, f5, f6, f7, f8)	\
	u8	f8, f7, f6, f5, f4, f3, f2, f1
#else
#error "Either _BIT_FIELDS_HTOL or _BIT_FIELDS_LTOH must be defined"
#endif

/*
 * Macros to support internationalization
 */
#ifdef	DOING_LOCALE
#define	NO_LOCALE(s)		""
#else					   /* DOING_LOCALE */
#define	NO_LOCALE(s)		s
#endif				   	   /* DOING_LOCALE */

/*
 * Macros to simplify use of malloc(), calloc(), realloc() They call
 * mallocHandler() if there's no memory available, mallocHandler() [see
 * below] can attempt to free memory and then return.  If there's still no
 * memory, they abort.
 * 
 * These are not defined if DEBUG is defined, since they get in the way of
 * debugger leak checking.
 */

#define DUP(type, ptr)				\
	((type *)memcpy(NEW(type, 1), ptr, sizeof(type)))

#define MEMDUP(ptr, size)			\
	(memcpy(NEW(u8, size), ptr, size))

#ifdef	DEBUG
#define	NEW(type, num)				\
	((type *)malloc((num)*sizeof(type)))
#define	NEW_ZEROED(type, num)			\
	((type *)calloc(num, sizeof(type)))
#define	RENEW(type, ptr, num)			\
	((type *)realloc(ptr, (num) * sizeof(type)))
#define	MALLOC(num, elsize)			\
	malloc((num)*(elsize))
#define	CALLOC(num, elsize)			\
	calloc(num, elsize)
#define	REALLOC(ptr, num, elsize)		\
	realloc(ptr, (num)*(elsize))
#else					   /* DEBUG */
#define	NEW(type, num)				\
	((type *)ckMalloc(NO_LOCALE(#type), NO_LOCALE(__FILE__),	\
			  __LINE__, num, sizeof(type)))
#define	NEW_ZEROED(type, num)			\
	((type *)ckCalloc(NO_LOCALE(#type), NO_LOCALE(__FILE__),	\
			  __LINE__, num, sizeof(type)))
#define	RENEW(type, ptr, num)			\
	((type *)ckRealloc(NO_LOCALE(#type), NO_LOCALE(__FILE__),	\
			   __LINE__, ptr, num, sizeof(type)))
#define	MALLOC(num, elsize)				\
	ckMalloc(NO_LOCALE("void*"), NO_LOCALE(__FILE__), __LINE__,	\
		num, elsize)
#define	CALLOC(num, elsize)			\
	ckCalloc(NO_LOCALE("void*"), NO_LOCALE(__FILE__), __LINE__,	\
		num, elsize)
#define	REALLOC(ptr, num, elsize)			\
	ckRealloc(NO_LOCALE("void*"), NO_LOCALE(__FILE__), __LINE__,	\
		ptr, num, elsize)
#endif					   /* DEBUG */

extern void        *
ckMalloc(const char *type, const char *file, int line,
	 size_t num, size_t elsize);

extern void        *
ckCalloc(const char *type, const char *file, int line,
	 size_t num, size_t elsize);

extern void        *
ckRealloc(const char *type, const char *file, int line,
	  void *ptr, size_t num, size_t elsize);

/*
 * A library default exists for mallocHander, clients can define their own
 * versions and override this if desired.
 *
 * If implemented, this should attempt to free memory and then return.
 * Or it can print any error message it likes and abort.
 */
extern void
mallocHandler(const char *type, size_t num, size_t elsize,
	      const char *file, int line);

#endif					   /* _COBJS_MACROS_H */
