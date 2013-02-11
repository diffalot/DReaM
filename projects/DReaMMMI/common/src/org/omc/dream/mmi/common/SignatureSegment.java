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
 * $(@)SignatureSegment.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.SigAlg;
import org.omc.dream.mmi.common.Signature;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm681f]
 */
public class SignatureSegment {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm675a]
 */
    private SigAlg sigAlg = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6744]
 */
    private Signature signature = null;

/**
 * 
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm458a]
 */
    public  SignatureSegment() {        
        
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4564]
 * @param sigAlg 
 * @param signature 
 */
    public  SignatureSegment(SigAlg sigAlg, Signature signature) {        
        this.sigAlg = sigAlg;
        this.signature = signature;
    } 

    public SigAlg getSigAlg() {
        return sigAlg;
    }
    
    public void setSignature(Signature signature) {
        this.signature = signature;
    }
    
    public Signature getSignature() {
        return signature;
    }
    
    public void setSigAlg(SigAlg sigAlg) {
        this.sigAlg = sigAlg;
    }
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm452a]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer();
        sb.append(delimiter+"Signature");
        sb.append(sigAlg.print());
        sb.append(delimiter+"Signature");
        sb.append(signature.print());
        return sb.toString();
    } 
 }
