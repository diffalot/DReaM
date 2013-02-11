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
 * $(@)HashTable.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Keyed Hash table
 */
#ifndef	_COBJS_HASHTABLE_H
#define	_COBJS_HASHTABLE_H

#pragma ident "@(#)HashTable.h 1.2	99/03/22 SMI"

#include <stddef.h>

#include "cobjs/Foreign.h"
#include "cobjs/Types.h"

_FOREIGN_START

#define	REHASH_RANGE	(1 << 5)	   /* MUST be power of 2 */

/*
 * OBJECT HashTable Instance Type
 */
typedef struct _HashTable *HashTable;

/*
 * OBJECT HashTable Public Types
 */
typedef enum HashItemState {
    HASHITEM_FREE = 0,
    HASHITEM_ACTIVE = 1,
    HASHITEM_DELETED = 2
} HashItemState;

typedef struct HashItem {
    const void         *key;
    const void         *value;
    HashItemState       state;
} HashItem;

typedef struct HashStats {
    int                 hits;
    int                 rehash[REHASH_RANGE];
} HashStats;

/*
 * These four routines must be implemented to construct a hashtable with an
 * arbitrary key.  (Currently, hashtable's must have a void * value.)
 * 
 * KeyHash must map keys as randomly and uniformly as possible across the space
 * [0..hashTableLen).  It should also return a second value via rehashp that
 * is functionally independent of the hash value and spans the space
 * [0..REHASH_RANGE).
 * 
 * KeyIsEqual returns TRUE if the keys are "equal" (you can decide what equal
 * means), FALSE otherwise.
 * 
 * KeyDup duplicates a key (a malloc if key is a reference, a simple return of
 * the value if key is a value).  KeyDup is also passed the value to stored
 * in the hashTable.  KeyDup may return a reference to a field in value if
 * the value contains the key.  (In this case, keyFree should of course
 * be a nop, since no key was malloc'ed.)
 * 
 * KeyFree frees a duplicated key (a free if key is a reference, a no-op if key
 * is a value).  A NULL may be passed if keyFree is a nop.
 * 
 * Implementations are provided for string and integer keys (although you might
 * still want to do your own implementations using better knowledge of you
 * keys or a faster hash function than used here).
 */
typedef unsigned long (*KeyHash) (const void *key, unsigned int *rehashp);
typedef Boolean     (*KeyIsEqual) (const void *key1, const void *key2);
typedef const void *(*KeyDup) (const void *key, const void *value);
typedef void        (*KeyFree) (void *key);

/**********************************************************************
 * OBJECT HashTable Class Interface
 **********************************************************************/

/*
 * Create a string keyed hashtable.
 */
extern HashTable    hashTableStrNew(void);

/*
 * Create an integer keyed hashtable.
 */
extern HashTable    hashTableIntNew(void);

/*
 * Creates and returns a new hash table with default size and fill factor.
 * 
 * keyHash, keyIsEqual, keyDup, and keyFree are routines which implement the
 * named key functions.
 */
extern HashTable
hashTableNew(KeyHash keyHash, KeyIsEqual keyIsEqual,
	     KeyDup keyDup, KeyFree keyFree);

extern HashTable    hashTableIntNewWithSizeAndFactor(int size, float factor);

extern HashTable    hashTableStrNewWithSizeAndFactor(int size, float factor);

#define	hashTablePtrNewWithSizeAndFactor(size, factor)			\
		    hashTableIntNewWithSizeAndFactor(size, factor)

/*
 * Create a hash table with a given initial size and resize factor. Resize
 * factor should be between 0.5 and 0.9, it will be set to 0.6 if outside
 * this range.
 */
extern HashTable
hashTableNewWithSizeAndFactor(int size, float factor, KeyHash keyHash,
			      KeyIsEqual keyIsEqual, KeyDup keyDup,
			      KeyFree keyFree);

/**********************************************************************
 * OBJECT HashTable Instance Interface
 **********************************************************************/

/*
 * Enters key-value pair into hash table. If key already exists, it's value
 * is overwritten.
 * 
 * Returns TRUE if put was successful, FALSE if out of memory.
 */
extern Boolean
_hashTablePut(HashTable hashTable, const void *key,
	      const void *value);

#define	hashTablePut(hashTable, key, value)		\
	_hashTablePut(hashTable, (void *)(key), (void *)(value))

/*
 * Returns the value associated with key. Returns NULL if the key is not
 * found.
 * 
 * NOTE: probe counting keeps this from being read-only on the hashtable.
 */
extern void        *_hashTableGet(HashTable hashTable, const void *key);

#define	hashTableGet(hashTable, key)			\
	_hashTableGet(hashTable, (void *)(key))

/*
 * Returns true if the key is a member of the hashtable.
 * 
 * NOTE: probe counting keeps this from being read-only on the hashtable.
 */
extern Boolean      _hashTableIsMember(HashTable hashTable, const void *key);

#define	hashTableIsMember(hashTable, key)		\
	_hashTableIsMember(hashTable, (void *)(key))

/*
 * Remove the key-value pair from the hash table. It is not an error if key
 * does not exist. Returns the value associated with the removed key-value.
 * Returns NULL if the key is not found.
 */
extern void        *_hashTableRemove(HashTable hashTable, const void *key);

#define	hashTableRemove(hashTable, key)			\
	_hashTableRemove(hashTable, (void *)(key))

/*
 * Returns the number of probes required by the last Get, Put, or Remove.
 */
extern int          hashTableProbeCount(const HashTable hashTable);

/*
 * Returns the current length of the hash table.
 */
extern int          hashTableLength(const HashTable hashTable);

/*
 * Returns the current number of entries used in the hash table.
 */
extern int          hashTableUsed(const HashTable hashTable);

/*
 * Returns the number of times the hash table was reallocated.
 */
extern int          hashTableReallocates(const HashTable hashTable);

/*
 * Return hash table statistics. Only implemented for debug version.  Returns
 * NULL in non-debug versions.
 */
extern HashStats   *hashTableStats(const HashTable hashTable);

/*
 * Returns hash table contents as an array of HashItems End of array is
 * marked by HashItem with key == NULL.
 * 
 * Caller should not modify the data referenced by keys or values returned.
 * 
 * Caller should free() the HashItem array when done.
 */
extern HashItem    *hashTableDump(const HashTable hashTable);

/*
 * Frees the hashtable. Does not free the values.
 */
extern void         hashTableFree(HashTable hashTable);

/************************************************************************
 * OBJECT HashIter Instance Type
 ************************************************************************/
typedef struct _HashIter *HashIter;

/************************************************************************
 * OBJECT HashIter Class Interface
 ************************************************************************/

/*
 * Create a HashTable Iterator
 * 
 * BEWARE: A hashTable iterator is invalidated by any modification to the
 * referred to hashtable. I.e. removal or addition of an item to the
 * hashtable will cause "undefined" results.
 */
extern HashIter     hashIterNew(const HashTable hashTable);

/************************************************************************
 * OBJECT HashIter Instance Interface
 ************************************************************************/

/*
 * Initialize iterator to first item in hashtable. Returns TRUE if hashtable
 * is non-empty; FALSE if hashtable is empty.
 */
extern Boolean      hashIterFirst(HashIter hi);

/*
 * Position iterator at next item in hashtable. Return TRUE if there is a
 * next item; FALSE if wrapping around.
 */
extern Boolean      hashIterNext(HashIter hi);

/**
 * Return a pointer to the item referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
extern const HashItem    *hashIterItem(const HashIter hi);

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
const void           *hashIterKey(const HashIter hi);

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
const void           *hashIterValue(const HashIter hi);

/*
 * Remove the item referenced by the iterator. Returns TRUE if iterator
 * positioned at valid item. Returns FALSE if iterator not positioned at
 * item.
 */
extern Boolean      hashIterRemove(HashIter hi);

/*
 * Return TRUE if iterator points to valid item
 */
extern Boolean      hashIterValid(const HashIter hi);

/*
 * Free an iterator
 */
extern void         hashIterFree(HashIter hi);

/************************************************************************
 * Useful Hash, Dup, and IsEqual Functions
 ************************************************************************/

extern unsigned long strHash(const void *key, unsigned *incrp);
extern Boolean      strIsEqual(const void *p1, const void *p2);
extern const void  *strDup(const void *p, const void *value);

extern unsigned long intHash(const void *key, unsigned *incrp);
extern Boolean      intIsEqual(const void *key1, const void *key2);
#define	intDup	    NULL

_FOREIGN_END

#endif					   /* _COBJS_HASHTABLE_H */
