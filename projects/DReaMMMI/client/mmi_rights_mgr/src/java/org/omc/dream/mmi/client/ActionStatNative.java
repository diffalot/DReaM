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
 * $(@)ActionStatNative.java $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:36:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
package org.omc.dream.mmi.client;

/**
 * Lightweight structure for passing information from client to the server and
 * back. This structure does not have any mutexes (TODO) and hence is not thread
 * safe. Take care when using this. This structure is meant to be altered by the
 * RightsManager so that the alterations can be picked up by the client after
 * a call to the RightsManager. 
 */
public class ActionStatNative implements java.io.Serializable {
    public int        num;
    public int        start;
    public int        end;
    public float      scale;
    public String     target;
    public byte []    keys;

    public ActionStatNative() {
        num = 1;
        start = 0;
        end = 0;
        scale = 0;
        target = "DVD";
        keys = null;
    }
}
