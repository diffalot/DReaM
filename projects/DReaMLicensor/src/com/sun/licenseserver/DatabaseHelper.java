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
 * $(@)DatabaseHelper.java $Revision: 1.3 $ $Date: 2006/07/24 17:00:52 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.logging.Logger;

import javax.servlet.ServletContext;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


/**
 * The singelton class that encapsulates the JDBC code
 * required to execute SQL statements. To use this class
 * one needs to call the init(...) method that will be used 
 * to initialize the JDBC properties (jdbc driver, url, username and password).
 * The class also checks whether the sunLsLicenses table exists in the database
 * and creates it if it does not exists.
 * 
 * 
 */
public class DatabaseHelper {
    
    /**
     * This is a singleton class and therefore a private constructor.
     */
    private DatabaseHelper() {
    }
    
    /**
     * 
     * This is a singleton class and therefore a private constructor.
     * @param The servlet context used to get a handle to the configuration file.
     */
    private DatabaseHelper(ServletContext sc) {
        m_log.finer("Entering Function...");
        String    confFile = "/WEB-INF/conf/license-server.xml";
        URL       confFileURL = null;
    	try {
	    	confFileURL = sc.getResource(confFile);
	    } catch (MalformedURLException e1) { 
            m_log.severe("License Server configuration file does not exists..");
		    m_initError = true;
		    e1.printStackTrace();
    	}
	    DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder        db  = null;
        Document               doc = null;
        try {
            db = dbf.newDocumentBuilder();
            doc = db.parse(confFileURL.openStream());
	    } catch (SAXException e2) {
           m_log.severe("Error in Parsing License Server Configuration File..");
	       e2.printStackTrace();
           m_initError = true;
	    } catch (IOException e2) {
           m_log.severe("Error in Parsing License Server Configuration File..");
           m_log.severe("Probably the file /WEB-INF/conf/license-server.xml does not exists");
           e2.printStackTrace();
           m_initError = true;
        } catch (ParserConfigurationException e) {
            m_log.severe("Error in Parsing License Server Configuration File..");
            e.printStackTrace();
            m_initError = true;
        }
        initializeProperties(doc);
        loadJDBCDriver();
        createTable();
        m_log.finer("Leaving Function...");
    }
    
    /**
     * 
     * This is a singleton class and therefore a private constructor.
     * @param The servlet context used to get a handle to the configuration file.
     */
    private DatabaseHelper(Document doc) {
        m_log.finer("Entering Function...");
        initializeProperties(doc);
        loadJDBCDriver();
        createTable();
        m_log.finer("Leaving Function...");
    }
    
    /**
     * One of the init functions should be called
     * before the database helper object can be used.
     * This init will be called from jsp pages.
     * 
     * @param sc
     * @return
     */
    public static DatabaseHelper init(ServletContext sc) {
        m_log.finer("Entering Function...");
        if (m_databaseHelper != null) {
           m_log.fine("No need to init. an instance already exists");
           return m_databaseHelper;     
        }
        m_log.fine("Creating a new instance");
        m_databaseHelper = new DatabaseHelper(sc);
        m_log.finer("Leaving Function...");
        return m_databaseHelper;
    }
    
    /**
     * One of the init functions should be called
     * before the database helper object can be used.
     * This init will be called by the LicenseServer servlet 
     * that needs to access some conf values.
     * 
     * @param doc
     * @return
     */
    public static DatabaseHelper init(Document doc) {
        m_log.finer("Entering Function...");
        if (m_databaseHelper != null) {
           m_log.fine("No need to init. an instance already exists");
           return m_databaseHelper;     
        }
        m_log.fine("Creating a new instance");
        m_databaseHelper = new DatabaseHelper(doc);
        m_log.finer("Leaving Function...");
        return m_databaseHelper;
    }
    
    /**
     * This class checks for the existence of the tables required for this
     * project and creates them if they do not exist.
     * 
     *
     */
    public void createTable() {
        if (m_initError) {
            m_log.severe("Databasehelper has not been initialized. Skipping table creation..");
            return;
        }
        
        try {
            if (!hasTable("sunLsLicenses")) {
                String sql = "CREATE TABLE sunLsLicenses ("
                             + "id varchar(20),"
                             + "contentId varchar(40), "
                             + "shopId varchar(40),"
                             + "userId varchar(40),"
                             + "license BLOB, " 
                             + "mime varchar(40)"
                             + ")";
                executeStatementWithoutResults(sql);
            }
        } catch (LicenseServerException e) {
            m_log.severe("Error in creating sunLsLicense table, or checking for its existense");
            m_initError = true;
            e.printStackTrace();
        }
        
    }
    
    /**
     * Open a connection to jdbc URL specified in configuration file.
     */
    public Connection getConnection() 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        Properties  p = new Properties();
        Connection  con = null;
        if ((m_databaseUser != null) && !m_databaseUser.equals("")) {
            p.put("user", m_databaseUser);
        }
        if ((m_databasePassword != null) && !m_databasePassword.equals(" ")) {
            m_log.fine("Using Password");
            p.put("password", m_databasePassword);
        }
        m_log.fine("Using JDBC URL [" + m_databaseURL + "]");
        m_log.fine("Using JDBC Driver [" + m_databaseDriver + "]"); 
        m_log.fine("Connecting to database with user [" + m_databaseUser + "]");
        try {
	        con = DriverManager.getConnection(m_databaseURL, p);
	    } catch (SQLException e) {
	        m_log.severe("Error in establishing a connection to the database [" + e.getMessage() + "]");
            e.printStackTrace();
            if (con != null) {
               try {
				con.close();
			   } catch (SQLException e1) {
				   // TODO Auto-generated catch block
				   e1.printStackTrace();
                   throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                            "Error in getting a connection to DB and closing the partial connection" + e.getMessage());
			   }   
            }
            throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                    "Error in getting a connection to DB" + e.getMessage()); 
	    }
        m_log.finer("Leaving Function...");
        return con;
     }
    
    /**
     * Release the jdbc connection.
     * @param Connection object to be released
     */
    public void freeConnection(Connection con) 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        if (con == null) {
        	return;
        }
        try {
			con.close();
		} catch (SQLException e) {
            m_log.severe("Error in closing a connection [" + e.getMessage() + "]");
		    throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                    "Error in frreing connection" + e.getMessage()); 
		}   
        m_log.finer("Leaving Function...");
    }

    /**
     * load JDBC driver specified in Configuration file.
     */
    private void loadJDBCDriver() {
        m_log.finer("Entering Function...");
        try {
	        Class.forName(m_databaseDriver).newInstance();
         } catch (InstantiationException e) {
             m_initError = true;
             m_log.severe("JDBC Driver can not be instantiated [" + e.getMessage() + "]");
             e.printStackTrace();
	     } catch (IllegalAccessException e) {
             m_initError = true;
             m_log.severe("Illegal access exception thrown while instantiating the JDBC driver [" + e.getMessage() + "]");
             e.printStackTrace();
	     } catch (ClassNotFoundException e) {
             m_initError = true;
             m_log.severe("JDBC Driver is not in the classpath at the runtime [" + e.getMessage() + "]");
	         e.printStackTrace();
	     }
         m_log.finer("Leaving Function...");
    }

	
    /**
     * Get the values of databaseDriver, databaseURL, username and password.
     * @param XML Document data structure containing the configuration file. 
     */
    private void initializeProperties(Document doc) {
        m_log.finer("Entering Function...");
        m_databaseDriver = getConfParameterValue(doc, "drivername");
        m_databaseURL = getConfParameterValue(doc, "databaseurl");
        m_databaseUser = getConfParameterValue(doc, "username");
        m_databasePassword = getConfParameterValue(doc, "password");
        m_listTableSql = getConfParameterValue(doc, "listTableSql");
        String tableColumnNumber = getConfParameterValue(doc, "tableNameColumnNumber");
        m_tableColumnNumber = Integer.parseInt(tableColumnNumber);
        m_log.finer("Leaving Function...");
    }
    
    /**
     * Gets the value of configuration parameters specified in configuration file.
     * @param Configuration file as a XML Document object.
     * @param Configuration parameter
     */
    String getConfParameterValue(Document doc, String confParameter) {
        m_log.finer("Entering Function...");
        NodeList tempNodeList = null;
        String value = null;
        tempNodeList = doc.getElementsByTagName(confParameter);
        if (tempNodeList == null) {
            m_log.severe("Value of parameter [" + confParameter + "] is null");
        	return null;
        }
        if (tempNodeList.item(0) == null) {
            m_log.severe("Value of parameter [" + confParameter + "] is null");
            return null;   
        }
        if (tempNodeList.item(0).getFirstChild() == null) {
            m_log.severe("Value of parameter [" + confParameter + "] is null");
            return null;   
        }
        value = tempNodeList.item(0).getFirstChild().getNodeValue();
        m_log.fine("Value of parameter [" + confParameter + "] is [" + value + "]");
        m_log.finer("Leaving Function...");
        return value;
    }
 
    /**
     * getDatabaseHelper is used to get the sole instance of DatabaseHelper.
     * @param  ServletContext used to obtain a handle to the database configuration file.
     * @return The sole instance of DatabaseHelper class.
     */
    public static DatabaseHelper  getDatabaseHelper()
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        if (m_databaseHelper == null) {
           m_log.severe("getDatabaseHelper is being called without calling init(..) first"); 
           throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE, 
                                            "getDatabaseHelper is being called without calling init(..) first");
        }
        return m_databaseHelper;
    }
    
    /**
     * Execute a sql statement which does not returns value
     * @param sql statement
     * @return number of rows affected by the statement
     */
    public int executeStatementWithoutResults(String s) 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        m_log.fine("Going to execute statement [" + s + "] that produces no results.");
        int          count = 0;
        Connection   con = getConnection();
        Statement    st = null;
		try {
			st    = con.createStatement();
            count = st.executeUpdate(s);
		} catch (SQLException e) {
			m_log.severe("Error [" + e.getMessage() + "] in executing SQL statement [" + s + "]");
			e.printStackTrace();
            throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                    "Error in executing a statement without results" + e.getMessage());
		} finally {
            if (st != null) {
                try {
					st.close();
				} catch (SQLException e1) {
                    m_log.severe("Error [" + e1.getMessage() + "] in releasing a handle to a statement");
					e1.printStackTrace();
                    throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                            "Error in releasing handle to a statement" + e1.getMessage());
				}    
            }
            freeConnection(con);
        }
        m_log.fine("Statement [" + s + "] affected [" + count + "] row");
        m_log.finer("Leaving Function...");
		return count;
    }
    
    /**
     * Execute a sql statement which returns a result. 
     * Freeing the resources associated with the resultset is the responsibilty of the
     * caller of this method.
     * @param sql statement
     * @return resultset object containing the result of the execution of sql statement.
     */
    public ResultSet executeStatementWithResults(String s)
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        m_log.fine("Going to execute statement [" + s + "] that produces results");
        ResultSet    results = null;
        Connection   con = getConnection();
        Statement    st = null;
        try {
            st = con.createStatement();
            st.executeQuery(s);
            results = st.getResultSet();
        } catch (SQLException e) {
            m_log.severe("Error [" + e.getMessage() + "] in executing SQL statement [" + s + "]");
            e.printStackTrace();
            try {
                if (results != null) {
                    results.close();   
                }
                if (st != null) {
                    st.close();
                }
            } catch (SQLException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
                throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                        "Error in releasing resources for a statement with results" + e1.getMessage());
            }    
            freeConnection(con);
            throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                    "Error in executing a statement with results" + e.getMessage());
        }
        m_log.fine("Statement [" + s + "] executed");
        m_log.finer("Leaving Function...");
        return results;
    }
    
    /**
     * Free the resources associated with a result set.
     * @param resultset whose resources are to be freed.
     */
    public void releaseResultSetResources(ResultSet res) 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        if (res == null) {
           return;   
        }
        Statement st = null;
        Connection con = null;
	    try {
            st = res.getStatement();
            res.close();
            if (st != null) {
                con = st.getConnection();   
                st.close();
            }
            freeConnection(con);
	    } catch (SQLException e) {
	       m_log.severe("Error [" + e.getMessage() + "] in releasing result set resources");
	       e.printStackTrace();
           throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                   "Error in releasing resources from a result" + e.getMessage());
	    }
        m_log.finer("Leaving Function...");
    }
    
    /**
     * Checks whether a table with a given name exists in the database
     * @param tableName to check
     * @return true if a table with name tableName exists.
     */
    public  boolean hasTable(String tableName) 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        m_log.fine("Going to find table [" + tableName + "]");
      //  ResultSet   resSet = executeStatementWithResults("show TABLES");
        ResultSet   resSet = executeStatementWithResults(m_listTableSql);
        try {
            while (resSet.next())  {
                String table = resSet.getString(m_tableColumnNumber);
                if (table != null && table.equalsIgnoreCase(tableName)) {
                    m_log.fine("A table with name [" + tableName + "] exists" );
                    return true;            
                }
	        }
	     } catch (SQLException e) {
             m_log.severe("Error in processing table names");
	         e.printStackTrace();
	     } finally {
            releaseResultSetResources(resSet);    
         }
        m_log.fine("A table with name [" + tableName + "] does not exists" );
        m_log.finer("Leaving Function...");
        return false;
    }
    
    /**
     * Returns the count of the number of rows in a result set.
     * Resets the resultset object so that it points before the first row.
     * @param resultset
     * @return number of rows in the resultset.
     */
    public int countNumberInResultSet(ResultSet res) 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        int count = 0;
        try {
            while (res.next()) {
                count++;
            }
            res.beforeFirst();
        } catch (SQLException e) {
            m_log.severe("Error [" + e.getMessage() + "] in counting items in result.");
            e.printStackTrace();
            throw new LicenseServerException(LicenseServerException.EC_DATABASE_ERROR,
                    "Error in counting number of results in result set" + e.getMessage());
        }
        m_log.fine("There are [" + count + "] items");
        m_log.finer("Leaving Function...");
        return count;
    }
    
    private static  DatabaseHelper  m_databaseHelper   = null;
    private         String          m_databaseURL;
    private         String          m_databaseDriver;
    private         String          m_databaseUser;
    private         String          m_databasePassword;
    private         String          m_listTableSql;
    private         int             m_tableColumnNumber;
    private static  Logger          m_log              = Logger.getLogger(DatabaseHelper.class.getName());
    
    
    // It is recommended that a constructor should not throw an excaeption
    // Error in construction will be tracked by this variable.
    private         boolean         m_initError        = false;
}
