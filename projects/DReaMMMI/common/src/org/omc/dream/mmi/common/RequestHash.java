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
 * $(@)RequestHash.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [Im7bf8e319m10ad492ce3amm6522]
 */
public class RequestHash {
    private String strRequestHash = null;
    
    public RequestHash() {
        
    }
    
    public RequestHash(String strRequestHash) {
        setRequestHash(strRequestHash);
    }
    
    public void setRequestHash(String strRequestHash) {
        this.strRequestHash = strRequestHash;
    }
    
    public String getRequestHash() {
        return strRequestHash;
    }
    
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @return String
 */
    public String print() {
        return new String(".RequestHash="+strRequestHash);
    }
    
 }
