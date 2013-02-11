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
 * $(@)LoggerInit.java $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:12 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.io.IOException;
import java.util.logging.LogManager;

import javax.servlet.http.HttpServlet;

/**
 * This servlet is used to initialize the java logger. 
 * The servlet is set to load when this web app starts 
 * in this web apps web.xml file. 
 * 
 *
 */
public class LoggerInit extends HttpServlet {
    
   
    public void init() {
        System.err.println("Loading servlet LoggerInit");
        String    confFile        = "/WEB-INF/conf/logger.properties";
        String    confFileAbsPath = getServletContext().getRealPath(confFile);
        System.err.println("Using logger conf file [" + confFileAbsPath + "]");
        System.setProperty("java.util.logging.config.file" , confFileAbsPath);
        try {
            LogManager.getLogManager().readConfiguration();
        } catch (SecurityException e) {
            System.err.println("Error in reading configuration for the logmanager");
            e.printStackTrace();
        } catch (IOException e) {
            System.err.println("Error in reading configuration for the logmanager");
            e.printStackTrace();
        }
        System.err.println("Done loading servlet LoggerInit...");
    }

}
