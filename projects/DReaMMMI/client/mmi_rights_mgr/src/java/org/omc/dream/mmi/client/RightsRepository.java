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
 * $(@)RightsRepository.java $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:36:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
package org.omc.dream.mmi.client;

import java.util.Hashtable;
import java.util.logging.*;
import java.io.*;


/**
 * This a simple repository that can store/cache rights information. 
 */
public class RightsRepository {
  
    /* Arbit string concatted to produce key */ 
    private static final String ACTION_STR = "ActionID";  

    private Hashtable _hash;
    private String repoPath;
    
    private static final Logger logger =
                           Logger.getLogger( RightsRepository.class.getName() );
    /**
     * This function tries to load a repository given a repoPath. 
     */ 
    private Hashtable loadRepo( String repoPath ) {
        FileInputStream fis = null;
        ObjectInputStream ois = null;
        Hashtable hash;

        try { 
            logger.fine("Trying to load repo");
            fis = new FileInputStream( repoPath );
            ois = new ObjectInputStream( fis );
            hash = (Hashtable)ois.readObject();
        } catch( IOException ex ) {
            logger.fine("Cannot load repo");
            ex.printStackTrace();
            hash = null;
        } catch( ClassNotFoundException cnf ) {
            logger.warning("Classnotfound exception occurred");
            cnf.printStackTrace();
            hash = null;
        }

        logger.fine("loaded repo hash:" + hash);
        return hash;
    }
    
    public RightsRepository( String repoPath ) {
        this.repoPath = repoPath;
        _hash = loadRepo( repoPath );
        
        /* couldnt recover repo. creating a new one */
        if( _hash == null ) {
            _hash = new Hashtable();    
        }
    }

    /**
     * Uses a concatenation of contentId and actionId as the key and the stat
     * object as the value. 
     */
    public void putStat( String contentId, ActionId actionId, ActionStat stat ) {
        String key = contentId + ACTION_STR + actionId;
        _hash.put( key, stat );
        logger.info( "Putting stat into repo. Key: " + key + " stat: " + stat );
    }

    /**
     * Given contentId and actionId, this function will return an actionStat. 
     */
    public ActionStat getStat( String contentId, ActionId actionId ) {
        String key = contentId + ACTION_STR + actionId;
        ActionStat stat = (ActionStat)_hash.get( key );
        logger.info( "Getting stat. Key: " + key + " stat: " + stat );
        return stat;
    }

    /**
     * Removes a stat object from the repo 
     */
    public void rmStat( String contentId, ActionId actionId ) {
        String key = contentId + ACTION_STR + actionId;
        _hash.remove( key );
    }

    /**
     * Write repository to file.
     */
    public void saveRepo() { 
        FileInputStream fis = null;
        ObjectInputStream ois = null;
        logger.info("Saving repo into " + repoPath );
        try {
            FileOutputStream fos = new FileOutputStream( repoPath );
            ObjectOutputStream oos = new ObjectOutputStream( fos );
            oos.writeObject( _hash );
            logger.finest("Wrote repo");
        } catch( IOException ex ) {
            logger.warning("Repo store failed");
            ex.printStackTrace();
        }
    }

    /**
     * Prints the contents of the hashtable 
     */
    public String toString() {
        return _hash.toString();                         
    }

    /**
     * Test code to create a sample repo
     */
    public static void main( String [] args ) {
        if( args.length < 2 ) {
            System.out.println("RightsRepository path_to_repo");
            return;
        }
        if( args[0].equals("create" ) ) {
            RightsRepository repo = new RightsRepository( args[1] );
            for( int i = 2; i < args.length; i+=3 ) {
                ActionStat stat = new ActionStat( Boolean.parseBoolean( args[i+2] ) ); 
                logger.info( "Putting into repo: "+ args[i] + " " +
                ActionId.valueOf(args[i+1]) + " " +  stat );
                repo.putStat( args[i], ActionId.valueOf(args[i+1]), stat );
            }

            repo.saveRepo();
        } else if (args[0].equals("read")) {
            RightsRepository repo = new RightsRepository( args[1] );
            System.out.println( repo );
        } else {
            System.err.println("Expected command `create` or `read`");
        }
        return;
    }

}
