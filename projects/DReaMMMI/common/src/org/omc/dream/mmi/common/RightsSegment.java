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
 * $(@)RightsSegment.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.MMIRightsRequestElement;
import org.omc.dream.mmi.common.ProfileId;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm686d]
 */
public class RightsSegment {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6db8]
 */
    private ProfileId profileId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5e70]
 */
    private MMIRightsRequestElement[] mmiRightsRequestElement = null;

/**
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3819]
 */
    public  RightsSegment() {        
        
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4547]
 * @return 
 */
    public ProfileId getProfileId() {        
        return profileId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4522]
 * @param profileId 
 */
    public void setProfileId(ProfileId profileId) {        
        this.profileId = profileId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm44d8]
 * @return 
 */
    public MMIRightsRequestElement[] getMMIRightsRequestElement() {        
        return mmiRightsRequestElement;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm44fd]
 * @param mmiRightsRequestElement 
 */
    public void setMMIRightsRequestElement(org.omc.dream.mmi.common.MMIRightsRequestElement[] mmiRightsRequestElement) {        
        this.mmiRightsRequestElement = mmiRightsRequestElement;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4712]
 * @param delimiter
 * @return String
 */

    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"Rights"+profileId.print());
        for(int i=0;i<mmiRightsRequestElement.length;i++) {
            sb.append(mmiRightsRequestElement[i].print(delimiter));
        }
        return sb.toString();        
    } 
 }
