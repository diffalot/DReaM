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
 * $(@)MMIMessage.java $Revision: 1.2 $ $Date: 2006/09/14 17:18:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.MMIDataObject;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm7035]
 */
public class MMIMessage implements java.io.Serializable {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6fea]
 */
    private MMIDataObject mmiDataObject = null;
/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm486f]
 */
    public MMIVersion mmiVersion = null;

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm6a22]
 */
    public  MMIMessage() {        
        // your code here
    } 

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm69fd]
 * @param strVersion 
 */
    public void setMMIVersion(String strVersion) {        
        try {
            mmiVersion = new MMIVersion(strVersion);
        } catch (InvalidMMIObjectException immoe) {
            immoe.printStackTrace();
        }
    } 
    
    public void setMMIVersion(MMIVersion mmiVersion) {
        this.mmiVersion = mmiVersion;
    }

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm69d8]
 * @param mmido 
 */
    public void setMMIDataObject(org.omc.dream.mmi.common.MMIDataObject mmido) {        
        mmiDataObject = mmido;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7b23ede3m10ad34a3c64mm64ff]
 * @return 
 */
    public MMIVersion getMMIVersion() {        
        return mmiVersion;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7b23ede3m10ad34a3c64mm64e5]
 * @return 
 */
    public MMIDataObject getMMIDataObject() {        
        return mmiDataObject;
    } 


/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3081]
 * @param delimiter
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer message = new StringBuffer();
        message.append(mmiVersion.print());
        message.append(mmiDataObject.print(delimiter));
        return message.toString();
    } 
        
 }
