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
 * $(@)Duration.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6c40]
 */
public class Duration {

/**
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm319e]
 */
    private String strDuration = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3ccf]
 */
    public  Duration() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3ca0]
 * @param strDuration 
 */
    public  Duration(String strDuration) {        
        this.strDuration = strDuration;
    } 
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [I2332d1f7m10af63edf1amm30b1]
 * @return String
 */
    public String print() {        
        return new String(".Duration="+strDuration);
    } 
 }
