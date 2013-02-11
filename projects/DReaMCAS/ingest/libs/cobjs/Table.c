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
 * $(@)Table.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Table.c -- Description of Table
 */

#pragma ident "@(#)Table.c 1.1	98/10/22 SMI"

#if	!defined(TABLE_HEADER)
#define	TABLE_BODY
#define	TABLE_INLINE		extern
#include "cobjs/Table.h"
#endif					   /* !defined(TABLE_HEADER) */

/*
 * Enters key-value pair into table. If key already exists, it's value
 * is overwritten.
 * 
 * Returns TRUE if put was successful, FALSE if out of memory.
 */
TABLE_INLINE Boolean
_tablePut(Table table, const void *key, const void *value)
{
    return (*table->funcs->put)(table->inst, key, value);
}


/*
 * Returns the value associated with key. Returns NULL if the key is not
 * found.
 */
TABLE_INLINE void        *
_tableGet(Table table, const void *key)
{
    return (*table->funcs->get)(table->inst, key);
}

/*
 * Returns true if the key is a member of the table.
 */
TABLE_INLINE Boolean
_tableIsMember(Table table, const void *key)
{
    return (*table->funcs->isMember)(table->inst, key);
}

/*
 * Remove the key-value pair from the table. It is not an error if key
 * does not exist. Returns the value associated with the removed key-value.
 * Returns NULL if the key is not found.
 */
TABLE_INLINE void        *
_tableRemove(Table table, const void *key)
{
    return (*table->funcs->remove)(table->inst, key);
}

/*
 * Returns the current number of entries in the table.
 */
TABLE_INLINE int
tableLength(const Table table)
{
    return (*table->funcs->length)(table->inst);
}

/*
 * Frees the table. Does not free the values.
 */
TABLE_INLINE void
tableFree(Table table)
{
    return (*table->funcs->free)(table->inst);
}

/************************************************************************
 * INTERFACE TableIter Instance Interface
 ************************************************************************/

/*
 * Initialize table iterator to first item in table. Returns TRUE if table
 * is non-empty; FALSE if table is empty.
 */
TABLE_INLINE Boolean
tableIterFirst(TableIter ti)
{
    return (*ti->funcs->first)(ti->inst);
}

/*
 * Position iterator at next item in table. Return TRUE if there is a
 * next item; FALSE if wrapping around.
 */
TABLE_INLINE Boolean
tableIterNext(TableIter ti)
{
    return (*ti->funcs->next)(ti->inst);
}

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
TABLE_INLINE const void    *
tableIterKey(const TableIter ti)
{
    return (*ti->funcs->key)(ti->inst);
}

/**
 * Return a pointer to the value referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
TABLE_INLINE const void    *
tableIterValue(const TableIter ti)
{
    return (*ti->funcs->value)(ti->inst);
}

/*
 * Return TRUE if iterator points to valid item
 */
TABLE_INLINE Boolean
tableIterValid(const TableIter ti)
{
    return (*ti->funcs->valid)(ti->inst);
}

/*
 * Free an iterator
 */
TABLE_INLINE void
tableIterFree(TableIter ti)
{
    return (*ti->funcs->free)(ti->inst);
}
