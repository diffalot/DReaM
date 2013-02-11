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
 * $(@)MMIVersion.java $Revision: 1.2 $ $Date: 2006/09/14 17:18:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm711d]
 */
public class MMIVersion {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm604d]
 */
    private int intMajorVersion = 1;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm6027]
 */
    private int intMinorVersion = 0;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm6000]
 */
    public  MMIVersion() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [Im2f5b5f7cm10ade107da0mm5fdb]
 * @param strVersion 
 */
    public  MMIVersion(String strMMIVersion) throws InvalidMMIObjectException {        
        setMMIVersion(strMMIVersion);
    } 
    
    public MMIVersion(String strMajor, String strMinor) throws InvalidMMIObjectException {
        setMMIVersion(strMajor + "."+strMinor);
    }
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [I2332d1f7m10af63edf1amm542d]
 * @return String
 */
    public String print() {        
        return new String("MMIVersion="+intMajorVersion+"."+intMinorVersion);
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm70ae]
 * @return 
 */
    public String getMMIVersion() {        
        return intMajorVersion + "." + intMinorVersion;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm708a]
 * @param strVersion 
 */
    void setMMIVersion(String strMMIVersion) throws InvalidMMIObjectException {        
        int intPeriodIndex = strMMIVersion.indexOf('.');
        if(intPeriodIndex == -1) {
            throw new InvalidMMIObjectException("MMIVersion: missing '.'");
        } 
        
        String strMajor = strMMIVersion.substring(0, intPeriodIndex);
        if(strMajor == null) {
            throw new InvalidMMIObjectException("MMIVersion: missing major version number");
        }
        String strMinor = strMMIVersion.substring(intPeriodIndex+1);
        if(strMinor == null) {
            throw new InvalidMMIObjectException("MMIVersion: missing minor version number");
        }            
        try {    
            intMajorVersion = Integer.parseInt(strMajor);
            intMinorVersion = Integer.parseInt(strMinor);
        } catch(NumberFormatException nfe) {
            nfe.printStackTrace();
            throw new InvalidMMIObjectException("MMIVersion: version number is not a number");
        }
    } 
 }
