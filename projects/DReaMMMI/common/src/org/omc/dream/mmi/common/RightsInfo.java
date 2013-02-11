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
 * $(@)RightsInfo.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * RightsInfo.java
 *
 * Created on September 27, 2005, 9:47 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 *
 * NOTE:
 * Derived from com.sun.sjc.idtv.vod.shared.data on May 12, 2006
 * A common component between the conductor and the license server to 
 * communicate rights
 *
 * Need to either consolidate and have a common library between conductor and license server
 * or can be implemented independently
 * 
 */

package org.omc.dream.mmi.common;

import java.io.*;
import java.util.*;

public class RightsInfo  implements Serializable {
    
    /** Creates a new instance of RightsInfo */
    public RightsInfo() {
      
       
    }
    public String userID;
    public String movieName;
    public int movieid;
    public int rightsType;
    public int remainingRights;
    public boolean expired;
    public Date rightsStartDate = null;
    public Date rightsEndDate = null;
    public String duration;
    public String verb;
    public String verbArgName;
    public String verbArgValue;
    
    public void setUserID(String userID) {
        this.userID = userID;
    }
    
    public String getUserID() {
        return userID;
    }

        
    public void setMovieName(String movieName) {
        this.movieName = movieName;
    }
    
    public String getMovieName() {
        return movieName;
    }
    
    public void setMovieid(int movieid) {
        this.movieid = movieid;
    }
    
    public int getMovieid() {
        return movieid;
    }
    
    public void setRightsType(int rightsType) {
        this.rightsType = rightsType;
    }
    
    public int getRightsType() {
        return rightsType;
    }
    
}
