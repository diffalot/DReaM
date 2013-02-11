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
 * $(@)VoucherGenerator.java $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:12 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.dream;

/**
 * Individual DRM vendors need to provide an 
 * implementation of this interface so
 * that their DRM type can be intergrated with the OPERA
 * infrastructure. 
 * 
 * The implementation's generateVoucher(...) method
 * will be used by the OPERA/VOD server to generate 
 * the license release message that will be send to 
 * the DRM type's license server. 
 * 
 * Opera Server has to be configured so that 
 * it knows about the DRM type and its voucher
 * generator implementation. This may be done by 
 * adding an initialization parameter with name 
 * opera.<drmtype>.voucher_generator_class to the 
 * web.xml file of Opera/VOD Server.
 *  
 */
public interface VoucherGenerator {
    
    /**
     * Each DRM type's voucher generator implementation
     * can have a single initialization parameter.
     * Opera Server calls the addConfigParameter(...)
     * method to pass the value of this single initialization 
     * parameter to the voucher generator implementation.
     *  
     * @param configString
     */
    void addConfigParameter(String configString);
    
    /**
     * OPERA infrastructure calls this method to pass
     * the parameter names and values that it 
     * received for a particular request.
     * The information may be used by the DRM type's
     * implementation to create a voucher that will be send
     * to the DRM type's license server.
     * 
     * @param name   parameter name
     * @param value  parameter value
     */
    void addParameter(String name, String value);

    /**
     * The method returns the voucher that is send by 
     * the OPERA infrastructure to the DRM type's license server. 
     *  
     *  @return voucher
     */
    String generateVoucher();
}

