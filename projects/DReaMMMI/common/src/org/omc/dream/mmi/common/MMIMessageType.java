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
 * $(@)MMIMessageType.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:05 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6fd4]
 */
public class MMIMessageType {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6f9f]
 */
    private String strMessageType = null;

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5e88]
 */
    public  MMIMessageType() {        
        // your code here
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5c0f]
 * @param strMesgType 
 */
    public  MMIMessageType(String strMesgType) {        
        setMMIMessageType(strMesgType);
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5e64]
 * @return 
 */
    public String getMMIMessageType() {        
        return strMessageType;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5e49]
 * @param strMesgType 
 */
    public void setMMIMessageType(String strMesgType) {        
        strMessageType = strMesgType;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4988]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        return new String(delimiter+"MMIMessageType="+strMessageType);
    } 
 }
