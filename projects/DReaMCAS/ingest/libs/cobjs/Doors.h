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
 * $(@)Doors.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Doors.h -- Object wrapper for Doors RPC.
 */

#ifndef	_COBJS_DOORS_H
#define	_COBJS_DOORS_H

#pragma ident "@(#)Doors.h 1.4	99/10/27 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <door.h>
#include <stdlib.h>
#include <unistd.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Defines
 ***********************************************************************/

#define	DOORS_RESULT_BUFFER(type, count)  \
	((type *)doorsResultBuffer(sizeof(type), count))

#define	DOORS_DESC_BUFFER(count)   (doorsDescBuffer(count))


/*
 * DOORS_NULL_ID -- an id that no door will ever have.
 * NOTE: This is a bit of a kludge, since the doors system
 * doesn't define an invalid id.  An examination of the code
 * shows that door id are allocated starting with 0.  Since
 * they are 64-bit unsigned values, it is assumed they will
 * never get here.
 */
#define	DOORS_NULL_ID		((door_id_t) -1LL)

/***********************************************************************
 * Instance Types
 ***********************************************************************/

typedef struct _Doors *Doors;

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef void (*DoorsNoClientFunc)(Object delegate);

typedef void (*DoorsThreadInitFunc)(Object delegate);

typedef struct DoorsResultData {
    void		*dataPtr;
    size_t		dataSize;
    door_desc_t		*descPtr;
    size_t		nDesc;
} DoorsResultData;

typedef DoorsResultData (*DoorsServerFunc)(Object delegate, Doors doors,
	const void *argp, size_t argSize,
	const door_desc_t *descp, size_t nDesc,
	const door_cred_t *credp);

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(DOORS_BODY)
#define	DOORS_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(DOORS_BODY) */
#define	DOORS_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(DOORS_BODY) */

/***********************************************************************
 * Class Interface
 ***********************************************************************/

extern void doorsInit(void);
extern void doorsFini(void);

extern Doors
doorsNew(Object delegate, DoorsThreadInitFunc threadInitFunc,
	 DoorsServerFunc serverFunc, DoorsNoClientFunc noClientFunc,
	 size_t stackSize, int maxThreads);

extern Doors
doorsNewWithFile(Object delegate, DoorsThreadInitFunc threadInitFunc,
	 DoorsServerFunc serverFunc, DoorsNoClientFunc noClientFunc,
	 size_t stackSize, int maxThreads, const char *file, uid_t uid,
	 gid_t gid, mode_t mode);

/***********************************************************************
 * Inline-able Instance Interface
 ***********************************************************************/

/*
 * DOORS_INLINE void doorsInline(Doors doors);
 */
DOORS_INLINE door_id_t doorsId(Doors doors);
DOORS_INLINE int doorsFd(Doors doors);
DOORS_INLINE int doorsThreadCount(Doors doors);
DOORS_INLINE DoorsResultData doorsResultData(void *datap,
	size_t dataSize, door_desc_t *descp, size_t nDesc);

/*
 * HashTable and SortTable support
 */
DOORS_INLINE unsigned long doorsIdKeyHash(const void *key,
	unsigned int *rehashp);
DOORS_INLINE Boolean doorsIdKeyIsEqual(const void *key1, const void *key2);
DOORS_INLINE int doorsIdKeyCmp(const void *key1, const void *key2);
DOORS_INLINE const void *doorsIdKeyDup(const void *key, const void *value);
DOORS_INLINE void doorsIdKeyFree(void *key);
	
/***********************************************************************
 * Non-inline-able Instance Interface
 ***********************************************************************/

extern void         *doorsResultBuffer(size_t size, size_t count);
extern door_desc_t  *doorsDescBuffer(size_t count);
/*
 * doorsNoClient() callable from doorsServerFunc() to simulate
 * loss of client.  This may be called when client voluntarily
 * shutdown session.
 */
extern void	     doorsNoClient(Doors doors);
extern void	     doorsSetTrace(Doors doors, Boolean doTrace);
extern void          doorsFree(Doors doors);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(DOORS_BODY)
#define	DOORS_HEADER
#include "cobjs/Doors.c"
#undef	DOORS_HEADER
#endif		   /* defined(DO_INLINING) && !defined(DOORS_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_DOORS_H */
