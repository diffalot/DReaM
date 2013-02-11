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
 * $(@)Const.java $Revision: 1.2 $ $Date: 2006/07/24 17:00:52 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

/**
 * This class contains the constant string's that are
 * used in the LicenseServer application.
 * 
 */
public final class Const {
    /**
     * The action that the LicenseServer should take with a
     * given request.
     */
    public static final String ACTION = "action";
    public static final String GET_LICENSE = "get_license";
    public static final String VERIFY_SIGNATURE = "verify_signature";
    public static final String CONTENT_ID = "content_id";
    public static final String USERID = "userid";
    public static final String SESSION_ID = "session_id";
    public static final String SIMID = "simid";
    public static final String SHOPID = "shopid";
    public static final String JSP_PATH = "jsp_path";
    
    public static final String LICENSE_SERVER = "opera.license.server";
    public static final String PUBLICKEY_FILE = "public.key";
    public static final String PRIVATEKEY_FILE = "private.key";
    public static final String PROXY_URL = "proxy_url";
    public static final String CAS_FLAG = "cas";
    
}

