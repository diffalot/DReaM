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
 * $(@)MMIRequest.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:06 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

/*
 * MMIRequest.java
 *
 * Created on April 3, 2006, 9:19 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.omc.dream.mmi.common;
import java.util.*;
import org.omc.dream.mmi.common.IdentitySegment;
import org.omc.dream.mmi.common.RightsSegment;

/**
 * 
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm71c7]
 */
public class MMIRequest extends org.omc.dream.mmi.common.MMIDataObject {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5f6e]
 */
    private MMIMessageType mmiMessageType = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5f5a]
 */
    private IdentitySegment identitySegment = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5ed9]
 */
    private DeviceSegment deviceSegment = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5ec0]
 */
    private RightsSegment rightsSegment = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5ead]
 */
    private SignatureSegment signatureSegment = null;
//    private Map map = null;

/**
 * Creates a new instance of MMIRequest
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm718f]
 */
    public  MMIRequest() {        
        // your code here
    } 

/**
 * 
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm7188]
 * @param map 
 */
    public  MMIRequest(java.util.Map map) throws InvalidMMIObjectException {        
//        this.map = map;
        //validate Map before setting
 /*       boolean invalid=false;
        StringBuffer sb = new StringBuffer();
        Iterator it = map.entrySet().iterator();
        while (it.hasNext()){
            Map.Entry mapE = (Map.Entry)it.next();
            String key = (String)mapE.getKey();
            String[] values = (String[])mapE.getValue(); 
            sb = sb.append(key+"="+values[0]);
            for(int i=1;i<values.length; i++) {
                    sb = sb.append(","+values[i]);
            }
        }
        if(invalid) {
            rights=null;
            device=null;
            identity=null;
            throw new invalidMMIObjectException("Invalid MMIRequest");
        }
  **/
    } 
/*   public MMIRequest(QueryString qs) throws InvalidMMIObjectException {
        
    }
 */

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5d83]
 * @return 
 */
    public MMIMessageType getMMIMessageType() {        
        return mmiMessageType; 
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5bbf]
 * @param strReqType 
 */
    public void setMMIMessageType(String strReqType) throws InvalidMMIObjectException {        
        if(!(strReqType.equals("MMIRightsRequest") || strReqType.equals("MMIRightsRelease"))) {
            throw new InvalidMMIObjectException("MMIRequest: MessageType '"+strReqType+"' not supported");
        }
        mmiMessageType = new MMIMessageType(strReqType);
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5b9a]
 * @return 
 */
    public IdentitySegment getIdentitySegment() {        
        return identitySegment;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5b7f]
 * @param identitySegment 
 */
    public void setIdentitySegment(org.omc.dream.mmi.common.IdentitySegment identitySegment) {        
        this.identitySegment = identitySegment;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5b5b]
 * @return 
 */
    public DeviceSegment getDeviceSegment() {        
        return deviceSegment;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5b40]
 * @param deviceSegment 
 */
    public void setDeviceSegment(org.omc.dream.mmi.common.DeviceSegment deviceSegment) {        
        this.deviceSegment = deviceSegment;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4b00]
 * @return 
 */
    public RightsSegment getRightsSegment() {        
        return rightsSegment;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4adc]
 * @param rightsSegment 
 */
    public void setRightsSegment(RightsSegment rightsSegment) {        
        this.rightsSegment = rightsSegment;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4a86]
 * @return 
 */
    public SignatureSegment getSignatureSegment() {        
        return signatureSegment;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4a51]
 * @param signatureSegment 
 */
    public void setSignatureSegment(SignatureSegment signatureSegment) {        
        this.signatureSegment = signatureSegment;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4906]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();
        sb.append(mmiMessageType.print(delimiter));
        sb.append(identitySegment.print(delimiter));
        if(deviceSegment != null) {
            sb.append(deviceSegment.print(delimiter));           
        }
        sb.append(rightsSegment.print(delimiter));
        if(signatureSegment != null) {
            sb.append(signatureSegment.print(delimiter));
        }
        return sb.toString();
    } 
 }
