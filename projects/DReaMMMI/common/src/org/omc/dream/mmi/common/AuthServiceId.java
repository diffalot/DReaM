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
 * $(@)AuthServiceId.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:03 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import java.net.*;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6f81]
 */
public class AuthServiceId {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6f4c]
 */
    private String strAuthServiceId = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm493a]
 */
    public  AuthServiceId() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4922]
 * @param strAuthServiceId 
 */
    public  AuthServiceId(String strAuthServiceId) throws InvalidMMIObjectException {        
        setAuthServiceId(strAuthServiceId);        
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm462d]
 * @return 
 */
    public String getAuthServiceId() {        
        return strAuthServiceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm45f4]
 * @param strAuthServiceId 
 */
    public void setAuthServiceId(String strAuthServiceId) {        
        this.strAuthServiceId=strAuthServiceId;
    } 
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4b52]
 * @return String
 */
    public String print() {        
        return new String(".AuthServiceId="+strAuthServiceId);
    } 
 }
