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
 * $(@)Hint.java $Revision: 1.1.1.1 $ $Date: 2006/07/16 21:24:04 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

/**
 * <p></p>
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm676d]
 */
public class Hint {

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5ffd]
 */
    private HintIndexNum hintIndexNum = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5fe8]
 */
    private Label label = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5fd5]
 * @poseidon-type 
 */
    private ContentId[] contentId = null;

/**
 * <p>Represents ...</p>
 * 
 * 
 * @poseidon-object-id [Idafcfb2m10ae17a2a41mm5fbc]
 * @poseidon-type 
 */
    private VerbElement[] verbElement = null;
   
    public Hint() {
        
    }
    
    public void setHintIndexNum(HintIndexNum hintIndexNum) {
        this.hintIndexNum = hintIndexNum;
    }
    
    public HintIndexNum getHintIndexNum() {
        return hintIndexNum;
    }
    
    public void setLabel(Label label) {
        this.label = label;
    }
    
    public Label getLabel() {
        return label;
    }
    
    public void setContentId(ContentId[] contentId) {
        this.contentId = contentId;
    }
    
    public ContentId[] getContentId() {
        return contentId;
    }
    
    public void setVerbElement(VerbElement[] verbElement) {
        this.verbElement = verbElement;
    }
    
    public VerbElement[] getVerbElement() {
        return verbElement;
    }
/**
 * <p>Generates an MMIMessage phrase of type String</p>
 * @param prefix
 * @return String
 */
    public String print(String prefix) {
        StringBuffer sb = new StringBuffer();
        String strHintIndexNum = hintIndexNum.getHintIndexNum();
        sb.append(prefix+".Hint"+hintIndexNum.print());
        sb.append(prefix+".Hint."+strHintIndexNum+label.print());
        StringBuffer strContentId = new StringBuffer();
        strContentId.append(contentId[0].getContentId());
        for(int i=1; i<contentId.length; i++) {
            strContentId.append(","+contentId[1].getContentId());
        }
        sb.append(prefix+".Hint."+strHintIndexNum+".ContentId="+strContentId);
        if(verbElement != null) {
            int intVerbElement = verbElement.length;
            for(int i=0; i<intVerbElement; i++) {
                sb.append(verbElement[i].print(prefix+".Hint."+strHintIndexNum));
            }
        }
        
        return sb.toString();
    }
 }
