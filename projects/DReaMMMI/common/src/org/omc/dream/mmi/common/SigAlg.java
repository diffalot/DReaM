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
 * $(@)SigAlg.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import java.net.*;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm67f4]
 */
public class SigAlg {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm67bf]
 */
    private String strURI = null;

/**
 * 
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm46af]
 */
    public  SigAlg() {        
        
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm468a]
 * @param strURI 
 */
    public  SigAlg(String strURI) throws InvalidMMIObjectException {   
        setSigAlg(strURI);
    } 

    public void setSigAlg(String strURI) throws InvalidMMIObjectException {
        try {
            URI uri = new URI(strURI);
        } catch (URISyntaxException urise) {
            urise.printStackTrace();
            throw new InvalidMMIObjectException("SigAlg: invalid URI");
        }
        this.strURI = strURI;        
    }
    
    public String getSigAlg() {
        return strURI;
    }

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm465b]
 * @return String
 */
    public String print() {        
        return new String(".SigAlg="+strURI);
    } 
 }
