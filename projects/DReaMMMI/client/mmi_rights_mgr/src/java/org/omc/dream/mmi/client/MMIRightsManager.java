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
 * $(@)MMIRightsManager.java $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:36:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
package org.omc.dream.mmi.client;

import java.util.Properties;
import java.util.logging.*;
import java.io.*;
import java.net.URL;
import java.net.MalformedURLException;

import org.omc.dream.mmi.common.*;

/**
 * Rights Manager translates rights requests from caller into MMIRightsRequest
 * messages. It may also implement storage for rights on the client system. 
 */
public class MMIRightsManager {
    
    private static final String CONFIG_FILE = "configure.txt"; 
   
    /* Keys in the config file */
    private static final String LICENSE_SERVER = "LicenseServer";
    private static final String PRIVATE_KEY_FILE = "PrivateKeyFile";
    private static final String LICENSE_NAME   = "LicenseName";
    private static final String PORT_NUMBER    = "PortNumber";
    private static final String ACTION         = "Action";
    private static final String REPO_PATH      = "RepoPath";
    private static final String CONTENT_ID_FIELD = "ContentIdField"; 
    private static final String DEBUG          = "Debug"; 

    /* other hardcodes */
    private static final String DEFAULT_REPO_PATH = "~/.mmirepo";
    private static final String DEF_CON_ID_FIELD = "content_id";

    private static final int RIGHTS_REQUEST = 0;
    private static final int RIGHTS_RELEASE = 1;
    private static final String MMI_RIGHTS_REQUEST_STR = "MMIRightsRequest";
    private static final String MMI_RIGHTS_RELEASE_STR = "MMIRightsRelease";

    /* logging */
    private static final Logger logger =
                           Logger.getLogger( MMIRightsManager.class.getName() );
   
    /* member variables */ 
    private RightsRepository _repo;
    private Properties       _props;
    private MMIClient        _mmiClient;


    public MMIRightsManager() {
        logger.entering("MMIRightsManager", "<init>");
        _props = new Properties();
        FileInputStream fis = null;
        try {
            fis = new FileInputStream( CONFIG_FILE );
            _props.load( fis );
            logger.finest("loaded config file");
        } catch ( IOException ex ) {
            logger.warning("Cannot open " + CONFIG_FILE );
            ex.printStackTrace();
            return;
        }

        _repo = new RightsRepository( _props.getProperty( REPO_PATH,
                                                           DEFAULT_REPO_PATH ) );
        

        _mmiClient = new MMIClient();
    }

    /**
     * parses the url to get content id. 
     */
    /*
    private String getContentId( String mediaUrl ) {
        String contentId;
        try {
            String conField = "&" + _props.getProperty( CONTENT_ID_FIELD,
                                                        DEF_CON_ID_FIELD ) + "=";

            int index = mediaUrl.lastIndexOf( conField );
            logger.finest("confield is " + conField);
            
            // look for first "&" in url from index, not counting the "&" before
            // content id field
            int endField = mediaUrl.indexOf( "&", index + 1 );
            logger.finest("endfield is " + endField);
            if( endField == -1 ) {
                endField = mediaUrl.length();
            }

            contentId = mediaUrl.substring( index + conField.length(), endField ); 
            logger.fine("content id is " + contentId);
        
        } catch( StringIndexOutOfBoundsException ex ) {
            logger.warning("content_id field probably not present in URL");
            ex.printStackTrace();
            return null;
        }

        return contentId;
    }
    */

    /**
     * Extracts contentId from the media url.
     */
    private String getContentId( String mediaUrl ) {

        // General algo: Look for .mpg, look for last slash before .mpg.
        // substring between the two indices is the tentative contentId.
        // Final check to see if there is a ".en" at the end. If so, trim it
        // off.

        String contentId;
        int l = mediaUrl.lastIndexOf(".mpg");
        if( l == -1 ) {
            System.err.println("Invalid mediaUrl");
            return null;
        }
        String tempUrl = mediaUrl.substring( 0, l );
        int m = tempUrl.lastIndexOf("/");
        if( m == -1 )
            m = 0;
        String mediaName = mediaUrl.substring( m+1, l );
    
        int e = mediaName.lastIndexOf(".en");
        if( e != -1 ) {
            contentId = mediaName.substring( 0, e );
        } else {
            contentId = mediaName;
        }

        logger.finest("ContentId is " + contentId);
        try {
        } catch ( Exception ex ) {
            ex.printStackTrace();
        }
        return contentId;
    }


    /**
     * Gets an appropriate verb for the actionId specified. 
    String getVerbString( int actionId ) {
        // XXX take care. This mapping comes from rights_xface.h
        // cross referenced with verbs allowed in the spec.
        // XXX
        switch( actionId ) {
        case 0: // PLAY
            return "SimplePlay"; 
        case 1: // RECORD
            return "Record";
        case 2: // FFWD
            return "ForwardPlay";
        case 3: // RWND
            return "ReversePlay";
        case 4: // CLEAR_COPY
            return "ClearCopy";
        case 5: // ADAPT
            return "Adapt";
        }
        return null;
    }
    */

    /**
     * Makes a MMImessage from ActionStat 
     */
     private MMIPlainTextMessage prepareMMIPlainTextMessage( int mesgType,
     String conId, ActionId actionId, ActionStat inStat ) {
  
        // XXX I should be looking at the actionId and then changing various
        // fields in the MMIRequest on that basis. However for now, I only
        // manipulate the count. 
   
        MMIMessageFactory mmf = MMIMessageFactory.PLAINTEXT;
        MMIPlainTextMessage mptm = (MMIPlainTextMessage)mmf.createMMIMessage();
        mptm.setMMIVersion("1.0");
        MMIRequest mmiRequest = new MMIRequest();
        try {
            if( mesgType == RIGHTS_REQUEST ) {
                mmiRequest.setMMIMessageType( MMI_RIGHTS_REQUEST_STR );
            } else {
                mmiRequest.setMMIMessageType( MMI_RIGHTS_RELEASE_STR );
            }
            IdentitySegment identitySegment = new IdentitySegment(new AuthServiceId("null"), null);            
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
            verbElement[0].setVerb(new Verb( actionId.toMMIVerbString() )); //get verb string here
            verbElement[0].setCount(new Count("" + inStat.num)); //set count here 
            
            /* Special case for clear copy. setting a verb specific arg */
            VerbSpecificArgs [] spec = null;             
            if ( actionId.equals( ActionId.CLEAR_COPY ) ) { /* XXX */
                spec = new VerbSpecificArgs[1];
                spec[0] = new VerbSpecificArgs("Target", inStat.target );
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

    /**
     * Given an MMIMessage this function will create an ActionStat that
     * contains the necessary information.
     */
    private ActionStat extractActionStat( MMIMessage mesg, ActionStat inStat ) {
        MMIResponse response = null;
        try {
            response =  (MMIResponse)mesg.getMMIDataObject();                 
        } catch( RuntimeException rex ) {
            rex.printStackTrace();
            logger.severe("message data object is probably not of type MMIResponse ");
        }
    
        // rights granted?
        ActionStat stat = null;
        MMIRightsResponseElement [] mre = response.getMMIRightsResponseElement();
        // Assume that the number of elements is 1
        if( mre[0].getNotification().getNotification().equalsIgnoreCase("granted") ) {
            stat = new ActionStat( true );
        } else {
            stat = new ActionStat( false );
        }
      
        if( mre[0].getNotification().getNotification().equalsIgnoreCase("error") ) {
            logger.warning("MMI notification: ERROR");
        }
        
        if( stat.getPermission() ) { 
            // get keys if any available 
            // logger.severe("Uncomment in production FROM HERE--------------------------");
            ///* UNCOMMENT in production XXX XXX
                byte [] keys;
                try {
                    keys = mre[0].getKeys().getKeys();
                } catch(Exception ex) {
                    ex.printStackTrace();
                    keys = null;
                }

                stat.keys = keys;
            //*/
            // logger.severe("Uncomment in production TO HERE--------------------------");
          
            // for now just copy all values in the instat into the outstat. 
            stat.num = inStat.num;
            stat.start = inStat.start;
            stat.end = inStat.end;
            stat.scale = inStat.scale;
            stat.target = inStat.target;    
        }

        return stat;
    }

    /**
     * Dispatches MMI request, parses the result and logs the result into the
     * repo. 
     */
    private ActionStat dispatchMMI( String contentId, ActionId actionId, String
    mediaUrl, int mesgType, ActionStat inStat ) {
        // XXX UNCOMMENT IN PRODUCTION XXX
        logger.severe("Uncomment in production FROM HERE--------------------------");
        
        String licenseServer = _props.getProperty( LICENSE_SERVER ); 
        String portNumber = _props.getProperty( PORT_NUMBER );
        String action = _props.getProperty( ACTION );
        logger.info("From props: licenseServer " + licenseServer + " portNumber " + portNumber);
        
        // create the url from where the mmiclient will fetch info
        URL url = null;
        try {
            url = new URL( "http", licenseServer, Integer.parseInt(portNumber), action );
        } catch (MalformedURLException ex) {
            ex.printStackTrace();
            logger.severe("Trouble forming url");
        }
        logger.info("url set on MMI: " + url );

        _mmiClient.setURL(url);

        // make an mmiPlainTextMessage
        MMIPlainTextMessage mesg = prepareMMIPlainTextMessage( mesgType, contentId, actionId, inStat );
        logger.info("MMIPlainTextMessage " + mesg.print("\n"));
        MMIMessage mmimesg = null;
        try { // request by GET
             mmimesg = _mmiClient.requestRightsByGET( mesg );
        } catch( Exception ex ) {
            ex.printStackTrace();
            logger.severe("requestRightsByGet failed!");
        }

        // from the mmimesg extract relevant information for our use.
        ActionStat outStat;
        if( _props.getProperty( DEBUG ) != null && _props.getProperty( DEBUG ).equals("true") )
            outStat = new ActionStat(true);
        else
            outStat = extractActionStat( mmimesg, inStat );
         
        return outStat;

        //return outStat;
        /* 
        logger.severe("Uncomment in production TO HERE--------------------------");
        byte [] keys = {
(byte)0x02,(byte)0x10,(byte)0x10,(byte)0x97,(byte)0x54,(byte)0xE3,(byte)0x34,(byte)0xC3,(byte)0x27,(byte)0xE8,(byte)0x36,(byte)0xEA,(byte)0x00,(byte)0xDA,(byte)0x83,(byte)0x14,(byte)0x4E,(byte)0xC6,(byte)0x21,(byte)0x9D,(byte)0x89,(byte)0x24,(byte)0xCD,(byte)0x6F,(byte)0x62,(byte)0x8D,(byte)0xD7,(byte)0x34,(byte)0x70,(byte)0x2F,(byte)0xEC,(byte)0x3B,(byte)0xFE,(byte)0x52,(byte)0x0E };
        ActionStat outStat = new ActionStat(true);
        outStat.keys = keys;
        return outStat;
        */
     }

    // ------------------------------------------------------------------
    // FUNCTIONS BELOW THIS CALLED FROM DRIVER CODE 
    /**
     * Called from driver code (possibly native code) to check if a particular
     * action is permitted under conditions stated in ActionStat. It first
     * checks local repository to see if rights already requested for. If not
     * then prepares MMI request, gets results and logs them. 
     */
    public boolean checkAction( int id, String url, 
                                 ActionStatNative natStat  ) {
        
        ActionId actionId = ActionId.valueOf( id );
        String contentId = getContentId( url );
        
        logger.info("ActionId " + actionId + " url " + url + " natStat " + natStat );
        // Convert it to something we can use.
        ActionStat inStat = new ActionStat( natStat ); 

        logger.finest("inStat " + inStat);
        
        // check in repo
        ActionStat outStat = _repo.getStat( contentId, actionId );
        if ( outStat == null || ( outStat.num == 0 && outStat.getPermission() == true ) ) {
            // not in repo? get from mmi
            if( outStat == null ) {
                logger.finest( actionId + " " + url + " stat not in repo" );
            } else {
                logger.finest( "Fetching using MMI since Outstat permission:" +
                outStat.getPermission() + " and outstat.num " + outStat.num );
            }
            outStat = dispatchMMI( contentId, actionId, url, RIGHTS_REQUEST, inStat );    
            logger.info(outStat.toString());
        }
        
        if ( outStat == null ) {
            logger.severe("stat after dispatchMMI is null");
            return false;      
        } else { // insert it back into the repository
            _repo.putStat( contentId, actionId, outStat );
        }
    
        // compare inStat and outStat and decided if request is permitted.
        boolean isPermitted = outStat.matchStat( inStat );
        logger.finest("Ispermitted? " + isPermitted);
         
        // doesn't do anything. forces the instat to update values in the native
        // object. These will become visible to the native code on return from
        // this function. 
        inStat.getNativeObject();

        return isPermitted;
    }

    /**
     * Release all rights that may have been acquired previously 
     */
    public void releaseRights( String url ) {
        String contentId = getContentId( url );
        
        // cycle through all actions and release rights if action was previously
        // granted.
        for( ActionId id : ActionId.values() ) {
            ActionStat inStat = _repo.getStat( contentId, id );    
            if( inStat != null && inStat.getPermission() && inStat.num > 0 ) {
                ActionStat outStat = dispatchMMI( contentId, id, url, RIGHTS_RELEASE, inStat );
                if( outStat.getPermission() == true ) {
                    logger.info( id + " rights released for " + url );
                } else {
                    logger.warning("Cannot release " + id + " rights for " + url);
                }
            }
            
            logger.info("Clearing cache for " + contentId + " " + id);
            _repo.rmStat( contentId, id );
        }
    }
 
    /**
     * Readies a media for consumption be getting keys. Keys are stored in a
     * license file which is configured from the configure.txt 
     */ 
    public void prepareKeys( int id, String url )
    {
        ActionId actionId = ActionId.valueOf( id );
        String contentId = getContentId( url );
        logger.info("PrepareKeys: actionId " + actionId + " contentId " + contentId );        

        /* Get stat from the repo */
        ActionStat stat = _repo.getStat( contentId, actionId );
        
        /* get keys from the stat and print to */ 
        String licenseName = _props.getProperty( LICENSE_NAME );
        logger.info("LicenseName value is " + licenseName);

        /* We dont have enough rights to prepare keys */
        if( stat.num <= 0 ) {
            logger.warning("insufficient rights to prepare keys");         

            if( _props.getProperty( DEBUG ) != null && _props.getProperty( DEBUG ).equals("true") ) 
                ;
            else {
                File f = new File( licenseName );
                f.delete();
                logger.info("No keys. Deleting license file");
                return;
            }
        }

        
        stat.num--;
      
        // replace the stat with the num decremented
        _repo.putStat( contentId, actionId, stat );

        FileOutputStream outStream; 
        if( _props.getProperty( DEBUG ) != null && _props.getProperty( DEBUG ).equals("true") )
            ;
        else {
            try {
                if( stat.keys != null  ) {
                    outStream = new FileOutputStream( licenseName );
                    outStream.write( stat.keys );
                    outStream.close();
                    logger.info("Wrote to license file");
                } else {
                    File f = new File( licenseName );
                    f.delete();
                    logger.info("No keys. Deleting license file");
                }
            } catch( Exception ex ) {
                ex.printStackTrace();
                logger.warning("Cannot open licenseName ");
            }
        }

        return;
    }
   
    /**
     * Called from driver code (possibly native code) to report the usage of
     * rights for a particular action and media url. This information is updated
     * in the local repository and may be used in determining the result of
     * future checkActions. 
     */ 
    public void reportUsage( int id, String url ) {
        ActionId actionId = ActionId.valueOf( id );
        String contentId = getContentId( url );
        logger.info("Reporting action " + actionId + " url " + url );
        ActionStat stat = _repo.getStat( contentId, actionId );
       
        if( stat != null ) {
            logger.finest("Decrementing stat.num");
            stat.num--;
        }

        _repo.putStat( contentId, actionId, stat );
    }
}
