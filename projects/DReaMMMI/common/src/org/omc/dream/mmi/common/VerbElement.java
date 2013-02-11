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
 * $(@)VerbElement.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:07 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;


/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm67dc]
 */
public class VerbElement {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b6d]
 */
    private VerbElementId verbElementId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b4f]
 */
    private Verb verb = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b39]
 */
    private Count count = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b1f]
 */
    private Duration duration = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6b09]
 */
    private Period period = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [I2035de77m10aa9053d1fmm6af3]
 */
    private VerbSpecificArgs[] verbSpecificArgs = null;

/**
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm351b]
 */
    public  VerbElement() {        
     
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm34f8]
 * @return 
 */
    public VerbElementId getVerbElementId() {        
        return verbElementId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm34d3]
 * @param verbElementId 
 */
    public void setVerbElementId(VerbElementId verbElementId) {        
        this.verbElementId = verbElementId;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm34ae]
 * @return 
 */
    public Verb getVerb() {        
        return verb;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3489]
 * @param verb 
 */
    public void setVerb(Verb verb) {        
        this.verb = verb;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3464]
 * @return 
 */
    public Count getCount() {        
        return count;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm343f]
 * @param count 
 */
    public void setCount(Count count) {        
        this.count = count;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm341a]
 * @return 
 */
    public Duration getDuration() {        
        return duration;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm33f5]
 */
    public void setDuration(Duration duration) {        
        this.duration = duration;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm33d0]
 * @return 
 */
    public Period getPeriod() {        
        return period;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm33ab]
 * @param period 
 */
    public void setPeriod(Period period) {        
        this.period = period;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3386]
 * @return 
 */
    public VerbSpecificArgs[] getVerbSpecificArgs() {        
        return verbSpecificArgs;
    } 

/**
 * <p>Does ...</p>
 * 
 * 
 * @poseidon-object-id [I2332d1f7m10af63edf1amm3361]
 * @param verbSpecificArgs 
 */
    public void setVerbSpecificArgs(VerbSpecificArgs[] verbSpecificArgs) {        
        this.verbSpecificArgs = verbSpecificArgs;
    } 

/**
 * <p>Generates an MMIMessage phrase of type String</p>
 *
 * @poseidon-object-id [Im1d3b7e4dm10aff99d2b5mm448c]
 * @param strPrefix 
 * @return String
 */
    public String print(String strPrefix) {        
         StringBuffer sb = new StringBuffer();
         sb.append(strPrefix+verbElementId.print());
         String strVerbElementId = verbElementId.getVerbElementId();
         sb.append(strPrefix+"."+strVerbElementId+verb.print());
         if(count != null) {
             sb.append(strPrefix+"."+strVerbElementId+count.print());
         }         
         if(duration != null) {
             sb.append(strPrefix+"."+strVerbElementId+duration.print());
         }         
         if(period != null) {
             sb.append(strPrefix+"."+strVerbElementId+period.print());
         }        
         if(verbSpecificArgs != null) {
             for(int i=0;i<verbSpecificArgs.length;i++) {
                sb.append(verbSpecificArgs[i].print(strPrefix+"."+strVerbElementId));
             }
         }                  
         return sb.toString();
    } 
 }
