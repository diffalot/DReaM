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
 * $(@)LicenseServer.java $Revision: 1.3 $ $Date: 2006/09/20 01:37:39 $
 *
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package com.sun.licenseserver;

import java.io.BufferedOutputStream;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.logging.Logger;
import java.io.*;
import java.net.*;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import javax.servlet.*;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.omc.dream.mmi.common.*;
// import com.sun.sjc.idtv.vod.shared.data.*;

import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import com.sun.dream.*;
import java.util.*;

import sun.misc.*;
import com.sun.sjc.idtv.vod.shared.data.RightsInfo;
import com.sun.sjc.idtv.vod.shared.data.VerbElement;

/**
 * The LicenseServer is a servlet that can be used to
 * get the license for a given media type. The license server
 * stores the licenses in a database. The database details have
 * to by the user in the configuration file WEB-INF/conf/LicenseServer.xml.
 *
 */
public class LicenseServer extends HttpServlet {
    
    
    /**
     * Initialize the servlet. The method reads
     * the configuration values stored in the file
     * WEB-INF/conf/License-Server.xml.
     *
     */
    public void init()
    throws ServletException {
        m_log.finest("Entering Function...");
        Document doc = loadConfFile();
        if (doc == null) {
            m_log.severe("Error in initializing the servlet because of problems in parsing conf file");
            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                    "error in parsing the conf file");
            
        }
        m_databaseHelper = DatabaseHelper.init(doc);
        m_operaProxyURL  = m_databaseHelper.getConfParameterValue(doc, "operaProxyURL");
        if (m_operaProxyURL == null || "".equals(m_operaProxyURL.trim())) {
            m_log.severe("Error in initializing the servlet because opera proxy URL is null. " +
                    "Change WEB-INF/conf/license-server.xml");
            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                    "error in parsing the conf file because opera proxy is null");
        }
        String authenticate = m_databaseHelper.getConfParameterValue(doc, "authenticate");
        if (authenticate != null && authenticate.equalsIgnoreCase("false")) {
            m_authenticate = false;
        }
        HashMap map = new HashMap();
        ServletContext servletContext = getServletContext();
        servletContext.setAttribute("SessionHashMap", map);
        
        m_log.finer("Leaving Function...");
    }
    
    /**
     * Loads the xml configuration file,
     * parses it and returns the Document object.
     *
     * @return
     */
    private Document loadConfFile() {
        m_log.finer("Entering Function...");
        String    confFile = "/WEB-INF/conf/license-server.xml";
        URL       confFileURL = null;
        try {
            confFileURL = getServletContext().getResource(confFile);
        } catch (MalformedURLException e1) {
            m_log.severe("License Server configuration file does not exists..");
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
        } catch (IOException e2) {
            m_log.severe("Error in Parsing License Server Configuration File..");
            m_log.severe("Probably the file /WEB-INF/conf/license-server.xml does not exists");
            e2.printStackTrace();
        } catch (ParserConfigurationException e) {
            m_log.severe("Error in Parsing License Server Configuration File..");
            e.printStackTrace();
        }
        m_log.finer("Leaving function...");
        return doc;
    }
    
    /*
     * This method is used to handle HTTP POST requests.
     *
     * @see javax.servlet.http.HttpServlet#doPost(javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)
     */
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        m_log.finest("Entering Function...");
        processRequest(request, response);
        m_log.finer("Leaving Function...");
    }
    
    
    /**
     * This method is used to handle HTTP GET requests.
     *
     * @see javax.servlet.http.HttpServlet#doGet(javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)
     */
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        m_log.finer("Entering Function...");
        processRequest(request, response);
        m_log.finer("Leaving Function...");
    }
    
    
    /**
     * This method processes all the requests that
     * are send to the LicenseServer.
     *
     * @param request
     * @param response
     * @throws IOException
     * @throws ServletException
     */
    protected void processRequest(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException {
        m_log.finer("Entering Function...");
        m_log.fine("LICENSE SERVER REQUEST.GETQUERYSTRING() "+request.getQueryString());
        String action = request.getParameter(Const.ACTION);
        String userID = null;
        String contentID = null;
        String shopID = null;
        String mmiVersion;
        HttpSession httpSession = null;
        MMIMessage mmiMessage = null;
        if (action == null) {
            m_log.fine("ACTION = null");
            mmiVersion = request.getParameter("MMIVersion");
            if(mmiVersion != null) {   //MMI route
                m_log.fine("There is a MMIMessage in this request");
                // This maybe an MMIMessage
                // check MMIMessage
                // get userID, contentID
                String strQuery = request.getQueryString();
                
                MMIParserFactory mmiParserFactory = MMIParserFactory.PLAINTEXT;
                MMIPlainTextParser mmiPlainTextParser = (MMIPlainTextParser)mmiParserFactory.createParser();
                try {
                    m_log.fine("QUERY:"+strQuery);
                    mmiMessage = mmiPlainTextParser.parseMessage(strQuery);
                    
                } catch (Exception e) {
                    e.printStackTrace();
                }
                httpSession = request.getSession();
                httpSession.setAttribute("MMIMessage", mmiMessage);
                
                ServletContext servletContext = getServletContext();
                
                HashMap hashMap = (HashMap)servletContext.getAttribute("SessionHashMap");
                hashMap.put(httpSession.getId(), mmiMessage);
                servletContext.setAttribute("SessionHashMap", hashMap);
                
                MMIRequest mmiRequest = (MMIRequest)mmiMessage.getMMIDataObject();
                userID = mmiRequest.getIdentitySegment().getAuthServiceId().getAuthServiceId();
                MMIRightsRequestElement[] mmiRightsRequestElement = mmiRequest.getRightsSegment().getMMIRightsRequestElement();
                
                StringBuffer sb = new StringBuffer();
                for(int i=0;i<mmiRightsRequestElement.length; i++) {
                    ContentId[] contentId = mmiRightsRequestElement[i].getContentId();
                    for(int j=0; j<contentId.length; j++) {
                        if(i==0 && j==0) {
                            sb.append(contentId[j].getContentId());
                        } else {
                            sb.append(","+contentId[j].getContentId());
                        }
                    }
                }
                contentID=sb.toString();
                shopID="operaShop0";
                
                strQuery = request.getQueryString();
                // check to see if vouchers exist
                // strQuery[1] are the vouchers
                m_log.finer("Calling handleLicenseRequest");
                handleLicenseRequest(userID, contentID, shopID, null, request, response);  // redirect to Opera Proxy
                m_log.finer("Exit handleLicenseRequest");
            } else if ("".equals(action.trim())) {
                throw new LicenseServerException(LicenseServerException.EC_INVALID_ARGUMENT,
                        "Action parameter can not be null.");
            }
        } else {
            m_log.finer("There is no MMIMessage in this request");
            userID = request.getParameter(Const.USERID);
            contentID = request.getParameter(Const.CONTENT_ID);
            shopID = request.getParameter(Const.SHOPID);
            String casFlag = request.getParameter(Const.CAS_FLAG);
            // See if we have a stored session
            // We need to extract defaults from there
            // if userid, contentId and shopid are null
            //
            HttpSession session = request.getSession(true);
            SessionInfo si = (SessionInfo) session.getAttribute("session");
            
            if (si != null) {
                if (userID == null || "".equals(userID.trim())) {
                    userID = si.getUserId();
                }
                if (contentID == null || "".equals(contentID.trim())) {
                    contentID = si.getContentId();
                }
                if (shopID == null || "".equals(shopID.trim())) {
                    shopID = si.getShopId();
                }
                if (casFlag == null || "".equals(casFlag.trim())) {
                    casFlag = si.getCasFlag(); 
                }
            }
            // The below has been remarked since those values are in the MMIMessage itself.
            //verifyLicenseRequestInput(userID, contentID, shopID);
            if (action.equals(Const.GET_LICENSE)) {
                handleLicenseRequest(userID, contentID, shopID, casFlag, request, response);
            }
            if (action.equals(Const.VERIFY_SIGNATURE)) {
                
                // A user's status is never set to authenticated in the session object.
                // Probably when a VERIFY SIGNATURE message is received
                // the status should be set to authenticated.
                if (verifyVoucher(request)) {
                    if(casFlag == null) {
                        sendMMIMessageResponse(userID, contentID, shopID, request, response);                                            
                    } else if(casFlag.equals("1")){
                        sendLicense(userID, contentID, shopID, response);
                        //sendLicense...
                    } else if(casFlag.equals("0")) {
                        // nothing
                    } else {
                        // error
                    }
                } else {
                    throw new LicenseServerException(
                            LicenseServerException.EC_NO_ERROR_CODE,
                            "Data send by Opera has been tampered"
                            );
                }
            }
        }
        m_log.finer("Leaving Function...");
    }
    
    /**
     * This method retrieves the license from the database 
     *
     * @param userId 
     * @param contentId 
     * @throws LicenseServerException
     * @return License
     */
    private  License getLicense(String userId, String contentId) throws LicenseServerException{
        return getLicenseFromDatabase(userId, contentId, "operaShop0");
        
    }
    
    /**
     * This method generates a response for an MMI request
     *
     * @param mmiMessageRequest
     * @param com.sun.sjc.idtv.vod.shared.data.RightsInfo[] 
     * @return MMIMessage
     */
    private MMIMessage generateRightsResponse(MMIMessage mmiMessageRequest, com.sun.sjc.idtv.vod.shared.data.RightsInfo[] ri) {
        m_log.finer("Entering generateRightsResponse");
        // check user
        // check if the user has rights to content
        // check if the rights has not expired
        // check if the user has rights to apply verb on content
        // check if the user has remaining rights to apply verb on content
        // check if the user has rights to apply verb on content with the verb specific arguments
        MMIMessage mmiMessageResponse = new MMIMessage();
        mmiMessageResponse.setMMIVersion("1.0");
        MMIResponse mmiResponse = new MMIResponse();
        if(mmiMessageRequest == null) {
            m_log.severe("ERROR: MMIMESSAGEREQUEST IS EMPTY");
        }
        MMIRequest mmiRequest = (MMIRequest)mmiMessageRequest.getMMIDataObject();
        MMIRightsRequestElement[] mmiRightsRequestElement = mmiRequest.getRightsSegment().getMMIRightsRequestElement();
        Status[] status = new Status[1];
        if(ri == null) {
            status[0] = new Status("RightsElementError");
        } else {
            status[0] = new Status("RequestOK");
        }
        
        mmiResponse.setStatus(status);
        mmiResponse.setResponseId(new ResponseId("1003"));
//        m_log.finer("SignatureSegment"+mmiRequest.getSignatureSegment().print("/n"));
        Signature signature = new Signature();
        SigAlg sigAlg = new SigAlg();
        SignatureSegment signatureSegment = new SignatureSegment(sigAlg,signature);
        mmiResponse.setSignatureSegment(signatureSegment);
        RequestHash requestHash = new RequestHash("jAxX0LfgwutvEdJb748IU4L8obXZPXfqTZ");  //
        HashAlg hashAlg = new HashAlg("http://www.w3.org/2001/10/xml-exc-c14n#");
        RequestHashSegment requestHashSegment = new RequestHashSegment();
        requestHashSegment.setHashAlg(hashAlg);
        requestHashSegment.setRequestHash(requestHash);
        mmiResponse.setRequestHashSegment(requestHashSegment);
        MMIRightsResponseElement[] mmiRightsResponseElement = new MMIRightsResponseElement[mmiRightsRequestElement.length];
        
        boolean found = false;
        boolean denied = false; boolean granted = false;
        boolean error = false;
        int riCountCheck = 0, CountCheck=0;
        for(int i=0; i<mmiRightsRequestElement.length; i++) {
            int grantCount = 0;
            int verbCount = 0;
            String strReqElemId = mmiRightsRequestElement[i].getReqElemId().getReqElemId();
            mmiRightsResponseElement[i] = new MMIRightsResponseElement();
            
            // assign request element id to identify the current request element
            mmiRightsResponseElement[i].setReqElemId(new ReqElemId(strReqElemId));
            boolean userFound = false;
            if(ri != null) {
		// FIX - THERE SEEMS TO BE SOME REUSABLE LOGIC BETWEEN HERE
		// and /DReaM/DReaMBackendSvcs/vod-server-cvs-ejb/src/java/com/sun/sjc/idtv/vod/server/comm/ConductorControllerServlet.java
		//
                for(int j=0; j<ri.length; j++) {
                    // interating through rightsInfo
                    m_log.finer("J="+j);
                    
                    ContentId[] contentId = mmiRightsRequestElement[i].getContentId();
                    if(contentId == null) {
                        
                    } else {
                        for(int k=0;k<contentId.length;k++) {
                            m_log.finer("K="+k);
                            // check contentId against RightsInfo
                            if(ri[j].movieName.equals(contentId[k].getContentId())) {
                                org.omc.dream.mmi.common.VerbElement[] verbElements = mmiRightsRequestElement[i].getVerbElement();
                                int intNumVerbElements = verbElements.length;
                                try {
                                    
                                    for(int l=0;l<intNumVerbElements;l++) {
                                        
                                        m_log.finer("L="+l);
                                        
                                        com.sun.sjc.idtv.vod.shared.data.VerbElement[] riVerbElements = ri[j].verbElements;
                                        if(riVerbElements != null) {
                                            m_log.finer("LENGTH="+riVerbElements.length);
                                            for(int n=0; n<riVerbElements.length; n++) {
                                                m_log.finer("N="+n);
                                                if(verbElements[i].getVerb().getVerb().equalsIgnoreCase(riVerbElements[n].verbName)) {
                                                    m_log.finer("riVerbElements="+riVerbElements[n].verbName);
                                                    verbCount++;
                                                    if(riVerbElements[n].verbArgName.equalsIgnoreCase("Count")) {
                                                        riCountCheck++;
                                                        if(verbElements[l].getCount()!= null) {
                                                            
                                                            if(Integer.parseInt(verbElements[l].getCount().getCount()) <= Integer.parseInt(riVerbElements[n].verbArgValue)) {
                                                                granted = true; n=riVerbElements.length; grantCount++;
                                                                m_log.finer("GRANTED IN LOOP");
                                                            } else {
                                                                denied = true; n=riVerbElements.length;
                                                                m_log.finer("DENIED IN LOOP");
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (riCountCheck == 0 && verbCount > 0) {
                                            granted=true;grantCount++;
                                        }
//                            if(verbCount < riVerbElements.length) {
//                                granted=false;grantCount=0;
//                            }
                                        riCountCheck=0;
                                        verbCount=0;
                                    }
                                } catch(Exception e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    }
                }
            } else {
                error = true;
            }
            if(error) {
                mmiRightsResponseElement[i].setNotification(new Notification("error"));                
                m_log.finer("ERROR");
                error = false;
            } else if(grantCount==mmiRightsRequestElement[i].getVerbElement().length) {
                mmiRightsResponseElement[i].setNotification(new Notification("granted"));
                m_log.finer("GRANTED");
                try {
                    License lic = getLicense(ri[0].userID, mmiRightsRequestElement[i].getContentId()[0].getContentId());
                    if (lic != null) {
                        InputStream is = lic.getLicense();
                        
                        
                        byte[] bytes = new byte[188];  // MAGIC NUMBER FIX THIS
                        try {
                            while(is.read(bytes) != -1) {
                            }
                            BASE64Encoder b = new BASE64Encoder();
                            String encodedKeys=b.encode(bytes);
                            encodedKeys=URLEncoder.encode(encodedKeys, "UTF-8");
                            m_log.finer("END OF LINE EXIST?  :"+encodedKeys.indexOf('\n'));
                            m_log.finer("END OF LINE EXIST?  :"+encodedKeys.indexOf('\r'));
                            
                            mmiRightsResponseElement[i].setKeys(new Keys(encodedKeys));
                        } catch(Exception ioe) {
                            ioe.printStackTrace();
                        }
                    }
                    
                    m_log.finer("Leaving Function...");
                } catch(LicenseServerException lse) {
                    lse.printStackTrace();
                }
            } else {
                mmiRightsResponseElement[i].setNotification(new Notification("denied"));
                m_log.finer("DENIED");
            }
            grantCount=0;
            //Fix for multiple keys
            
        }
        mmiResponse.setMMIRightsResponseElement(mmiRightsResponseElement);
        mmiMessageResponse.setMMIDataObject(mmiResponse);
        m_log.info("MMIResponse: "+mmiMessageResponse.print("\n"));
        return mmiMessageResponse;
        
    }
    
    
    /**
     * If a secure voucherGenerator is implemented, then this method
     * is used to decrypt and verify the voucher.
     *
     * @param req
     * @return true if data send by OPERA server has not been tampered.
     *         false otherwise
     */
    boolean verifyVoucher(HttpServletRequest req) {
        return true;
    }
    
    boolean verifyVoucher(String[] vouchers) {
        return true;
    }
    
    /**
     * Process the request for a license.
     *
     * @param userID
     * @param contentID
     * @param shopID
     * @param request
     * @param response
     * @throws ServletException
     * @throws IOException
     */
    private void handleLicenseRequest(String userID, String contentID, String shopID, String casFlag, HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        m_log.finer("Entering Function...");
        
        
        // See if the session exists for the particular
        // user and if not then create a new session
        // and get it verified by the Proxy Server
        //
        HttpSession session = request.getSession(true);
        SessionInfo si = (SessionInfo) session.getAttribute("session");
        MMIMessage mmiMessage = (MMIMessage) session.getAttribute("MMIMessage");
        if (si == null) {
            si = new SessionInfo(userID, contentID, shopID, casFlag);
            session.setAttribute("session", si);
            session.setMaxInactiveInterval(600); //10 minutes
            m_log.fine("Created a new session");
        }
        String sessionId = session.getId();
        
        if (!si.isAuthenticated() && m_authenticate) {
            // To authenticated the user redirect to Opera Proxy
            //
            m_log.finer("REDIRECTING TO OPERA PROXY");
            m_log.fine("User is not authenticated");
            String operaProxyURL = getOperaProxyURL(userID, contentID, shopID, sessionId);
            m_log.fine("Redirecting to URL [" + operaProxyURL + "]");
            m_log.info("OPERA PROXY URL:" + operaProxyURL);
            if(mmiMessage != null) {
                response.sendRedirect(operaProxyURL+"&"+mmiMessage.print("&"));
            } else {
                response.sendRedirect(operaProxyURL);
            }
        } else {
            m_log.finer("SENDING LICENSE");
            m_log.fine("User is authenticated");
            sendLicense(userID, contentID, shopID, response);
        }
        m_log.finer("Leaving Function...");
    }
    
    
    /**
     * Ensures that valid (non null and non empty) values have been
     * provided for user, content and shop id.
     *
     * @param userID
     * @param contentID
     * @param shopID
     * @return
     * @throws LicenseServerException
     */
    private boolean verifyLicenseRequestInput(String userID, String contentID, String shopID)
    throws LicenseServerException {
        m_log.finer("Entering Function...");
        if (userID == null || "".equals(userID.trim())) {
            m_log.finer("Servlet invoked with a null or empty user id value");
            throw new LicenseServerException(
                    LicenseServerException.EC_INVALID_ARGUMENT,
                    "Missing userID for license request."
                    );
        }
        if (contentID == null || "".equals(contentID.trim())) {
            m_log.finer("Servlet invoked with a null or empty content id value");
            throw new LicenseServerException(
                    LicenseServerException.EC_INVALID_ARGUMENT,
                    "Missing contentID for license request."
                    );
        }
        if (shopID == null || "".equals(shopID.trim())) {
            m_log.finer("Servlet invoked with a null or empty shop id value");
            throw new LicenseServerException(
                    LicenseServerException.EC_INVALID_ARGUMENT,
                    "Missing shopID for license request."
                    );
        }
        m_log.fine("userID=[" + userID + "] contentID=[" + contentID + "] shopID=[" + shopID + "]");
        m_log.finer("Leaving Function...");
        return true;
    }
    
    /**
     * Create the GET HTTP URL that will be used to
     * redirect the client to opera proxy for authentication.
     *
     * @param userID
     * @param contentID
     * @param shopID
     * @param sessionID
     * @throws ServletException
     * @throws IOException
     */
    String getOperaProxyURL(String userID, String contentID, String shopID, String sessionID)
    throws ServletException, IOException {
        m_log.finer("Entering Function...");
        StringBuffer operaProxyURL = new StringBuffer(m_operaProxyURL);
        operaProxyURL.append("?" + Const.ACTION + "=" + Const.GET_LICENSE);
        operaProxyURL.append("&" + Const.USERID + "=" + userID);
        operaProxyURL.append("&" + Const.CONTENT_ID + "=" + contentID);
        operaProxyURL.append("&" + Const.SHOPID + "=" + shopID);
        operaProxyURL.append("&" + Const.SESSION_ID + "=" + sessionID);
        m_log.finer("Leaving Function...");
        return operaProxyURL.toString();
    }
    
    
    /**
     * Handle request to verify signature
     *
     * @param request
     * @param response
     * @throws ServletException
     * @throws IOException
     */
    boolean verifySignature(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        
        HttpSession session = request.getSession(true);
        SessionInfo si = (SessionInfo) session.getAttribute("session");
        if (si == null) {
            //Throw exception here
            //Tried to send license to
            //non-existent session
            return false;
        }
        
        //Do all decrypting of LRM here
        //and if all OK then send license
        //In the simple version of VoucherGenerator,
        //there is no decryption of LRM.
        //If a secure VoucherGenerator is implemented,
        //then the appropriate verfication code goes here
        return true;
    }
    
    
    /**
     * Send license through HTTP response
     *
     * @param userID
     * @param contentID
     * @param shopID
     * @param response
     * @throws ServletException
     * @throws IOException
     */
    void sendLicense(String userID, String contentID, String shopID, HttpServletResponse response)
    throws ServletException, IOException {
        m_log.finer("Entering Function...");
        License lic = getLicenseFromDatabase(userID, contentID, shopID);
        m_log.fine("Sending back the retrieved license");
        if (lic == null) {
            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                    "License not available in the database");
        }
        InputStream licStream = lic.getLicense();
        if (licStream == null) {
            throw new LicenseServerException(LicenseServerException.EC_NO_ERROR_CODE,
                    "license is not valid");
        }
        response.setContentType(lic.getMime());
        BufferedOutputStream outS = new BufferedOutputStream(response.getOutputStream());
        byte by[] = new byte[32768];
        int index = licStream.read(by, 0, 32768);
        while (index != -1) {
            outS.write(by, 0, index);
            index = licStream.read(by, 0, 32768);
        }
        outS.flush();
        m_log.finer("Leaving Function...");
    }
    
    /**
     * Extracts the encoded rights from the vouchers which are embedded in the HTTP request.
     *
     * @param request
     * @param response
     * @throws ServletException
     * @throws IOException
     */
    private com.sun.sjc.idtv.vod.shared.data.RightsInfo[] extractRightsInfoFromVouchers(HttpServletRequest request, HttpServletResponse response) throws IOException {
        
        String[] temp = request.getQueryString().split("&voucher=");
        if(temp.length == 1) {
            // no vouchers found
            return null;
        }
        String voucher = temp[temp.length-1];
        m_log.finer("voucher: "+voucher);
        
        BASE64Decoder decoder = new BASE64Decoder();
        voucher = URLDecoder.decode(voucher, "UTF-8");
        byte[] bytes = decoder.decodeBuffer(voucher);
        ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
        ObjectInputStream is = new ObjectInputStream(bais);
        com.sun.sjc.idtv.vod.shared.data.RightsInfo[] rightsInfo =null;
        try {
            rightsInfo = (com.sun.sjc.idtv.vod.shared.data.RightsInfo[])is.readObject();
            is.close();
        } catch(ClassNotFoundException cnfe) {
            cnfe.printStackTrace();
        }
        
        m_log.finer("RIGHTSINFO: "+rightsInfo[0].userID+" "+rightsInfo[0].remainingRights);
        return rightsInfo;
    }
    
    
    
    /**
     * This method sends the generated MMI response back to the client through an HTTP response 
     *
     * @param mmiMessageRequest
     * @param com.sun.sjc.idtv.vod.shared.data.RightsInfo[] 
     * @return MMIMessage
     */
    private void sendMMIMessageResponse(String userID, String contentID, String shopID, HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        
        com.sun.sjc.idtv.vod.shared.data.RightsInfo[] rightsInfo = extractRightsInfoFromVouchers(request, response);
        if(rightsInfo == null) {
            m_log.finer("NO RIGHTS INFO FOUND");
        } else {
            m_log.finer("NUMBER OF RIGHTS INFO FOUND: "+rightsInfo.length);
        }
        HashMap hashMap = (HashMap) getServletContext().getAttribute("SessionHashMap");
        MMIMessage mmiMessage = (MMIMessage) hashMap.get(request.getParameter("session_id"));
        if(mmiMessage == null) {
            m_log.finer("MMIMessage from Session is empty");
        }
        MMIMessage mmiResponse = generateRightsResponse(mmiMessage, rightsInfo);                
        response.setContentType("text/plain;charset=UTF-8");
        PrintWriter out = response.getWriter();
        
        out.println(mmiResponse.print("\n"));
        out.flush();
        out.close();
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
    License getLicenseFromDatabase(String userID, String contentID, String shopID)
    throws LicenseServerException {
        m_log.finer("Entering Function..");
        License lic = null;
        m_log.finer("Requesting license for "+userID+" "+contentID+" "+shopID);
        lic = License.getLicenseFromDatabase(userID, contentID, shopID);
        m_log.finer("Leaving Function..");
        return lic;
    }
    
    /**
     * m_authenticate is useful for development and stand alone testing of the License Server.
     * By setting the value of the configuration
     * value <b>authenticate</b> in the configuration file <b> /WEB-INF/conf/license-server.xml
     * to false we can disable the License Server from going to
     * the Opera Infrastructure for authentication.
     * A client of the license server should always be tested with authentication to
     * ensure that it will work properly.
     *
     */
    private boolean             m_authenticate       = true;
    private String              m_operaProxyURL;
    private String              m_keyDirectory;
    private DatabaseHelper      m_databaseHelper;
    private static Logger       m_log                = Logger.getLogger(LicenseServer.class.getName());
    private SimpleVoucherGenerator    m_voucherHandler = new SimpleVoucherGenerator();
    private static final long   serialVersionUID     = 7735661521175646355L;
}
