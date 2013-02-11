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
 * $(@)MMIClient.java $Revision: 1.2 $ $Date: 2006/09/20 01:33:37 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
package org.omc.dream.mmi.client;
import java.io.*;
import java.net.*;
import javax.net.ssl.*;
import org.omc.dream.mmi.common.*;

/**
 * 
 * 
 * @poseidon-object-id [Im3611ec07m10a5e3fe01emm6cc9]
 */
public class MMIClient {

/**
 * 
 * 
 * @poseidon-object-id [Im3611ec07m10a5e3fe01emm6cb8]
 */
    private HttpURLConnection hucMmiServer;

/**
 * 
 * 
 * @poseidon-object-id [I5ceaaa43m10a71a32fbamm6cfa]
 */
    private URL url;

/**
 * Creates a new instance of MMIClient
 * 
 * @poseidon-object-id [Im3611ec07m10a5e3fe01emm6cb2]
 */
    public  MMIClient() {        
        url=null;
        hucMmiServer=null;
    } 

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm6862]
 * @param strUrl 
 */
    public  MMIClient(String strUrl) {        
        try {
            url = new URL(strUrl);
        } catch(MalformedURLException mue) {
            mue.printStackTrace();
        }
    } 


/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm680b]
 * @param url 
 */
    public void setURL(URL url) {        
        this.url = url;
    } 

/**
 * 
 * 
 * @poseidon-object-id [I11454a74m10ad1e2cbd2mm67e7]
 * @return URL
 */
    public URL getURL() {        
        return url;
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im3611ec07m10a5e3fe01emm6ca5]
 * @return MMIMessage
 * @param mmiPlainTextMessage 
 */
    public MMIMessage requestRightsByGET(MMIPlainTextMessage mmiPlainTextMessage) throws MMIClientException {        
        if (mmiPlainTextMessage == null) {
            throw new MMIClientException("Empty request");            
        }
        StringBuffer sb = new StringBuffer();
        MMIMessage mmiMessage = null;
        try {            
            String strURL = new String(url.toString()+"?"+mmiPlainTextMessage.print("&"));
            URL newurl = new URL(strURL);
            hucMmiServer = (HttpURLConnection)newurl.openConnection();
            hucMmiServer.setDoOutput(false);
            hucMmiServer.setDoInput(true);
            hucMmiServer.setRequestMethod("GET");
            hucMmiServer.setRequestProperty("Content-Type","text/plain");
            hucMmiServer.setUseCaches(false);
            hucMmiServer.connect();
            BufferedReader br = null;
            String strTemp = new String();
            MMIParserFactory mmiParserFactory = MMIParserFactory.PLAINTEXT;
            MMIPlainTextParser mmiPlainTextParser = (MMIPlainTextParser)mmiParserFactory.createParser();
            try {
                System.out.println("Parsing response...");
                mmiMessage = mmiPlainTextParser.parseMessage(new InputStreamReader(hucMmiServer.getInputStream()));
                mmiMessage.print("\\&");
                System.out.println("Finished parsing response.");
                
            } catch(InvalidMMIObjectException imoe) {
                imoe.printStackTrace();
            } catch(IOException ioe) {
                ioe.printStackTrace();
            }
        } catch(IOException ioe) {
            sb = null;
            ioe.printStackTrace();
        } finally {
            return mmiMessage;
            //return sb.toString();
        }
    } 

/**
 * 
 * 
 * @poseidon-object-id [Im7deb1398m10af0a9f5a5mm5c3f]
 * @return 
 * @param mmiPlainTextMessage 
 */
    public MMIMessage requestRightsByPOST(MMIPlainTextMessage mmiPlainTextMessage) throws MMIClientException {        
        if (mmiPlainTextMessage == null) {
            throw new MMIClientException("Empty request");            
        }
        StringBuffer sb = new StringBuffer();
        MMIMessage mmiMessage = null;
        try {
            hucMmiServer = (HttpURLConnection)url.openConnection();
            hucMmiServer.setDoOutput(true);
            hucMmiServer.setDoInput(true);
            hucMmiServer.setRequestMethod("POST");
            hucMmiServer.setRequestProperty("Content-Type","text/plain");
            hucMmiServer.setUseCaches(false);
            hucMmiServer.connect();
            BufferedWriter bw = null;
            try {
                bw = new BufferedWriter(new OutputStreamWriter(hucMmiServer.getOutputStream()));
                bw.write(mmiPlainTextMessage.print("\n"));                
                bw.flush();
                //bw.close();
            } catch(IOException ioe) {
                ioe.printStackTrace();
            } finally {
                bw.close();
            }
            MMIParserFactory mmiParserFactory = MMIParserFactory.PLAINTEXT;
            MMIPlainTextParser mmiPlainTextParser = (MMIPlainTextParser)mmiParserFactory.createParser();
            try {
                mmiMessage = mmiPlainTextParser.parseMessage(new InputStreamReader(hucMmiServer.getInputStream()));
            } catch(InvalidMMIObjectException imoe) {
                imoe.printStackTrace();
            }
            
        } catch(IOException ioe) {
            ioe.printStackTrace();
            
        } finally {
            return mmiMessage;
//        return sb.toString();
        }
    } 
    
     private MMIPlainTextMessage sampleMMIMessage( String conId, String action, boolean release ) {
  
        // XXX I should be looking at the actionId and then changing various
        // fields in the MMIRequest on that basis. However for now, I only
        // manipulate the count. 
   
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            if(release) {
                mmiRequest.setMMIMessageType("MMIRightsRelease");
            } else {
                mmiRequest.setMMIMessageType("MMIRightsRequest");                
            }
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("Anakin"), null);            
            mmiRequest.setIdentitySegment(identitySegment);
            DeviceSegment deviceSegment = new DeviceSegment();
            DeviceId[] deviceId = new DeviceId[1];
            deviceId[0] = new DeviceId("123456abc");           
            deviceSegment.setDeviceId(deviceId);
            mmiRequest.setDeviceSegment(deviceSegment);
            
            RightsSegment rightsSegment = new RightsSegment();
            rightsSegment.setProfileId(new ProfileId("org.omc.dream.profiles.media"));
            MMIRightsRequestElement[] mmiRightsRequestElement = 
                    new MMIRightsRequestElement[1]; // Made this a single element.
            mmiRightsRequestElement[0] = new MMIRightsRequestElement();
            mmiRightsRequestElement[0].setReqElemId(new ReqElemId("23"));
            ContentId[] contentId = new ContentId[1];
            contentId[0] = new ContentId( conId ); // added conId here
            mmiRightsRequestElement[0].setContentId(contentId);
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb(action)); //get verb string here
            verbElement[0].setCount(new Count("" + 1)); //set count here 
            
            /* Special case for clear copy. setting a verb specific arg */
            VerbSpecificArgs [] spec = null;             
            if ( action.equalsIgnoreCase("ClearCopy") ) { /* XXX */
                spec = new VerbSpecificArgs[1];
                spec[0] = new VerbSpecificArgs("Target", "Disc" );
                verbElement[0].setVerbSpecificArgs( spec );
            }
            
            mmiRightsRequestElement[0].setVerbElement(verbElement);
            rightsSegment.setMMIRightsRequestElement(mmiRightsRequestElement);
            
            mmiRequest.setRightsSegment(rightsSegment);
            mptm.setMMIDataObject(mmiRequest);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;         
     }
    
    
    
    
    
    public MMIPlainTextMessage SimpleRightsRelease() {
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            mmiRequest.setMMIMessageType("MMIRightsRelease");
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("Anakin"), null);            
            mmiRequest.setIdentitySegment(identitySegment);
            DeviceSegment deviceSegment = new DeviceSegment();
            DeviceId[] deviceId = new DeviceId[1];
            deviceId[0] = new DeviceId("123456abc");           
            deviceSegment.setDeviceId(deviceId);
            mmiRequest.setDeviceSegment(deviceSegment);
            
            RightsSegment rightsSegment = new RightsSegment();
            rightsSegment.setProfileId(new ProfileId("org.omc.dream.profiles.media"));
            MMIRightsRequestElement[] mmiRightsRequestElement = new MMIRightsRequestElement[1];
            mmiRightsRequestElement[0] = new MMIRightsRequestElement();
            mmiRightsRequestElement[0].setReqElemId(new ReqElemId("23"));
            ContentId[] contentId = new ContentId[1];            
            contentId[0] = new ContentId("birds");
            mmiRightsRequestElement[0].setContentId(contentId);
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb("SimplePlay"));
            verbElement[0].setCount(new Count("1"));
            mmiRightsRequestElement[0].setVerbElement(verbElement);
            rightsSegment.setMMIRightsRequestElement(mmiRightsRequestElement);
            
            mmiRequest.setRightsSegment(rightsSegment);
            mptm.setMMIDataObject(mmiRequest);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;        
    }

    public MMIPlainTextMessage SimpleRightsRelease2() {
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            mmiRequest.setMMIMessageType("MMIRightsRelease");
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("Anakin"), null);            
            mmiRequest.setIdentitySegment(identitySegment);
            DeviceSegment deviceSegment = new DeviceSegment();
            DeviceId[] deviceId = new DeviceId[1];
            deviceId[0] = new DeviceId("123456abc");           
            deviceSegment.setDeviceId(deviceId);
            mmiRequest.setDeviceSegment(deviceSegment);
            
            RightsSegment rightsSegment = new RightsSegment();
            rightsSegment.setProfileId(new ProfileId("org.omc.dream.profiles.media"));
            MMIRightsRequestElement[] mmiRightsRequestElement = new MMIRightsRequestElement[1];
            mmiRightsRequestElement[0] = new MMIRightsRequestElement();
            mmiRightsRequestElement[0].setReqElemId(new ReqElemId("23"));
            ContentId[] contentId = new ContentId[1];            
            contentId[0] = new ContentId("blahblah");
            mmiRightsRequestElement[0].setContentId(contentId);
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb("SimplePlay"));
            verbElement[0].setCount(new Count("1"));
            mmiRightsRequestElement[0].setVerbElement(verbElement);
            rightsSegment.setMMIRightsRequestElement(mmiRightsRequestElement);
            
            mmiRequest.setRightsSegment(rightsSegment);
            mptm.setMMIDataObject(mmiRequest);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;        
    }
    
    
    public MMIPlainTextMessage SimpleRightsRequest() {
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            mmiRequest.setMMIMessageType("MMIRightsRequest");
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("Anakin"), null);            
            mmiRequest.setIdentitySegment(identitySegment);
            DeviceSegment deviceSegment = new DeviceSegment();
            DeviceId[] deviceId = new DeviceId[1];
            deviceId[0] = new DeviceId("123456abc");           
            deviceSegment.setDeviceId(deviceId);
            mmiRequest.setDeviceSegment(deviceSegment);
            
            RightsSegment rightsSegment = new RightsSegment();
            rightsSegment.setProfileId(new ProfileId("org.omc.dream.profiles.media"));
            MMIRightsRequestElement[] mmiRightsRequestElement = new MMIRightsRequestElement[1];
            mmiRightsRequestElement[0] = new MMIRightsRequestElement();
            mmiRightsRequestElement[0].setReqElemId(new ReqElemId("23"));
            ContentId[] contentId = new ContentId[1];            
            contentId[0] = new ContentId("birds");
            mmiRightsRequestElement[0].setContentId(contentId);
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb("SimplePlay"));
            verbElement[0].setCount(new Count("1"));
            mmiRightsRequestElement[0].setVerbElement(verbElement);
            rightsSegment.setMMIRightsRequestElement(mmiRightsRequestElement);
            
            mmiRequest.setRightsSegment(rightsSegment);
            mptm.setMMIDataObject(mmiRequest);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;        
    }

    public MMIPlainTextMessage SimpleRightsRequest2() {
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            mmiRequest.setMMIMessageType("MMIRightsRequest");
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("Anakin"), null);            
            mmiRequest.setIdentitySegment(identitySegment);
            DeviceSegment deviceSegment = new DeviceSegment();
            DeviceId[] deviceId = new DeviceId[1];
            deviceId[0] = new DeviceId("123456abc");           
            deviceSegment.setDeviceId(deviceId);
            mmiRequest.setDeviceSegment(deviceSegment);
            
            RightsSegment rightsSegment = new RightsSegment();
            rightsSegment.setProfileId(new ProfileId("org.omc.dream.profiles.media"));
            MMIRightsRequestElement[] mmiRightsRequestElement = new MMIRightsRequestElement[1];
            mmiRightsRequestElement[0] = new MMIRightsRequestElement();
            mmiRightsRequestElement[0].setReqElemId(new ReqElemId("23"));
            ContentId[] contentId = new ContentId[1];            
            contentId[0] = new ContentId("blahblah");
            mmiRightsRequestElement[0].setContentId(contentId);
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb("SimplePlay"));
            verbElement[0].setCount(new Count("1"));
            mmiRightsRequestElement[0].setVerbElement(verbElement);
            rightsSegment.setMMIRightsRequestElement(mmiRightsRequestElement);
            
            mmiRequest.setRightsSegment(rightsSegment);
            mptm.setMMIDataObject(mmiRequest);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;        
    }
    
    public MMIPlainTextMessage SimpleResponse() {
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIResponse mmiResponse = new MMIResponse();
        try {            
            Status[] status = new Status[1];
	    status[0] = new Status("RequestOK");
            mmiResponse.setStatus(status);

            MMIRightsResponseElement[] mmiRightsResponseElement = new MMIRightsResponseElement[1];
            mmiRightsResponseElement[0] = new MMIRightsResponseElement();
            mmiRightsResponseElement[0].setReqElemId(new ReqElemId("23"));
            mmiRightsResponseElement[0].setNotification(new Notification("granted"));
            Hint[] hint = new Hint[1];
            hint[0] = new Hint();
            hint[0].setHintIndexNum(new HintIndexNum("1"));
            hint[0].setLabel(new Label("CanDo"));
            
            ContentId[] contentId = new ContentId[3];
            contentId[0] = new ContentId("113");
            contentId[1] = new ContentId("114");
            contentId[2] = new ContentId("115");
            
            VerbElement[] verbElement = new VerbElement[1];
            verbElement[0] = new VerbElement();
            verbElement[0].setVerbElementId(new VerbElementId("1"));
            verbElement[0].setVerb(new Verb("SimplePlay"));
            verbElement[0].setCount(new Count("29"));
            hint[0].setVerbElement(verbElement);
            hint[0].setContentId(contentId);
                                  
            mmiRightsResponseElement[0].setHint(hint);
            mmiRightsResponseElement[0].setKeys(new Keys("OD6Ox9svtSgFJ+iXkZ"));
            //mmiRightsResponseElement[0].setRightsErrorStatus();
            
            
            mmiResponse.setMMIRightsResponseElement(mmiRightsResponseElement);
            
            ResponseId responseId = new ResponseId();
            responseId.setResponseId("1003");
            mmiResponse.setResponseId(responseId);
            
            RequestHashSegment requestHashSegment = new RequestHashSegment();
            requestHashSegment.setHashAlg(new HashAlg("http://www.w3.org/2001/10/xml-exc-c14n#"));
            requestHashSegment.setRequestHash(new RequestHash("jAxX0LfgwutvEdJb748IU4L+8obXPXfq TZ"));
            mmiResponse.setRequestHashSegment(requestHashSegment);
            
            SignatureSegment signatureSegment = new SignatureSegment();
            signatureSegment.setSigAlg(new SigAlg("http://www.w3.org/2001/10/xml-exc-c14n#"));
            signatureSegment.setSignature(new Signature("OWqP5Gqm8A1+/2b5gNzF4L4L"));
            mmiResponse.setSignatureSegment(signatureSegment);
            
            
            mptm.setMMIDataObject(mmiResponse);            
        } catch (InvalidMMIObjectException imoe) {
            imoe.printStackTrace();
        }
        return mptm;        
    }
    

    
/**
 * 
 * 
 * @poseidon-object-id [Im3611ec07m10a5e3fe01emm6c9d]
 * @param args the command line arguments
 */
    public static void main(String[] args) {        
        // TODO code application logic here
        if (args.length < 1) {
            System.out.println("require URL argument");
            System.exit(1);
        }
        MMIClient client = new MMIClient(args[0]);
        /* The following two messages are used to create a request and response message
         * NOTE: The servlet on the other other side will only read the message, parse it,
         * and then send back the same message.  This test is only to validate that the parser
         * can:
         * - parse through a MMI message in plain text and generate MMIObject
         * - test if the MMIMessage.print() results in the same message
         */
        try {
            MMIPlainTextMessage msg = client.sampleMMIMessage( "birds", "SimplePlay", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "SimplePlay", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "Record", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "Record", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ForwardPlay", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ForwardPlay", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ReversePlay", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ReversePlay", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ClearCopy", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "ClearCopy", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "Adapt", false );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            msg = client.sampleMMIMessage( "birds", "Adapt", true );
            System.out.println("REQUEST message: "+msg.print("&")); 
            System.out.println("Regurgitated REQUEST message from servlet: "+client.requestRightsByGET(msg).print("\n")); 
            
        } catch (MMIClientException mce) {
            mce.printStackTrace();
        }
    } 
 }
