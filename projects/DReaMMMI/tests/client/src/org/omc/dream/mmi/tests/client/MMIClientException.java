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
 * $(@)MMIClientException.java $Revision: 1.1 $ $Date: 2006/07/18 00:44:32 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * MMIClientException.java
 *
 * Created on April 18, 2006, 12:16 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.omc.dream.mmi.tests.client;

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm6ae3]
 */
public class MMIClientException extends Exception {

/**
 * Creates a new instance of <code>MMIClientException</code> without detail message.
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm67a2]
 */
    public  MMIClientException() {        
        // your code here
    } 

/**
 * Constructs an instance of <code>MMIClientException</code> with the specified detail message.
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm679b]
 * @param msg the detail message.
 */
    public  MMIClientException(String msg) {        
        super(msg);
    } 
 }
