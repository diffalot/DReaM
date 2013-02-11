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
 * $(@)Foreign.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

/*
 * Foreign.h
 * 
 * Do what's necessary to make this C header file work in a foreign language
 * enviroment.
 */

#ifndef	_COBJS_FOREIGN_H
#define	_COBJS_FOREIGN_H

#pragma ident "@(#)Foreign.h 1.1	98/10/22 SMI"

#if defined(__cplusplus)
#define	_FOREIGN_START 		extern              "C" {
#else
#define	_FOREIGN_START
#endif					   /* defined(__cplusplus) */

#if defined(__cplusplus)
#define	_FOREIGN_END		}
#else
#define	_FOREIGN_END
#endif					   /* defined(__cplusplus) */

#endif					   /* _COBJS_FOREIGN_H */
