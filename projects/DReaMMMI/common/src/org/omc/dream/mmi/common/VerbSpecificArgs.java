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
 * $(@)VerbSpecificArgs.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:08 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6c2d]
 */
public class VerbSpecificArgs {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3b56]
 */
    private String strVerbSpecificArgName = null;
    private String strVerbSpecificArgValue = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3dbb]
 */
    public  VerbSpecificArgs() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3d8c]
 * @param strVerbSpecificArgs 
 */
    public  VerbSpecificArgs(String strVerbSpecificArgName, String strVerbSpecificArgValue) {        
        this.strVerbSpecificArgName = strVerbSpecificArgName;
        this.strVerbSpecificArgValue = strVerbSpecificArgValue;
    } 

    public String getVerbSpecificArgName() {
        return strVerbSpecificArgName;
    }
    
    public void setVerbSpecificArgName(String strVerbSpecificArgName) {
        this.strVerbSpecificArgName = strVerbSpecificArgName;
    }

    public String getVerbSpecificArgValue() {
        return strVerbSpecificArgValue;
    }
    
    public void setVerbSpecificArgValue(String strVerbSpecificArgValue) {
        this.strVerbSpecificArgValue = strVerbSpecificArgValue;
    }

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [I2332d1f7m10af63edf1amm2fca]
 * @param strPrefix
 * @return String
 */
    public String print(String strPrefix) {        
        return new String(strPrefix+"."+strVerbSpecificArgName+"="+strVerbSpecificArgValue);
    } 
 }
