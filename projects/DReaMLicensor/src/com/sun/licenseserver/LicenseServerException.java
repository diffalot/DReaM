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
 * $(@)LicenseServerException.java $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:12 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.util.Vector;

import javax.servlet.ServletException;

/**
 * The exceptions that are thrown by the LicenseServer.
 * 
 */
public class LicenseServerException extends ServletException {

    /**
     * Constructor. 
     * @param code. The error code for this exception.
     */
    public LicenseServerException(int code) {
        super("");
        init(code);
    }
        
    /**
     * Constructor.
     * @param code The error code for this exception.
     * @param msg A message for this exception.
     */
    public LicenseServerException(int code, String msg) {
        super(msg);
        init(code);
    }

    
    /**
     * Populate m_message vector.
     */
    private synchronized static void init_messages()
    {
        // Run this methode only once if application
        // lifetime.
        if( m_init_messages_ran ){
            return;
        }
        
        // One MUST add in the very same numeric order given above.
        // Not compliant order will raise a java.lang.ArrayIndexOutOfBoundsException
        m_messages.add( EC_NO_ERROR_CODE,
        "");
        m_messages.add(EC_INVALID_ARGUMENT,
         "Invalid arguments");
        m_messages.add(EC_DATABASE_ERROR,
        "Error in performing a database operation");
        m_messages.add(EC_PERMISSION_DENIED,
          "Permission denied");
        m_messages.add(EC_SESSION_EXPIRED,
          "Session Expired");
        m_init_messages_ran = true;
    }

    /**
     * Initialise this exception with Code
     * @param code The error code for this exception.
     */
    private void init(int code)
    {
        init_messages();
        m_errorCode = code;
    }

        
    /**
     * @return This exception error code.
     */
    public int errorCode() {
        return m_errorCode;
    }
    
    /**
     * Return this exception detailed message.
     * This message is composed of : 
     * - the error code associated msg
     * - the optionaly passed msg at construction time.
     * 
     * @return Exception message.
     */
    public String getMessage(){
        if( m_errorCode == EC_NO_ERROR_CODE ){
            return super.getMessage();
        }
        return m_messages.get(m_errorCode) + " " + super.getMessage();
    }


    
    /**
     * Holds error messages.
     * Populated by init_messages() methode.
     */
    private static final Vector m_messages = new Vector(32); 
     
    static final int EC_NO_ERROR_CODE             = 0;
    static final int EC_INVALID_ARGUMENT           = 1;
    static final int EC_DATABASE_ERROR            = 2;
    static final int EC_PERMISSION_DENIED         = 3;
    static final int EC_SESSION_EXPIRED           = 4;

    private int m_errorCode = EC_NO_ERROR_CODE;
    /**
     * Conditional init_messages() methode execution.
     */
    private static boolean    m_init_messages_ran = false;
    private static final long serialVersionUID = 1L;
}
