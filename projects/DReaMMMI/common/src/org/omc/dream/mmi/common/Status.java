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
 * $(@)Status.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.Code;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm67c2]
 */
public class Status {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6083]
 */
    private String[] strStatus = null;
    //private Code code;
    public Status() {
        
    }
    
    public Status(String strStatusSingle) {
        setStatus(strStatusSingle);
    }
    
    public Status(String[] strStatus) {
        setStatus(strStatus);
    }
    
    public String[] getStatus() {
        return strStatus;
    }
    
    public void setStatus(String[] strStatus) {
        this.strStatus = strStatus;
    }
    
    public void setStatus(String strStatusSingle) {
        String[] strTemp = new String[1];
        strTemp[0] = strStatusSingle;
        setStatus(strTemp);        
    }
    
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {
        StringBuffer sb = new StringBuffer();
        sb.append(strStatus[0]);
        for(int i=1;i<strStatus.length; i++) {
            sb.append(","+strStatus[i]);
        }
        return new String(delimiter+"Status="+sb.toString());
    }
 }
