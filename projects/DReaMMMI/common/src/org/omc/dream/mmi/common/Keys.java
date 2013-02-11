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
 * $(@)Keys.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import sun.misc.*;
import java.io.*;
import java.net.*;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [Im7bf8e319m10ad492ce3amm6501]
 */
public class Keys {
    
    private byte[] byteKeys = null;
    
    public Keys() {
        
    }
    
    public Keys(byte[] byteKeys) {
        setKeys(byteKeys);
    }   
    
    public Keys(String strKeys) {
        setBASE64EncodedKeys(strKeys);
    }
    
    public void setKeys(byte[] byteKeys) {
        this.byteKeys = byteKeys;
    }
    
    public byte[] getKeys() {
        return byteKeys;
    }
    
    public String getBASE64EncodedKeys() {
        
        BASE64Encoder b = new BASE64Encoder();
        String encodedKeys=b.encode(byteKeys);
        try {
            encodedKeys=URLEncoder.encode(encodedKeys, "UTF-8");
        } catch (Exception ioe) {
            ioe.printStackTrace();
        }
        
        return encodedKeys;
    }
    
    public void setBASE64EncodedKeys(String strKeys) {
        BASE64Decoder decoder = new BASE64Decoder();
        try {
        String encodedKeys = URLDecoder.decode(strKeys, "UTF-8");
        byteKeys = decoder.decodeBuffer(encodedKeys);

        } catch(IOException ioe) {
            ioe.printStackTrace();
        }
    }
        
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @return String
 */
    public String print() {
        return new String(".Keys="+getBASE64EncodedKeys());
    }
 }
