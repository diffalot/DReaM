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
 * $(@)Table.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 */

/*
 * Table.h -- Description of Table
 */

#ifndef	_COBJS_TABLE_H
#define	_COBJS_TABLE_H

#pragma ident "@(#)Table.h 1.1	98/10/22 SMI"

/***********************************************************************
 * Global Includes
 ***********************************************************************/

#include <stdlib.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/RetCode.h"
#include "cobjs/Types.h"

_FOREIGN_START

/***********************************************************************
 * Instance Types
 ***********************************************************************/

/*
 * FIXME: Think about how implementer will initialize all of this.
 * TableFuncs sortTableFuncs = {
 *	sortTablePut, sortTableGet, sortTableIsMember,
 *	sortTableRemove, sortTableLength, sortTableFree
 * };
 *
 * sortTableNew(...)
 * {
 *	....
 *	st->tableIsa.inst = st;
 *	st->tableIsa.funcs = &sortTableFuncs;
 *	....
 * }
 *
 * INLINE Table
 * sortTableToTable(SortTable st)
 * {
 *	return &st->tableIsa;
 * }
 */
typedef struct TableFuncs {
    Boolean	(*put)(Object inst, const void *key, const void *value);
    Object	(*get)(Object inst, const void *key);
    Boolean	(*isMember)(Object inst, const void *key);
    Object	(*remove)(Object inst, const void *key);
    int		(*length)(Object inst);
    void	(*free)(Object inst);
} TableFuncs;

typedef struct _Table *Table;
typedef struct _Table TableIsa;

struct _Table {
    Object	    instance;
    TableFuncs	    *funcs;
};

typedef struct TableIterFuncs {
    Boolean	(*first)(Object inst);
    Boolean	(*next)(Object inst);
    void	*(*key)(Object inst);
    void	*(*value)(Object inst);
    Boolean	(*valid)(Object inst);
    void	(*free)(Object inst);
} TableIterFuncs;

struct _TableIter {
    Object	       inst;
    TableIterFuncs    *funcs;
};

typedef struct _TableIter *TableIter;
typedef struct _TableIter TableIterIsa;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TABLE_BODY)
#define	TABLE_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(TABLE_BODY) */
#define	TABLE_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(TABLE_BODY) */

/**********************************************************************
 * Table Instance Interface
 **********************************************************************/

/*
 * Enters key-value pair into table. If key already exists, it's value
 * is overwritten.
 * 
 * Returns TRUE if put was successful, FALSE if out of memory.
 */
TABLE_INLINE Boolean
_tablePut(Table table, const void *key, const void *value);

#define	tablePut(table, key, value)		\
	_tablePut(table, (void *)(key), (void *)(value))

/*
 * Returns the value associated with key. Returns NULL if the key is not
 * found.
 */
TABLE_INLINE void        *_tableGet(Table table, const void *key);

#define	tableGet(table, key) _tableGet(table, (void *)(key))

/*
 * Returns true if the key is a member of the table.
 */
TABLE_INLINE Boolean      _tableIsMember(Table table, const void *key);

#define	tableIsMember(table, key)		\
	_tableIsMember(table, (void *)(key))

/*
 * Remove the key-value pair from the table. It is not an error if key
 * does not exist. Returns the value associated with the removed key-value.
 * Returns NULL if the key is not found.
 */
TABLE_INLINE void        *_tableRemove(Table table, const void *key);

#define	tableRemove(table, key)			\
	_tableRemove(table, (void *)(key))

/*
 * Returns the current number of entries in the table.
 */
TABLE_INLINE int          tableLength(const Table table);

/*
 * Frees the table. Does not free the values.
 */
TABLE_INLINE void         tableFree(Table table);

/************************************************************************
 * INTERFACE TableIter Instance Interface
 ************************************************************************/

/*
 * Initialize table iterator to first item in table. Returns TRUE if table
 * is non-empty; FALSE if table is empty.
 */
TABLE_INLINE Boolean      tableIterFirst(TableIter ti);

/*
 * Position iterator at next item in table. Return TRUE if there is a
 * next item; FALSE if wrapping around.
 */
TABLE_INLINE Boolean      tableIterNext(TableIter ti);

/**
 * Return a pointer to the key referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
TABLE_INLINE const void    *tableIterKey(const TableIter ti);

/**
 * Return a pointer to the value referenced by the iterator.
 * Returns NULL if iterator not positioned at item.
 */
TABLE_INLINE const void    *tableIterValue(const TableIter ti);

/*
 * Return TRUE if iterator points to valid item
 */
TABLE_INLINE Boolean      tableIterValid(const TableIter ti);

/*
 * Free an iterator
 */
TABLE_INLINE void         tableIterFree(TableIter ti);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TABLE_BODY)
#define	TABLE_HEADER
#include "cobjs/Table.c"
#undef	TABLE_HEADER
#endif		   /* defined(DO_INLINING) && !defined(TABLE_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_TABLE_H */
