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
 * $(@)MMIRightsResponseElement.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:06 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.Hint;
import org.omc.dream.mmi.common.Keys;
import org.omc.dream.mmi.common.Notification;
import org.omc.dream.mmi.common.ReqElemId;
import org.omc.dream.mmi.common.RightsErrorStatus;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [Im7b23ede3m10ad34a3c64mm633b]
 */
public class MMIRightsResponseElement {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6094]
 */
    private ReqElemId reqElemId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm606e]
 */
    private Notification notification = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm605b]
 * @poseidon-type org.omc.dream.mmi.common.Hint
 */
    private Hint[] hint = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6042]
 */
    private Keys keys = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6029]
 * @poseidon-type org.omc.dream.mmi.common.RightsErrorStatus
 */
    private RightsErrorStatus[] rightsErrorStatus = null;
    
    public MMIRightsResponseElement() {
        
    }
    
    public void setReqElemId(ReqElemId reqElemId) {
        this.reqElemId = reqElemId;
    }
    
    public ReqElemId getReqElemId() {
        return reqElemId;
    }
    
    public void setNotification(Notification notification) {
        this.notification = notification;
    }
    
    public Notification getNotification() {
        return notification;        
    }
    
    public void setHint(Hint[] hint) {
        this.hint = hint;
    }
    
    public Hint[] getHint() {
        return hint;
    }
    
    public void setKeys(Keys keys) {
        this.keys = keys;
    }
    
    public Keys getKeys() {
        return keys;
    }
    
    public void setRightsErrorStatus(RightsErrorStatus[] rightsErrorStatus) {
        this.rightsErrorStatus = rightsErrorStatus;
    }
    
    public RightsErrorStatus[] getRightsErrorStatus() {
        return rightsErrorStatus;
    }
    
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"Response"+reqElemId.print());
        sb.append(delimiter+"Response."+reqElemId.getReqElemId()+notification.print());
        if(hint!= null) {
            for(int i=0;i<hint.length; i++) {
                sb.append(hint[i].print(delimiter+"Response."+reqElemId.getReqElemId()));
            }  
        }
        if(keys != null) {
            sb.append(delimiter+"Response."+reqElemId.getReqElemId()+keys.print());
        }
        return sb.toString();
    }
 }
