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
 * $(@)ActionStat.java $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:36:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
package org.omc.dream.mmi.client;

/**
 * Container for storing rights.
 */
public class ActionStat implements java.io.Serializable {

    public int        num;
    public int        start;
    public int        end;
    public float      scale;
    public String     target;
    public byte []    keys;
  
    private ActionStatNative nativeStat = null;
   
    /*
     * Field stores the actual permission (yes/no) from the MMIResponse. 
     */ 
    private boolean permission = false;
   
    /**
     * Cosntructor will set stats using the values provided in ActionStatNative. 
     */ 
    ActionStat( ActionStatNative nstat ) {
        this.num = nstat.num;
        this.start = nstat.start;
        this.end = nstat.end;
        this.scale = nstat.scale;
        this.target = nstat.target;            
        this.keys = nstat.keys; 
        // May need it later. 
        nativeStat = nstat;
    }
   
    ActionStat( boolean perm ) {
        this.permission = perm;
        num = 1;
        start = 0;
        end = 0;
        scale = 0;
        target = null;
        keys = null;
    }

    /**
     * Given an ActionStat this function will match the fields and return a
     * response denoting if the requested parameters are acceptable. 
     */
    public boolean matchStat( ActionStat stat ) {
        /* this function is required since it is possible that the stat queried
         * in the mmi is broader than the stat query that the driver/native code
         * requested. If however the two stat objects have the same values then
         * return on the basis of if the operation was successful or not. For
         * now this is what we are going to do.
         */
        if( this.num > 0 ) {
            return this.permission;
        } else {
            return false;
        }
    }

    public boolean getPermission() {
        return this.permission;
    }

    /**
     * Given a stat, this function will match the fields and set the values that
     * are allowed. 
     * TODO not supported for now.
     */
    private void setAllowedStat( ActionStat stat ) {
    }

    /**
     * Return the exact same native object that was passed in. Also must delete
     * reference to the native object after this call. Also set the values of
     * the ActionStatNative object to the values of this object.
     */
    public ActionStatNative getNativeObject() {
        ActionStatNative natStat = null;
        
        if ( nativeStat != null ) {
            nativeStat.num = this.num;
            nativeStat.start = this.start;
            nativeStat.end = this.end;
            nativeStat.scale = this.scale;
            nativeStat.target = this.target;
        } 

        natStat = nativeStat;
        nativeStat = null;
        return natStat;
    }

    /**
     * Given a stat this will update the fields to match that of the ActionStat
     * provided. Incremental updates must be signed and verified. Decremental
     * updates need not be signed and can originate from the client as well as
     * the server. TODO
     */
    void update( ActionStat stat ) {
        
    }

    /**
     * string representation 
     */
    public String toString() {
 
        StringBuffer buf = new StringBuffer();
        buf.append( super.toString() + "{ Num = " + num);
        buf.append(", Start = " + start);
        buf.append(", End = " + end);
        buf.append(", scale = " + scale);
        buf.append(", target = " + target);
        buf.append(", nativeStat =  " + nativeStat); 
        buf.append(", permission = " + permission);
        buf.append(", keys = " + keys);
        buf.append(" }\n");

        return new String(buf);
    }
}
