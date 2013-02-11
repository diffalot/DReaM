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
 * $(@)SimpleVoucherGenerator.java $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:12 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * The contents of this file are subject to the terms 
 * of the Common Development and Distribution License 
 * (the "License").  You may not use this file except 
 * in compliance with the License.
 * 
 * You can obtain a copy of the license at 
 * OperaServer/CDDLv1.0.txt or 
 * http://www.opensource.org/licenses/cddl1.php. 
 * See the License for the specific language governing 
 * permissions and limitations under the License.
 * 
 * When distributing Covered Code, include this CDDL 
 * HEADER in each file and include the License file at 
 * OperaServer/CDDLv1.0.txt.  If applicable, 
 * add the following below this CDDL HEADER, with the 
 * fields enclosed by brackets "[]" replaced with your 
 * own identifying information: Portions Copyright [yyyy] 
 * [name of copyright owner]
 */

/*
 * @(#)SimpleVoucherGenerator.java  1.1 05/11/09
 *
 * Copyright 2002-2005 Sun Microsystems, Inc. All Rights Reserved.
 */


package com.sun.dream;

public class SimpleVoucherGenerator implements VoucherGenerator {
    
    public void addConfigParameter(String configString) {
        return;
    }

    public void addParameter(String name, String value) {
        if (name != null && !name.trim().equals("") && value != null) {
            if (voucher.equals("")) {
                voucher = name + "=" + value;
            } else {
                voucher = voucher + "&" + name + "=" + value;
            }
        }
    }

    public String generateVoucher() {
        return voucher;
    }

    String voucher = "";
}

