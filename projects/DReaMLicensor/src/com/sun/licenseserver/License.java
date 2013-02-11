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
 * $(@)License.java $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:12 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.io.IOException;
import java.io.InputStream;
import java.sql.Blob;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Logger;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.fileupload.DiskFileUpload;
import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.FileUpload;
import org.apache.commons.fileupload.FileUploadException;

public class License {
   
    /**
     * Construct a license object using the input provided in the 
     * request object,
     * 
     * @param req
     */
    public License(HttpServletRequest req) {
        if (FileUpload.isMultipartContent(req)) {
            DiskFileUpload upload = new DiskFileUpload();
            upload.setSizeMax(2*1024*1024);
            List items;
            try {
                items = upload.parseRequest(req);
            } catch (FileUploadException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return;
            }
            Iterator iter = items.iterator();
            while (iter.hasNext()) {
               FileItem item = (FileItem) iter.next();
               if (!item.isFormField()) {
                  if (item.getSize() > 2*1024*1024  ) {
                      continue;
                  }
                  m_log.fine("Size of uploaded license is [" + item.getSize() + "]");
                  try {
                      license    = item.getInputStream();
                      licSize    = item.getSize();
                      mime       = item.getContentType();
                  } catch (IOException e1) {
                      // TODO Auto-generated catch block
                      e1.printStackTrace();
                      return;
                  }
                  
               } else {
                   String name = item.getFieldName();
                   String value = item.getString();
                   m_log.fine("MC ItemName [" + name + "] Value[" + value + "]");
                   if (name != null) {
                       if (name.equals("id")) {
                           id = value;
                           continue;
                       }
                       if (name.equals("userId")) {
                           userId = value;
                           continue;
                       }
                       if (name.equals("contentId")) {
                           contentId = value;
                           continue;
                       }
                       if (name.equals("shopId")) {
                           shopId = value;
                           continue;
                       }
                       
                   }
               }

            }
        }
    }
    
    public License() {
        
    }
    
    /**
     * Constructor 
     * 
     * @param id
     * @param userId
     * @param shopId
     * @param contentId
     * @param license
     */
    License(String id, String userId, String shopId, String contentId, String mime, InputStream license) {
        m_log.finer("Entering Constructor...");
        m_log.fine("Constructing a license object with id=[" + id + "]&shopId=[" + shopId 
                    + "]&contentId=[" + contentId + "]&userId=[" + userId + "]");
        this.id = id;
        this.userId = userId;
        this.shopId = shopId;
        this.contentId = contentId;
        this.license = license;
        this.mime = mime;
        m_log.finer("Leaving Constructor...");
    }
    
    public static License getLicenseFromDatabase(HttpServletRequest req) 
    throws LicenseServerException {
        String userId = req.getParameter("userId");
        String contentId = req.getParameter("contentId");
        String shopId = req.getParameter("shopId");
        verifyLicenseFields(userId, contentId, shopId);
        return License.getLicenseFromDatabase(userId, contentId, shopId);
    }
    
    /**
     * Retrieve a license from the database that belongs to the given 
     * userID, contentID and shopID combination.
     * 
     * @param userID
     * @param contentID
     * @param shopID
     * @return
     * @throws LicenseServerException 
     */
    public static License getLicenseFromDatabase(String userID, String contentID, String shopID) 
    throws LicenseServerException {
        m_log.finer("Entering Function..");
        m_log.fine("Find license for user=[" + userID 
                    + "]contentID=[" + contentID 
                    + "]shopID=[" + shopID + "]");
        ResultSet rs      = null;
        License   lic     =  null;
        Blob      license = null;
        String    id      = "";
        String    mime    = "";
        String    expression = "select license, id, mime from sunLsLicenses where contentId= '" + contentID 
                            + "' and shopId='" + shopID
                            + "' and userId='"  + userID + "'";
        DatabaseHelper dbh = DatabaseHelper.getDatabaseHelper();
        rs = dbh.executeStatementWithResults(expression);
        // The check below is not working. 
        // We need to invoke the beforeFirst() function on the 
        // result set after we have counted the number of entries in the result set.
        // The beforeFirst() method does not works and throws an exception.
        // A work around has to be found to reinstate this test.
        //
//        if (dbh.countNumberInResultSet(rs) > 1) {
//            m_log.severe("More than one license retrieved for a given userID, contentID and shopID combo");
//            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
//                                             "More than one license retrieved for a given userID, contentID and shopID combo");
//        }
            try {
                if (rs != null && rs.next()) {;
                    license = rs.getBlob("license");
                    id = rs.getObject("id").toString();
                    mime = rs.getObject("mime").toString();
                    id = id == null ? "" : id;
                    mime = mime == null ? "" : mime;
                    lic = new License(id, userID, contentID, shopID, mime, license.getBinaryStream());
                }
                dbh.releaseResultSetResources(rs);
            } catch (SQLException e) {
                m_log.severe("Error in retrieveing license from result set");
                e.printStackTrace();
                throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                                                 "Error in retrieveing license from result set");
        }
        m_log.finer("Leaving Function..");
        return lic;
    }
    
    /**
     * Retrieve a license from the database that belongs to the given 
     * userID, contentID and shopID combination.
     * 
     * @param userID
     * @param contentID
     * @param shopID
     * @return
     * @throws LicenseServerException 
     */
    public License getLicenseFromDatabase() 
    throws LicenseServerException {
        verifyLicenseFields();
        return getLicenseFromDatabase(userId, contentId, shopId);
    }
   
    
    public void addToDatabase() 
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        if (id != null && !"".equals(id.trim())) {
            m_log.severe("Trying to add an already existing license to the database");
            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                                             "Trying to add an already existing license to the database");  
        }
        verifyLicenseFields();
        id = String.valueOf(System.currentTimeMillis());
        m_log.fine("Adding to database a license bject with id=[" + id + "]&shopId=[" + shopId 
                + "]&contentId=[" + contentId + "]&userId=[" + userId + "]&license=[" + license + "]");
        String sql = "insert into sunLsLicenses (id, contentId, shopId, userId, mime, license) values"
                     + "("
                     + "'" + id + "', "
                     + "'" + contentId + "', "
                     + "'" + shopId + "', "
                     + "'" + userId + "', "
                     + "'" + mime + "', "
                     + "?"
                     + ")";
        DatabaseHelper dh = DatabaseHelper.getDatabaseHelper();
        PreparedStatement  ps  =  null;
        Connection         con = null;
        con = dh.getConnection();
        try {
            ps = con.prepareStatement(sql);  
            ps.setBinaryStream(1, license, (int) licSize);
            ps.executeUpdate();
        } catch(SQLException e) {
            e.printStackTrace();
            new LicenseServerException(
                                       LicenseServerException.EC_DATABASE_ERROR,
                                       "Error in executing the SQL statement to add a license");
        }
        m_log.finer("Leaving Function...");
        return;
    }
    
    /**
     * Ensures that valid (non null and non empty) values have been 
     * provided for user, content and shop id and license
     * 
     * @param userID
     * @param contentID
     * @param shopID
     * @return
     * @throws LicenseServerException
     */
    private boolean verifyLicenseFields() 
    throws LicenseServerException { 
        m_log.finer("Entering Function...");
        if (userId == null || "".equals(userId.trim())) {
            m_log.info("License can not have a null or empty user id value");
            throw new LicenseServerException(
                                       LicenseServerException.EC_INVALID_ARGUMENT, 
                                      "Missing userID"
                                      );
       }
       if (contentId == null || "".equals(contentId.trim())) {
           m_log.info("License can not have a null or empty content id value");
           throw new LicenseServerException(
                                      LicenseServerException.EC_INVALID_ARGUMENT,
                                      "Missing contentID."
                                      );
      }
       if (shopId == null) {
            m_log.info("License can have a null or empty shop id value. Setting the null shop id to an empty string");
           shopId = "";

       }
       m_log.finer("Leaving Function...");
       return true;
    }
    
    
    /**
     * Ensures that valid (non null and non empty) values have been 
     * provided for user, content and shop id and license
     * 
     * @param userID
     * @param contentID
     * @param shopID
     * @return
     * @throws LicenseServerException
     */
    private static boolean verifyLicenseFields(String userId, String contentId, String shopId) 
    throws LicenseServerException { 
        m_log.finer("Entering Function...");
        if (userId == null || "".equals(userId.trim())) {
            m_log.info("License can not have a null or empty user id value");
            throw new LicenseServerException(
                                       LicenseServerException.EC_INVALID_ARGUMENT, 
                                      "Missing userID"
                                      );
       }
       if (contentId == null || "".equals(contentId.trim())) {
           m_log.info("License can not have a null or empty content id value");
           throw new LicenseServerException(
                                      LicenseServerException.EC_INVALID_ARGUMENT,
                                      "Missing contentID."
                                      );
      }
       if (shopId == null) {
            m_log.info("License can have a null or empty shop id value. Setting the null shop id to an empty string");
           shopId = "";

       }
       m_log.finer("Leaving Function...");
       return true;
    }
    
    /**
     * @return Returns the contentId.
     */
    public String getContentId() {
        return contentId;
    }
    /**
     * @return Returns the id.
     */
    public String getId() {
        return id;
    }
    
    public InputStream getLicense() {
        return license;
    }
    
    /**
     * @return Returns the shopId.
     */
    public String getShopId() {
        return shopId;
    }
    /**
     * @return Returns the userId.
     */
    public String getUserId() {
        return userId;
    }
    
    public String getMime() {
        return mime;
    }

    String          id;
    String          userId;
    String          contentId;
    String          shopId;
    String          mime;
    InputStream     license;
    long            licSize;
    static   Logger m_log = Logger.getLogger(License.class.getName());
    
}
