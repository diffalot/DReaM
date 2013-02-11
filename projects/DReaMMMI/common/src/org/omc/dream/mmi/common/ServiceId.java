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
 * $(@)ServiceId.java $Revision: 1.2 $ $Date: 2006/09/11 19:15:26 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6d27]
 */
public class ServiceId {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3fe5]
 */
    private String strServiceId = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3fc3]
 */
    public  ServiceId() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3f94]
 * @param strServiceId 
 */
    public  ServiceId(String strServiceId) {        
        this.strServiceId = strServiceId;
    } 

    
    public void setServiceId(String strServiceId) {
        this.strServiceId = strServiceId;
    }
    
    public String getServiceId() {
        return strServiceId;
    }
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm46e9]
 * @param strPrefix
 * @return String
 */
    public String print(String strPrefix) {        
        return new String(strPrefix+".ServiceId="+strServiceId);
    } 
 }
