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
 * $(@)DeviceSegment.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;
import org.omc.dream.mmi.common.DeviceId;
import org.omc.dream.mmi.common.LocationId;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm6853]
 */
public class DeviceSegment {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6e2f]
 */
    private LocationId locationId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6e19]
 */
    private DeviceId[] deviceId = null;

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4266]
 */
    public  DeviceSegment() {        
        // your code here
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm4237]
 * @param locationId 
 * @param deviceId 
 */
    public  DeviceSegment(LocationId locationId, DeviceId[] deviceId) {        
        this.locationId = locationId;
        this.deviceId = deviceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm484f]
 * @return 
 */
    public LocationId getLocationId() {        
        return locationId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm481e]
 * @param locationId 
 */
    public void setLocationId(LocationId locationId) {        
        this.locationId = locationId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm47d9]
 * @return 
 */
    public DeviceId[] getDeviceId() {        
        // your code here
        return deviceId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm47b5]
 * @param deviceId 
 */
    public void setDeviceId(DeviceId[] deviceId) {        
        this.deviceId = deviceId;
    } 


/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm4a5f]
 * @return String
 */
    public String print(String delimiter) {        
        StringBuffer sb = new StringBuffer(); 
        if(locationId != null) {
            sb.append(delimiter+"Device"+locationId.print());
        }
        if(deviceId != null) {
            for(int i=0; i<deviceId.length; i++){
                sb.append(delimiter+"Device"+deviceId[i].print());
            }
        }
        return sb.toString();
    } 
 }
