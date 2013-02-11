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
 * $(@)MMIDataObject.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import java.util.*;
import org.omc.dream.mmi.common.MMIMessageType;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [Ia8a97abm10a769a484amm738d]
 */
public abstract class MMIDataObject {

/**
 * 
 * 
 * @poseidon-object-id [Im7b23ede3m10ad34a3c64mm6483]
 */
    
    public  MMIDataObject() {        
        
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm49c4]
 * @return String
 */
    public abstract String print(String delimiter);
    
 }
