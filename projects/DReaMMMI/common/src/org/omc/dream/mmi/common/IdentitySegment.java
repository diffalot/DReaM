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
 * $(@)IdentitySegment.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.AuthServiceId;
import org.omc.dream.mmi.common.AuthTkn;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm6839]
 */
public class IdentitySegment {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6ef2]
 */
    private AuthServiceId authServiceId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6edc]
 */
    private AuthTkn authTkn = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4a0c]
 * @return 
 */
    public AuthServiceId getAuthServiceId() {        
        // your code here
        return authServiceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm49e8]
 * @param authServiceId 
 */
    public void setAuthServiceId(AuthServiceId authServiceId) {        
        this.authServiceId = authServiceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm49a3]
 * @return 
 */
    public AuthTkn getAuthTkn() {        
        return authTkn;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm498b]
 * @param authTkn 
 */
    public void setAuthTkn(AuthTkn authTkn) {        
        this.authTkn = authTkn;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4365]
 */
    public  IdentitySegment()  {            
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm434d]
 * @param authServiceId 
 * @param authTkn 
 */
    public  IdentitySegment(AuthServiceId authServiceId, AuthTkn authTkn) throws InvalidMMIObjectException {        
        if(authServiceId == null) {
            throw new InvalidMMIObjectException("IdentitySegment: improper format");
        }
        this.authServiceId = authServiceId;
        this.authTkn = authTkn;
    } 


/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4a02]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"Identity"+authServiceId.print());
        if(authTkn != null) {
            sb.append("Identity"+authTkn.print());
        }
        return sb.toString();
    } 
 }
