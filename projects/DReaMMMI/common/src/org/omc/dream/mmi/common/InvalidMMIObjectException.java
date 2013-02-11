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
 * $(@)InvalidMMIObjectException.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

/*
 * invalidMMIObjectException.java
 *
 * Created on April 13, 2006, 4:29 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.omc.dream.mmi.common;

/**
 * 
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm6eb2]
 */
public class InvalidMMIObjectException extends Exception {

/**
 * Creates a new instance of <code>invalidMMIObjectException</code> without detail message.
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm6e12]
 */
    public  InvalidMMIObjectException() {        
        // your code here
    } 

/**
 * Constructs an instance of <code>invalidMMIObjectException</code> with the specified detail message.
 * 
 * @poseidon-object-id [I6782ce2dm10a93ec0689mm6e0b]
 * @param msg the detail message.
 */
    public  InvalidMMIObjectException(String msg) {        
        super(msg);
    } 
 }
