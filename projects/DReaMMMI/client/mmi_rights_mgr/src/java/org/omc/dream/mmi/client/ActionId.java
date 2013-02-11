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
 * $(@)ActionId.java $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:36:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 

/*
 * @(#)ActionId.java 1.1 05/04/07
 *
 * Copyright 2005 Sun Microsystems, Inc. All Rights Reserved.
 */
package org.omc.dream.mmi.client;

/**
 * This maps the native enumeration of ActionId to the Java enumeration of
 * ActionId. XXX For now ensure that the mapping is kept in sync with the
 * enumeration in the native code (see rights_xface.h). This must be changed so
 * that the native code passes a ActionId enum instead of an integer. XXX
 * TODO
 */
public enum ActionId {

    PLAY        (0) { public String toMMIVerbString() { return "SimplePlay"; } },
    RECORD      (1) { public String toMMIVerbString() { return "Record";} },    
    FFWD        (2) { public String toMMIVerbString() { return "ForwardPlay"; } },
    RWND        (3) { public String toMMIVerbString() { return "ReversePlay"; } },
    CLEAR_COPY  (4) { public String toMMIVerbString() { return "ClearCopy"; } },
    ADAPT       (5) { public String toMMIVerbString() { return "Adapt"; } };

    /**
     * Must be implemented by any new ActionId 
     */
    public abstract String toMMIVerbString(); 

    int num = -1;
    ActionId( int num ) {
        this.num = num;
    }

    public int getNum() {
        return this.num;
    }
 
    /**
     * Given a number, this function will return the corresponding ActionId 
     */ 
    public static ActionId valueOf( int num ) {
        switch( num ) {
        case 0:
            return PLAY;
        case 1:
            return RECORD;
        case 2:
            return FFWD;
        case 3:
            return RWND;
        case 4:
            return CLEAR_COPY;
        case 5:
            return ADAPT;
        default:
            return null;
        }
    }
}
