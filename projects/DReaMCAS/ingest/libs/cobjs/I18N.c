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
 * $(@)I18N.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1995 by Sun Microsystems, Inc.
 */

#pragma ident "@(#)I18N.c 1.2	99/07/29 SMI"

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

static void
i18nInit(void)
{
    /*
     * Makefile.lib defines *_TEXT_DOMAIN
     * Makefile.macros defines PACKAGE
     */
    logLibSetTextDomain(NO_LOCALE(cobjs_TEXT_DOMAIN), NO_LOCALE(TARGETNAME));
}

#pragma init(i18nInit)
