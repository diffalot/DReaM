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
 * $(@)MMIRightsRequestElement.java $Revision: 1.2 $ $Date: 2006/09/11 19:15:26 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.ContentId;
import org.omc.dream.mmi.common.ReqElemId;
import org.omc.dream.mmi.common.ServiceId;
import org.omc.dream.mmi.common.VerbElement;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm67f6]
 */
public class MMIRightsRequestElement {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6da2]
 */
    private ReqElemId reqElemId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6baf]
 */
    private ContentId[] contentId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b99]
 */
    private ServiceId[] serviceId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b83]
 */
    private VerbElement[] verbElement = null;

/**
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm388e]
 */
    public  MMIRightsRequestElement() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm37ec]
 * @return 
 */
    public ReqElemId getReqElemId() {        
        return reqElemId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm37c7]
 * @param reqElemId 
 */
    public void setReqElemId(ReqElemId reqElemId) {        
        this.reqElemId = reqElemId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm37a2]
 * @return 
 */
    public ContentId[] getContentId() {        
        return contentId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm377d]
 * @param contentId 
 */
    public void setContentId(ContentId[] contentId) {        
        this.contentId = contentId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3758]
 * @return 
 */
    public ServiceId[] getServiceId() {        
        return serviceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3733]
 * @param serviceId 
 */
    public void setServiceId(org.omc.dream.mmi.common.ServiceId[] serviceId) {        
        this.serviceId = serviceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm370e]
 * @return 
 */
    public VerbElement[] getVerbElement() {        
        return verbElement;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm36e9]
 * @param verbElement 
 */
    public void setVerbElement(VerbElement[] verbElement) {        
        this.verbElement = verbElement;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4810]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"Rights"+reqElemId.print());
        String strReqElemId = reqElemId.getReqElemId();
        // if(contentId == null && serviceId == null)  This case needs to be checked but not here        
        if(contentId != null) {
            StringBuffer concat = new StringBuffer();
            concat.append(contentId[0].getContentId());
            for(int i=1;i<contentId.length; i++) {
                concat.append(","+contentId[i].getContentId());                
            }
            sb.append(delimiter+"Rights."+strReqElemId+".ContentId="+concat.toString());
        }
        if(serviceId != null) {
            StringBuffer concat = new StringBuffer();
            concat.append(serviceId[0].getServiceId());
            for(int i=1;i<serviceId.length; i++) {
                concat.append(","+serviceId[i].getServiceId());
            }
            sb.append(delimiter+"Rights."+strReqElemId+".ServiceId="+concat.toString());
        }
        for(int i=0; i<verbElement.length; i++) {
            sb.append(verbElement[i].print(delimiter+"Rights."+strReqElemId));
        }
        return sb.toString();
    } 
 }
