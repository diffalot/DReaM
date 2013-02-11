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
 * $(@)VerbElementId.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6cf2]
 */
public class VerbElementId {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3f38]
 */
    private String strVerbElementId = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3f16]
 */
    public  VerbElementId() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3ee7]
 * @param strVerbElementId 
 */
    public  VerbElementId(String strVerbElementId) {        
        this.strVerbElementId = strVerbElementId;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm445a]
 * @return 
 */
    public String getVerbElementId() {        
        return strVerbElementId;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4440]
 * @return String
 */
    public String print() {        
        return new String(".VerbId="+strVerbElementId);
    } 
 }
