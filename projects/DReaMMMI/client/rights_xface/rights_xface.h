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
 * $(@)rights_xface.h $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:37:53 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef _RIGHTS_QUERY_H
#define _RIGHTS_QUERY_H

/* 
 * TODO
 * Must include other verbs when we are capable of supporting them.
 */
typedef enum
{
    PLAY   = 0,
    RECORD = 1,
    FFWD   = 2,
    RWND   = 3,
    CLEAR_COPY = 4,
    ADAPT  = 5

}ActionId;

typedef struct
{
    int millis;    
}Period;

typedef struct
{
    int nr;     /* numerator */
    int dr;     /* denominator */
}Scale;

typedef enum
{
    N_A,
    DVD,
    CD
} DeviceTarget;

/*
 * TODO
 * Must extend the members of this struct to encompass all types of VerbSpecific
 * Arguments.
 */
typedef struct
{
    /* number of times some action was/can be performd */
    int          cnt; 

    /* TODO Device Location Information */
    /* DeviceLocation location; */
    
    /* Start time from when action can be performed */ 
    Period       start;       
    
    /* End time till when action can be performed */
    /* if the period is a relative value then set
    * start to ANY_PERIOD */ 
    Period       end;         
   
    /* Scale at which fast forward or rewuind must be
       performed */ 
    Scale        scale;       

    /* Target argument for some action. (e.g. record
       to target DVD).*/
    DeviceTarget target;          

    /* keys required to perform some action */
    unsigned char * keys;
    int keys_len;

} ActionStat;

/* XXX XXX 
 * Assumption: media_url contains some identifier that the rights manager knows
 * how to parse
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* returns a void * pointer to the xface_data that must be passed to other
 * functions such as canDoAction, reportAction etc. */
void *initRightsXFace();

void releaseRights( char * media_url );

unsigned char canDoFFWD( char *media_url );

unsigned char canDoRWND( char *media_url );

unsigned char canDoCCopy( char *media_url );

unsigned char canDoRecord( char *media_url );

unsigned char canDoPlay( char *media_url );

void reportFFWD( char * media_url );
void reportRWND( char * media_url );
void reportCCopy( char * media_url );
void reportPlay( char * media_url );
void reportRecord( char * media_url );

void fetchRights( char *media_url );
void prepareForPlay( char *media_url );
unsigned char isProtected( char* media_url );

/*
 * This should be replaced with preferrably some sort of a structure that
 * specifies which callbacks MUST be implemented by the vlc player. We dont want
 * to collect stats on every action possible.  
 */
void donePlay( char *media_url );
void doneRWND( char *media_url );
void doneFFWD( char *media_url );
void doneRecord( char *media_url );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RIGHTS_QUERY_H */
