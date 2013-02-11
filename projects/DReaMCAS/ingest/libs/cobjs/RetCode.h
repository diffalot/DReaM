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
 * $(@)RetCode.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:35 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * RetCode.h
 * 
 * RetCode allocates the return code space for all objects.
 */

#ifndef	_COBJS_RETCODE_H
#define	_COBJS_RETCODE_H

#pragma ident "@(#)RetCode.h 1.2	99/06/07 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/
#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

_FOREIGN_START

#define	RETCODE_CONS(id, localCode)	RetCode((id) << 16 | (localCode))

/*
 * id 0 is reserved for syscall errors
 * id 1 is reserved for client side errors
 */
#define	BASE_ID			2


typedef enum RetCode {
    RETCODE_SUCCESS = 0,
    RETCODE_FAILED = (BASE_ID << 16) | 0x0001
} RetCode;

typedef u16 RetCodeId;

typedef struct RetCodeTable {
    u32                 localCode;
    const char	       *codeName;
    const char         *msg;
} RetCodeTable;

typedef const char *(*RetCodeFunc) (u16 localCode, char *buf, size_t buflen);

typedef void (*RetCodeDumpFunc)(const char *buf);

extern RetCodeId
retCodeRegisterWithTable(const char *className,
			 RetCodeTable *table);

extern RetCodeId
retCodeRegisterWithFunc(const char *className,
			RetCodeFunc func);

/*
 * Prints retCode names and values in form suitable for constructing enum
 * E.g.
 *	<prefix><codeName> = <globalCode>,
 */
extern void
retCodeDumpNames(const char *prefix, RetCodeDumpFunc func);


/*
 * Prints retCode names and messages in form suitable for constructing table
 * E.g.
 *	{<prefix><codeName>, "<message>"}
 */
extern void
retCodeDumpMessages(const char *prefix, RetCodeDumpFunc func);

/*
 * If buf == NULL or buflen < 80, a thread-specific buffer will be allocated
 * and used.
 */
extern const char  *retCodeMsg(RetCode retCode, char *buf, size_t buflen);

extern RetCode      retCodeFor(char *className, u32 localCode);

extern void	    retCodeFreeze(void);

_FOREIGN_END

#endif					   /* _COBJS_RETCODE_H */
