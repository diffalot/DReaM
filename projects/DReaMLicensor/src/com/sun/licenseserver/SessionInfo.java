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
 * $(@)SessionInfo.java $Revision: 1.2 $ $Date: 2006/07/24 17:00:52 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.io.*;

public class SessionInfo {

    private String userId;
    private String contentId;
    private String shopId;
    private String casFlag;
    private boolean authenticated; //true if authenticate by Opera Proxy

    public SessionInfo(String user_id,
                        String content_id,
                        String shop_id,
                        String cas_flag) {
            userId = user_id;
            contentId = content_id;
            shopId = shop_id;
            authenticated = false;
            casFlag = cas_flag;
    }    
    

    public String getUserId() {
        return userId;
    }

    public String getContentId() {
        return contentId;
    }

    public String getShopId() {
        return shopId;
    }

    public boolean isAuthenticated() {
        return authenticated;
    }
    
    public void setAuthenticated(boolean value) {
        authenticated = value;
    }    
    
    public String getCasFlag() {
        return casFlag;
    }
}
