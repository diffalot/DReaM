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
 * $(@)MMIResponse.java $Revision: 1.2 $ $Date: 2006/09/20 01:18:38 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm71c8]
 */
public class MMIResponse extends MMIDataObject {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6113]
 * @poseidon-type org.omc.dream.mmi.common.Status
 */
    private Status[] status = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm60fa]
 * @poseidon-type org.omc.dream.mmi.common.MMIRightsResponseElement
 */
    private MMIRightsResponseElement[] mmiRightsResponseElement = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm60e1]
 */
    private RequestHashSegment requestHashSegment = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm60c0]
 */
    private ResponseId responseId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm60a7]
 */
    private SignatureSegment signatureSegment = null;

/**
 * 
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm7172]
 */
    public  MMIResponse() {        
        
    } 
    public void setStatus(Status[] status) {
        this.status = status;
    }
    public Status[] getStatus() {
        return status;
    }
    public void setMMIRightsResponseElement(MMIRightsResponseElement[] mmiRightsResponseElement) {
        this.mmiRightsResponseElement = mmiRightsResponseElement;
    }
    public MMIRightsResponseElement[] getMMIRightsResponseElement() {
        return mmiRightsResponseElement;
    }    
    public void setRequestHashSegment(RequestHashSegment requestHashSegment) {
        this.requestHashSegment = requestHashSegment;
    }
    public RequestHashSegment getRequestHashSegment() {
        return requestHashSegment;
    }
    public void setResponseId(ResponseId responseId) {
        this.responseId = responseId;
    }
    public ResponseId getResponseId() {
        return responseId;
    }
    public void setSignatureSegment(SignatureSegment signatureSegment) {
        this.signatureSegment = signatureSegment;
    }
    public SignatureSegment getSignatureSegment() {
        return signatureSegment;
    }
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm48b6]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();   
        for(int i=0;i<status.length;i++) {
            sb.append(status[i].print(delimiter));                    
        }
        for(int i=0;i<mmiRightsResponseElement.length;i++) {
            sb.append(mmiRightsResponseElement[i].print(delimiter));
        }
        sb.append(requestHashSegment.print(delimiter));
        sb.append(responseId.print(delimiter));
        
        ////////  FIX THIS 
        if(signatureSegment != null) { 
            sb.append(signatureSegment.print(delimiter));
        }
        return sb.toString();
    } 
 }
