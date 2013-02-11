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
 * $(@)rights_xface.c $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:37:53 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <unistd.h>
#include "rights_xface.h"


/* XXX This variable may need mutex protection.*/
void *xface_ptr;

void * initRightsXFace()
{
    return (void *)(jniInit( (char *)0, (char *)0 ));
}

static unsigned char isEncrypted( char *media_url )
{
    char *isEnc = strcasestr( media_url, ".en.mpg" );
    return ( isEnc ? 1 : 0 );
}

/**
 * Says if the given media url is rights protected or not. 
 */
unsigned char isProtected( char* media_url )
{
    char *isRTSP = strcasestr( media_url, "rtsp://");
    return ( isRTSP ? 1:0 ); 
}

static unsigned char canDoAction( ActionId id, char * media_url, 
                    ActionStat * stat )
{
    if( !isEncrypted( media_url ) ) {
        return 1;
    }

    unsigned char ret = jniCheckRights( xface_ptr, id, media_url, stat);            
    
#ifdef _DEBUG
    fprintf( stderr, "\n%s:%d::media_url %s retval %d", __FILE__, __LINE__,
    media_url, ret );
#endif /* _DEBUG */
    return ret;
}

/* TODO later on this should take a ActionStat as well. ActionStat will carry
 * all the usage statistics that must be decremented from the cache */
static void reportAction( ActionId id, char * media_url )
{
    jniReportUsage( xface_ptr, id, media_url );
}

void reportFFWD( char * media_url )
{
    reportAction( FFWD, media_url );    
}

void reportRWND( char * media_url )
{
    reportAction( RWND, media_url ); 
}

void reportCCopy( char * media_url )
{
    reportAction( CLEAR_COPY, media_url ); 
}

void reportPlay( char * media_url )
{
    reportAction( PLAY, media_url ); 
}

void reportRecord( char * media_url )
{
    reportAction( RECORD, media_url );    
}


/* Free ActionStat after use. */
ActionStat *createDefaultActionStat()
{
    ActionStat *stat = (ActionStat *)malloc( sizeof(ActionStat) );
    stat->start.millis = 0;
    stat->end.millis   = 0;
    stat->cnt   = 1;
    stat->scale.nr = 1;
    stat->scale.dr = 1;
    stat->target = N_A;
    stat->keys = 0;
    stat->keys_len = 0;

#ifdef _DEBUG
fprintf( stderr, "\n%s:%d::ActionStat = %p", __FILE__, __LINE__, stat );
#endif /* _DEBUG */

    return stat;
}

void freeActionStat( ActionStat *stat ) 
{
    free(stat->keys); 
    free(stat);
}

unsigned char canDoPlay( char *media_url ) 
{
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 1;
    retVal = canDoAction( PLAY, media_url, stat ); 
    freeActionStat(stat);
    return retVal;
}

unsigned char canDoFFWD( char *media_url )
{
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 4; /* for now all ffwd is 4x */
    retVal = canDoAction( FFWD, media_url, stat );
    freeActionStat(stat);
    return retVal;
}

/*
 * checks if rewind is allowed. if return is non null then true else false.
 * return value is the key required for rewinding - if applicable.  
 */
unsigned char canDoRWND( char *media_url  )
{
    /*
    return canDoRWND( xface_ptr, RWND, media_url );
    */    
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 4; /* for now all ffwd is 4x */
    retVal = canDoAction( RWND, media_url, stat );
    freeActionStat(stat);
    return retVal;

}

unsigned char canDoRecord( char *media_url  )
{
    /*
    return canDoRWND( xface_ptr, RWND, media_url );
    */    
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 4; /* for now all ffwd is 4x */
    retVal = canDoAction( RECORD, media_url, stat );
    freeActionStat(stat);
    return retVal;
}

unsigned char canDoCCopy( char *media_url  )
{
    /*
    return canDoRWND( xface_ptr, RWND, media_url );
    */    
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 4; /* for now all ffwd is 4x */
    retVal = canDoAction( CLEAR_COPY, media_url, stat );
    freeActionStat(stat);
    return retVal;
}

void fetchRights( char * media_url ) {
    if( xface_ptr == NULL ) {
        xface_ptr = initRightsXFace();
    }
    canDoPlay( media_url );
    canDoFFWD( media_url );
    canDoRWND( media_url );
    canDoCCopy( media_url );
    canDoRecord( media_url );
}

void releaseRights( char * media_url ) {
    int i = 0;
    
    jniReleaseRights( xface_ptr, media_url );
    
}

void prepareForPlay( char * media_url ) {
    jniPrepareKeys( xface_ptr, media_url, PLAY ); 
}

#if _TESTS

unsigned char testCanDo( int id, char *media_url, int cnt )
{
    unsigned char retVal;
    ActionStat *stat = createDefaultActionStat();
    stat->scale.nr = 4; /* for now all ffwd is 4x */
    stat->cnt = cnt;
    retVal = canDoAction( id, media_url, stat );
    freeActionStat(stat);
    return retVal;
   
}

int main( int argc, char **argv )
{
    xface_ptr = initRightsXFace();
//    char * media_url = "rtsp://pn5.eng.sun.com:11000/javacar_en.mpg&cas=1&content_id=pianist&dmduid=a&userid=John&shopid=operaShop0";
    char * media_url="rtsp://localhost:11000/npng.mpg/hoohaa?dada&content_id=npng.mpg";
    // canDoFFWD( media_url );     
    testCanDo( FFWD, media_url, 1 );     
    testCanDo( RWND, media_url, 1 );     
    testCanDo( CLEAR_COPY, media_url, 1 );     
    testCanDo( PLAY, media_url, 1 );
    testCanDo( RECORD, media_url, 1);

    testCanDo( FFWD, media_url, 10 );     
    testCanDo( RWND, media_url, 10 );     
    testCanDo( CLEAR_COPY, media_url, 10 );     
    testCanDo( PLAY, media_url, 10 );
    testCanDo( RECORD, media_url, 10 );
}

#endif /* _TESTS */
