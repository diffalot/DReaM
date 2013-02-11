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
 * $(@)HashTable.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:34 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

#pragma	ident "@(#)HashTable.c 1.3	99/07/29 SMI"

/*
 * Keyed Hash table
 * 
 * FIXME: Make realloc growth smarter -- at small sizes grow more (*2) than at
 * large sizes (*1.25).
 * 
 * FIXME: Consider keep state in separate bit vector from hashItems to reduce
 * memory usage.
 */
#include <stdlib.h>
#include <string.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

#include "cobjs/HashTable.h"

#define	MAX_FILL_FACTOR	0.9		   /* fraction full before resize */
#define	MIN_FILL_FACTOR	0.6		   /* fraction full before resize */
#define	FILL_FACTOR	0.8		   /* fraction full before resize */
#define	INIT_LENGTH	67		   /* initial length */
#define	GROWTH_FACTOR	1.5		   /* growth factor (must be > 1) */

#ifdef	DEBUG
#define	KEEP_STATS
#endif	/* DEBUG */

/*
 * OBJECT HashTable Instance Variables
 */
struct _HashTable {
    HashItem           *hashItems;
#ifdef	KEEP_STATS
    HashStats          *hashStats;
#endif					   /* KEEP_STATS */
    int                 length;
    Boolean             isPrime;
    int                 used;
    int                 deleted;
    int                 maxActive;
    int                 probeCount;
    float               fillFactor;
    int                 reallocates;
    KeyHash             keyHash;
    KeyDup              keyDup;
    KeyIsEqual          keyIsEqual;
    KeyFree             keyFree;
#ifdef	ASSERTS
    Boolean		wasModified;
#endif	/* ASSERTS */
};

/*
 * Private types
 */
typedef enum HashKeyType {
    HASH_COPY_KEY,			   /* must copy key */
    HASH_REF_KEY			   /* ok to just take reference */
} HashKeyType;

typedef enum HashFindType {
    HASH_FIND_FREE,			   /* if not found, return free slot */
    HASH_FIND_ACTIVE			   /* if not found, return NULL */
} HashFindType;

/*
 * Private methods
 */
static void
hashTableInsert(HashTable hashTable, const void *key,
		const void *value, HashKeyType keyType);
static HashItem    *
hashTableFind(HashTable hashTable, const void *key,
	      HashFindType findType);
static int          hashTablePrimeSize(HashTable hashTable, int size);

/*
 * Hash table length is constrained (for best performance) to be prime. Next
 * larger prime than desired size is used.  If no prime in this table is
 * larger, just use size.
 * 
 * Program for generating primes is ifdef'ed out at end of this file.
 */
static const int    primes[] = {
    5, 11, 19, 23, 29, 37, 43, 53, 67, 79, 101, 127, 157, 191, 239, 307,
    367, 461, 577, 719, 907, 1117, 1399, 1741, 2179, 2719, 3407, 4243, 5303,
    6637, 8287, 10357, 12941, 16183, 20219, 25301, 31601, 39499, 49363, 61703,
    77137, 96419, 120503, 150649, 188291, 235369, 294199, 367739, 459677
};

/*
 * Create a string keyed hashtable.
 */
HashTable
hashTableStrNew(void)
{
    return hashTableNew(strHash, strIsEqual, strDup, free);
}

/*
 * Create an integer keyed hashtable.
 */
HashTable
hashTableIntNew(void)
{
    return hashTableNew(intHash, intIsEqual, NULL, NULL);
}

/*
 * Create a hashtable with given key type
 */
HashTable
hashTableNew(KeyHash keyHash, KeyIsEqual keyIsEqual, KeyDup keyDup,
	     KeyFree keyFree)
{
    /*
     * Creates and returns a new hash table with default size and fill
     * factor.
     */
    return hashTableNewWithSizeAndFactor(INIT_LENGTH, FILL_FACTOR,
				      keyHash, keyIsEqual, keyDup, keyFree);
}

HashTable
hashTableIntNewWithSizeAndFactor(int size, float factor)
{
    return hashTableNewWithSizeAndFactor(size, factor,
				      intHash, intIsEqual, NULL, NULL);
}

HashTable
hashTableStrNewWithSizeAndFactor(int size, float factor)
{
    return hashTableNewWithSizeAndFactor(size, factor,
					 strHash, strIsEqual, strDup, free);
}

HashTable
hashTableNewWithSizeAndFactor(int size, float factor, KeyHash keyHash,
			      KeyIsEqual keyIsEqual, KeyDup keyDup,
			      KeyFree keyFree)
{
    /*
     * Creates and returns a new hash table
     */
    HashTable           hashTable;

#if (REHASH_RANGE & (REHASH_RANGE - 1)) != 0
#error REHASH_RANGE must be power of two
#endif	/* __lint */
    if (factor < MIN_FILL_FACTOR) {
	factor = MIN_FILL_FACTOR;
    }
    if (factor > MAX_FILL_FACTOR) {
	factor = MAX_FILL_FACTOR;
    }
    if ((hashTable = NEW_ZEROED(struct _HashTable, 1)) != NULL) {
	hashTable->length = hashTablePrimeSize(hashTable, size);
	hashTable->fillFactor = factor;
	hashTable->maxActive = hashTable->fillFactor * hashTable->length;
	if (hashTable->maxActive >= hashTable->length) {
	    hashTable->maxActive = hashTable->length - 1;
	}
	hashTable->keyHash = keyHash;
	hashTable->keyIsEqual = keyIsEqual;
	hashTable->keyDup = keyDup;
	hashTable->keyFree = keyFree;
#ifdef	ASSERTS
	hashTable->wasModified = FALSE;
#endif	/* ASSERTS */
#ifdef	KEEP_STATS
	hashTable->hashStats = NEW_ZEROED(HashStats, hashTable->length);
#endif					   /* KEEP_STATS */
	if ((hashTable->hashItems = NEW_ZEROED(HashItem, hashTable->length))
		== NULL) {
	    free(hashTable);
	    hashTable = NULL;
	}
    }
    return hashTable;
}


Boolean
_hashTablePut(HashTable hashTable, const void *key, const void *value)
{
    /*
     * Enters key-value pair into hash table. If key already exists, it's
     * value is overwritten.
     */
    hashTable->probeCount = 0;
    if (hashTable->used + hashTable->deleted >= hashTable->maxActive) {
	HashItem           *oldItems = hashTable->hashItems;
	int                 oldMaxActive = hashTable->maxActive;

#ifdef	KEEP_STATS
	HashStats          *oldStats = hashTable->hashStats;

#endif					   /* KEEP_STATS */
	int                 oldLength = hashTable->length;
	HashItem           *item;

	if (hashTable->deleted * 3 <= hashTable->used) {
	    hashTable->length = hashTablePrimeSize(hashTable, 1 +
				 (int) (hashTable->length * GROWTH_FACTOR));
	    hashTable->maxActive = hashTable->fillFactor * hashTable->length;
	    if (hashTable->maxActive >= hashTable->length) {
		hashTable->maxActive = hashTable->length - 1;
	    }
	}
	if ((hashTable->hashItems = NEW_ZEROED(HashItem, hashTable->length))
		== NULL) {
	    hashTable->length = oldLength;
	    hashTable->maxActive = oldMaxActive;
	    hashTable->hashItems = oldItems;
#ifdef	KEEP_STATS
	    hashTable->hashStats = oldStats;
#endif					   /* KEEP_STATS */
	    return FALSE;
	}
#ifdef	KEEP_STATS
	hashTable->hashStats = NEW_ZEROED(HashStats, hashTable->length);
#endif					   /* KEEP_STATS */
	hashTable->used = 0;
	hashTable->deleted = 0;

	for (item = oldItems; item < &oldItems[oldLength]; item++) {
	    if (item->state == HASHITEM_ACTIVE) {
		hashTableInsert(hashTable, item->key,
				item->value, HASH_REF_KEY);
	    }
	}
	free(oldItems);
#ifdef	KEEP_STATS
	free(oldStats);
#endif					   /* KEEP_STATS */
	hashTable->reallocates += 1;
    }
    hashTableInsert(hashTable, key, value, HASH_COPY_KEY);
#ifdef	ASSERTS
    hashTable->wasModified = TRUE;
#endif	/* ASSERTS */
    return TRUE;
}

void               *
_hashTableGet(HashTable hashTable, const void *key)
{
    /*
     * Returns the value associated with key. Returns NULL if the key is not
     * found.
     */
    HashItem           *hashItem;

    hashTable->probeCount = 0;
    hashItem = hashTableFind(hashTable, key, HASH_FIND_ACTIVE);
    return hashItem != NULL ? (void *) hashItem->value : NULL;
}

Boolean
_hashTableIsMember(HashTable hashTable, const void *key)
{
    /*
     * Returns TRUE if key is in hashtable.
     */
    HashItem           *hashItem;

    hashTable->probeCount = 0;
    hashItem = hashTableFind(hashTable, key, HASH_FIND_ACTIVE);
    return Boolean(hashItem != NULL);
}

void               *
_hashTableRemove(HashTable hashTable, const void *key)
{
    /*
     * Remove the key-value pair from the hash table. It is not an error if
     * key does not exist. Returns the value associated with the removed
     * key-value pair. Returns NULL if the key is not found.
     */
    HashItem           *hashItem;
    void               *value = NULL;

    hashTable->probeCount = 0;
    hashItem = hashTableFind(hashTable, key, HASH_FIND_ACTIVE);

    if (hashItem != NULL) {
	value = (void *) hashItem->value;
	if (hashTable->keyFree != NULL) {
	    (*hashTable->keyFree) ((void *) hashItem->key);
	}
	hashItem->state = HASHITEM_DELETED;
	hashTable->used--;
	hashTable->deleted++;
    }
#ifdef	ASSERTS
    hashTable->wasModified = TRUE;
#endif	/* ASSERTS */
    return value;
}

int
hashTableProbeCount(const HashTable hashTable)
{
    return hashTable->probeCount;
}

int
hashTableLength(const HashTable hashTable)
{
    /*
     * Returns the current length of the hash table.
     */
    return hashTable->length;
}

int
hashTableUsed(const HashTable hashTable)
{
    /*
     * Returns the current number of entries used in the hash table.
     */
    return hashTable->used;
}

int
hashTableReallocates(const HashTable hashTable)
{
    /*
     * Returns the number of times the hash table was reallocated.
     */
    return hashTable->reallocates;
}

/* ARGSUSED */
HashStats          *
hashTableStats(const HashTable hashTable)
{
#ifdef	KEEP_STATS
    HashStats          *dupStats;

    dupStats = NEW_ZEROED(HashStats, hashTable->length);
    (void) memcpy(dupStats, hashTable->hashStats,
		  sizeof(HashStats) * hashTable->length);
    return dupStats;
#else					   /* KEEP_STATS */
                        return NULL;
#endif					   /* KEEP_STATS */
}

HashItem           *
hashTableDump(const HashTable hashTable)
{
    /*
     * Returns hash table contents as an array of HashItems End of array is
     * marked by HashItem with key == NULL.
     * 
     * Caller should not modify the data referenced by keys or values returned.
     * 
     * Caller should free() the HashItem array when done.
     */
    HashItem           *itemsDump = NEW(HashItem, hashTable->used + 1);
    HashItem           *item;
    HashItem           *dumpItem;

    for (item = hashTable->hashItems, dumpItem = itemsDump;
	    item < &hashTable->hashItems[hashTable->length];
	    item++) {
	if (item->state == HASHITEM_ACTIVE) {
	    *dumpItem++ = *item;
	}
    }
    dumpItem->key = NULL;
    dumpItem->value = NULL;
    return itemsDump;
}

void
hashTableFree(HashTable hashTable)
{
    /*
     * Frees the hashtable. Does not free the values.
     */
    HashItem           *item;

    if (hashTable->keyFree != NULL) {
	for (item = hashTable->hashItems;
		item < &hashTable->hashItems[hashTable->length];
		item++) {
	    if (item->state == HASHITEM_ACTIVE) {
		(*hashTable->keyFree) ((void *) item->key);
	    }
	}
    }
    free(hashTable->hashItems);
#ifdef	KEEP_STATS
    free(hashTable->hashStats);
#endif					   /* KEEP_STATS */
    free(hashTable);
}

/************************************************************************
 * Useful Hash, Dup, and IsEqual Functions
 ************************************************************************/

unsigned long
strHash(const void *key, unsigned int *incrp)
{
    const unsigned char *kp = (const unsigned char *) key;
    unsigned long       h = 0;
    unsigned            incr = 0;
    unsigned long       g;
    unsigned char       k;

    while ((k = *kp++) != '\0') {
	h = (h << 4) + k;
	incr += k;
	if ((g = h & 0xF0000000) != 0) {
	    h ^= g >> 24;
	}
	h &= ~g;
    }
    *incrp = incr;
    return h;
}

Boolean
strIsEqual(const void *p1, const void *p2)
{
    const char         *s1 = (const char *) p1;
    const char         *s2 = (const char *) p2;
    char                c;

    while ((c = *s1++) == *s2++) {
	if (c == '\0') {
	    return TRUE;
	}
    }
    return FALSE;
}

/* ARGSUSED1 */
const void  *
strDup(const void *p, const void *value)
{
    int                 len = strlen((char *) p) + 1;
    void               *p2 = NEW(char, len);

    return memcpy(p2, p, len);
}

unsigned long
intHash(const void *key, unsigned int *incrp)
{
    *incrp = (unsigned int) key >> 4;
    return ((unsigned long) key * 123821) >> 10;
}

Boolean
intIsEqual(const void *key1, const void *key2)
{
    return Boolean(key1 == key2);
}

/************************************************************************
 * OBJECT HashIter Instance Type
 ************************************************************************/
struct _HashIter {
    HashTable           hashTable;
    int                 index;
};

/************************************************************************
 * OBJECT HashIter Class Interface
 ************************************************************************/

/**
 * Create a HashTable Iterator
 *
 * BEWARE: A hashTable iterator is invalidated by any modification to the
 * referred to hashtable. I.e. removal or addition of an item to the hashtable
 * will cause "undefined" results.
 */
HashIter
hashIterNew(const HashTable hashTable)
{
    HashIter            hi;

    if ((hi = NEW_ZEROED(struct _HashIter, 1)) != NULL) {
	hi->hashTable = hashTable;
	hi->index = -1;
    }
    return hi;
}

/************************************************************************
 * OBJECT HashIter Instance Interface
 ************************************************************************/

/**
 * Initialize iterator to first item in hashtable.
 * Returns TRUE if hashtable is non-empty; FALSE if hashtable is empty.
 */
Boolean
hashIterFirst(HashIter hi)
{
    Boolean             isNonEmpty = Boolean(hashTableUsed(hi->hashTable) != 0);

    hi->index = -1;
    if (isNonEmpty) {
	(void) hashIterNext(hi);
    }
    return isNonEmpty;
}

/**
 * Position iterator at next item in hashtable. Return TRUE if there
 * is a next item; FALSE if wrapping around.
 */
Boolean
hashIterNext(HashIter hi)
{
    HashTable           hashTable = hi->hashTable;


    ASSERT(hi->index == -1 || ! hashTable->wasModified);
    while (++hi->index < hashTable->length) {
	ASSERT(hi->index >= 0 && hi->index < hashTable->length);
	if (hashTable->hashItems[hi->index].state == HASHITEM_ACTIVE) {
#ifdef	ASSERTS
	    hi->hashTable->wasModified = FALSE;
#endif	/* ASSERTS */
	    return TRUE;
	}
    }
    hi->index = -1;
    return FALSE;
}

/**
 * Return a pointer to the item referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
const HashItem           *
hashIterItem(const HashIter hi)
{
    HashTable           hashTable = hi->hashTable;

    ASSERT(! hashTable->wasModified);
    return hashIterValid(hi) ? &hashTable->hashItems[hi->index] : NULL;
}

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
const void           *
hashIterKey(const HashIter hi)
{
    HashTable           hashTable = hi->hashTable;

    ASSERT(! hashTable->wasModified);
    return hashIterValid(hi) ? hashTable->hashItems[hi->index].key : NULL;
}

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
const void           *
hashIterValue(const HashIter hi)
{
    HashTable           hashTable = hi->hashTable;

    ASSERT(! hashTable->wasModified);
    return hashIterValid(hi) ? hashTable->hashItems[hi->index].value : NULL;
}

/**
 * Return TRUE if iterator points to valid item.
 */
Boolean
hashIterValid(const HashIter hi)
{
    HashTable           hashTable = hi->hashTable;

    ASSERT(! hashTable->wasModified);
    return Boolean((hi->index >= 0 && hi->index < hashTable->length
	    && hashTable->hashItems[hi->index].state == HASHITEM_ACTIVE));
}

/**
 * Remove the item referenced by the iterator.
 * Returns TRUE if iterator positioned at valid item.
 * Returns FALSE if iterator not positioned at item.
 */
Boolean
hashIterRemove(HashIter hi)
{
    HashTable           hashTable = hi->hashTable;
    HashItem           *hashItem = (HashItem *)hashIterItem(hi);
    Boolean             isValid = Boolean(hashItem != NULL);

    ASSERT(! hashTable->wasModified);
    if (isValid) {
	if (hashTable->keyFree != NULL) {
	    (*hashTable->keyFree) ((void *) hashItem->key);
	}
	hashItem->state = HASHITEM_DELETED;
	hashTable->used--;
	hashTable->deleted++;
    }
    return isValid;
}

/*
 * Free an iterator
 */
void
hashIterFree(HashIter hi)
{
    free(hi);
}

static void
hashTableInsert(HashTable hashTable, const void *key, const void *value,
		HashKeyType keyType)
{
    /*
     * Insert a new key-value pair into the hash table. keyType indicates
     * whether the key must be copied (user insertion) or may simply be
     * referenced (reallocate -- i.e. it's already been copied).
     */
    HashItem           *newItem = hashTableFind(hashTable, key, HASH_FIND_FREE);

    ASSERT(newItem != NULL);
    if (newItem->state != HASHITEM_ACTIVE) {
	if (newItem->state == HASHITEM_DELETED) {
	    hashTable->deleted--;
	}
	newItem->state = HASHITEM_ACTIVE;
	hashTable->used++;
	newItem->key = (keyType == HASH_REF_KEY || hashTable->keyDup == NULL)
	  ? key : (*hashTable->keyDup)(key, value);
    }
    newItem->value = value;
}

static HashItem    *
hashTableFind(HashTable hashTable, const void *key, HashFindType findType)
{
    /*
     * Look up a key in the hash table. If found, return a pointer to the
     * entry. If not found and findFree, return a pointer to where the entry
     * should go.
     */
    unsigned            incr = 0;	   /* avoid lint complaint */
    unsigned            index = (*hashTable->keyHash) (key, &incr)
    % hashTable->length;
    HashItem           *hip = &hashTable->hashItems[index];
    HashItem           *freeHip = NULL;
    int                 probes = 0;

#ifdef	KEEP_STATS
    int                 oindex = index;

#endif					   /* KEEP_STATS */

    if (hashTable->isPrime && hashTable->length > REHASH_RANGE) {
	incr = incr & (REHASH_RANGE - 1);
	incr = REHASH_RANGE - incr;
    } else {
	incr = 1;
    }

    hashTable->probeCount += 1;
    while (hip->state != HASHITEM_FREE) {
	ASSERT(probes < hashTable->length);
	if (hip->state == HASHITEM_ACTIVE
		&& (*hashTable->keyIsEqual) (key, hip->key)) {
	    return hip;
	} else if (hip->state == HASHITEM_DELETED && freeHip == NULL) {
	    freeHip = hip;
	}
	index = (index + incr) % hashTable->length;
	hip = &hashTable->hashItems[index];
	hashTable->probeCount += 1;
	probes += 1;
    }
    if (freeHip == NULL) {
	freeHip = hip;
    }
#ifdef	KEEP_STATS
    if (findType == HASH_FIND_FREE) {
	hashTable->hashStats[oindex].hits += 1;
	hashTable->hashStats[oindex].rehash[incr - 1] += 1;
    }
#endif					   /* KEEP_STATS */
    return findType == HASH_FIND_FREE ? freeHip : NULL;
}

static int
hashTablePrimeSize(HashTable hashTable, int size)
{
    /*
     * Find the first entry in the prime table that's greater or equal to
     * size.
     */
    int                 i;

    for (i = 0; i < NELEM(primes); i++) {
	if (primes[i] >= size) {
	    hashTable->isPrime = TRUE;
	    return primes[i];
	}
    }
    hashTable->isPrime = FALSE;
    return size;
}
