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
 * $(@)RequestHashSegment.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;


/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm67a8]
 */
public class RequestHashSegment {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5f92]
 */
    private HashAlg hashAlg = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5f81]
 */
    private RequestHash requestHash = null;
    
    public RequestHashSegment() {
        
    }
    
    public void setRequestHash(RequestHash requestHash) {
        this.requestHash = requestHash;
    }
    
    public RequestHash getRequestHash() {
        return requestHash;
    }
    
    public void setHashAlg(HashAlg hashAlg) {
        this.hashAlg = hashAlg;
    }
    
    public HashAlg getHashAlg() {
        return hashAlg;
    }
    
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"ReqHash"+hashAlg.print());
        sb.append(delimiter+"ReqHash"+requestHash.print());
        return sb.toString();
    }
    
 }
