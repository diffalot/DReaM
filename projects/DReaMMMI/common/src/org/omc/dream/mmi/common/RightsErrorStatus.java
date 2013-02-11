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
 * $(@)RightsErrorStatus.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.RightsCode;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [Im7bf8e319m10ad492ce3amm64e7]
 */
public class RightsErrorStatus {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm6010]
 */
    private RightsCode rightsCode;
    
    public RightsErrorStatus() {
        
    }
    
    public void setRightsErrorStatus(RightsCode rightsCode) {
        this.rightsCode = rightsCode;
    }
    
    public RightsCode getRightsCode() {
        return rightsCode;
    }
 }
