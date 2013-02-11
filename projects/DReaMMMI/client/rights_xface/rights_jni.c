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
 * $(@)rights_jni.c $Revision: 1.1.1.1 $ $Date: 2006/07/31 17:37:53 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <jni.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "rights_xface.h"

#define PATH_SEPARATOR ';'                /* define it to be ':' on Solaris */

#ifndef DEFAULT_CLASSPATH
#define DEFAULT_CLASSPATH "../mmi_client"             /* classpath */
#endif /* DEFAULT_CLASSPATH */

#define LOGGING_PATH "./logging.properties"

#define ACTION_STAT_CLASS_NAME "org/omc/dream/mmi/client/ActionStatNative" /* name of actionStat class */
#define RIGHTS_CLASS_NAME "org/omc/dream/mmi/client/MMIRightsManager"   /* name of java mmi class */ 
#define STRING_CLASS_NAME "java/lang/String"

#define CHECK_ACTION "checkAction"
#define CHECK_ACTION_SIG "(ILjava/lang/String;Lorg/omc/dream/mmi/client/ActionStatNative;)Z"

#define REPORT_USAGE "reportUsage"
#define REPORT_USAGE_SIG "(ILjava/lang/String;)V"

#define PREPARE_KEYS "prepareKeys"
#define PREPARE_KEYS_SIG "(ILjava/lang/String;)V"

#define RELEASE_RIGHTS "releaseRights"
#define RELEASE_RIGHTS_SIG "(Ljava/lang/String;)V"

#define CONS_SIG "()V"                   /* signature of a simple constructor */

typedef struct 
{

    JavaVM      *jvm;         
    
    /* this should be loaded everytime for a jni call from a new thread */
    JNIEnv      *env;        

    /* following variables have global references.*/
    jclass      rights_class; 
    jclass      action_stat_class; 
    jclass      string_class;
    jobject     rights_mgr;

}JNIData;

/*
 * checks for exceptions  
 */
unsigned char checkException( JNIEnv *env )
{
    jthrowable exc;
    exc = (*env)->ExceptionOccurred(env);
    if( exc ) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear( env );
        return 1;
    }

    return 0;
}

/* 
 * only creates objects with simple constructors without args 
 * if classname is NULL then gclass must not be NULL;
 * gclass can be non NULL if there is a cached class reference already
 * available.
 */
jobject createNewJNIObject( JNIEnv *env, char *classname, jclass gclass ) 
{
    jclass cls;
    if ( env == NULL ) {
        return NULL;
    }

    if ( classname != NULL ) {
        cls = (*env)->FindClass( env, classname );    
        if ( cls == NULL ) {
            checkException( env );
            fprintf( stderr, "\ncreateNewJNIObject: cannot find class %s", classname ); 
            return NULL;
        }
    } else if ( gclass == NULL ) {
        fprintf( stderr, "\ncreateNewJNIObject: gclass cannot be NULL");       
        return NULL;
    } else {
        cls = gclass;
    }

    jmethodID mid = (*env)->GetMethodID( env, cls, "<init>", CONS_SIG );
    if ( mid == NULL ) {
        checkException( env );
        fprintf( stderr, "\ncreateNewJNIObject: no simple constructor found"); 
        return NULL;
    }

    jobject obj = (*env)->NewObject( env, cls, mid ); 
    if ( obj == NULL ) {
        checkException( env );
        fprintf( stderr, "\ncreateNewJNIObject: cannot instantiate object");
        return NULL;
    }
   
    /* create a weak global reference. In case we forget to delete the reference */ 
    jobject gobj = (*(env))->NewWeakGlobalRef( env, obj );
    if ( gobj == NULL ) {
        checkException( env );
    }
    (*(env))->DeleteLocalRef( env, obj );
   
    #ifdef _DEBUG
    fprintf( stderr, "\n%s:%d:: weak gloabl ref created for class %p ptr: %p ", __FILE__, __LINE__, cls, gobj );
    #endif /* _DEBUG */
     
    return gobj; 
}

void clearRefs( JNIData *data )
{
    if ( data->rights_class != NULL ) {
        (*(data->env))->DeleteGlobalRef( data->env, data->rights_class );
        data->rights_class = NULL;
    }

    if( data->rights_mgr != NULL ) {
        (*(data->env))->DeleteGlobalRef( data->env, data->rights_mgr );
        data->rights_mgr = NULL;
    }

    if ( data->action_stat_class != NULL ) {
        (*(data->env))->DeleteGlobalRef( data->env, data->action_stat_class );
        data->action_stat_class = NULL;
    }

    if ( data->string_class != NULL ) {
        (*(data->env))->DeleteGlobalRef( data->env, data->string_class );
        data->string_class = NULL;
    }
}

/* Creates a jvm and returns a pointer to that */
void * jniInit( char * classpath )
{
    jint res;
    char *default_classpath = DEFAULT_CLASSPATH;
    char user_classpath[1024];
    char *env_classpath;

    JNIData *jni_data = (JNIData *)malloc( sizeof( JNIData ) );
    if ( jni_data == NULL ) {
        fprintf( stderr, "\n%s:%d::Malloc failed", __FILE__, __LINE__ );
        exit( -1 );
    }
    /*
    if( classpath == NULL ) {
        env_classpath = getenv("CLASSPATH");
        
        if( env_classpath == NULL ) {
            user_classpath = (char *)malloc( strlen( default_classpath ) );
            strcpy( user_classpath, default_classpath );
        } else {
            user_classpath = (char *)malloc( strlen( env_classpath ) );
            strcpy( user_classpath, env_classpath );
        }
    } else {
        user_classpath = (char *)malloc( strlen( classpath ) );
        strcpy( user_classpath, env_classpath );
    }
    */
    env_classpath = getenv("CLASSPATH");
    if( env_classpath ) {
        strcpy( user_classpath, env_classpath );
    }
#ifdef _DEBUG
    fprintf( stderr, "\n%s:%d::classpath %s\n", __FILE__, __LINE__, user_classpath );
#endif /* _DEBUG */

    
    /* store class name */
    
#ifndef JNI_VERSION_1_2
    fprintf(stderr, "\nOnly JNI version 1.2 is supported");
    exit(-1);
#endif /* JNI_VERSION_1_2 */

    JavaVMInitArgs vm_args;
    JavaVMOption options[3];
    
    options[0].optionString = (char *) malloc( strlen(user_classpath) + 30 );
    strcpy( options[0].optionString, "-Djava.class.path=" ); 
    strcat( options[0].optionString, user_classpath );
     
    options[1].optionString = (char *) malloc( strlen(LOGGING_PATH) + 50 );
    strcpy( options[1].optionString, "-Djava.util.logging.config.file=" ); 
    strcat( options[1].optionString, LOGGING_PATH );

#ifdef _DEBUG
fprintf( stderr, "\n%s:%d::jniinit - loggingpath %s",  __FILE__, __LINE__, options[1].optionString );
#endif /* _DEBUG */

    vm_args.version = 0x00010002;
    vm_args.options = options;
    vm_args.nOptions = 2;
    vm_args.ignoreUnrecognized = JNI_FALSE;
    /* Create the Java VM */
    res = JNI_CreateJavaVM(&(jni_data->jvm), (void**)&(jni_data->env), &vm_args);

    free(options[0].optionString);
    free(options[1].optionString);
    /* free( user_classpath ); */

    if (res < 0) {
        fprintf(stderr, "Can't create Java VM\n");
        clearRefs( jni_data );
        free( jni_data );
        return NULL;
    }

    /* create and cache global to rights_class */
    jclass lcls1 = (*(jni_data->env))->FindClass( jni_data->env,
                                                  RIGHTS_CLASS_NAME
                                                 );
    jni_data->rights_class = (*(jni_data->env))->NewGlobalRef( jni_data->env, lcls1 );
    if ( jni_data->rights_class == NULL ) {
        checkException( jni_data->env );
    }
    (*(jni_data->env))->DeleteLocalRef( jni_data->env, lcls1 );
    
    /* creating global reference to rights manager */ 
    jobject lobj = createNewJNIObject( jni_data->env, NULL, jni_data->rights_class );
    jni_data->rights_mgr = (*(jni_data->env))->NewGlobalRef( jni_data->env, lobj );
    /* Since createNewJNIObject returns an object with a weak global ref */
    (*(jni_data->env))->DeleteWeakGlobalRef( jni_data->env, lobj );


    /* create and cache global to action stat class */
    jclass lcls2 = (*(jni_data->env))->FindClass( jni_data->env,
                                                  ACTION_STAT_CLASS_NAME
                                                 );
    jni_data->action_stat_class = (*(jni_data->env))->NewGlobalRef( jni_data->env, lcls2 );
    if ( jni_data->action_stat_class == NULL ) {
        checkException( jni_data->env );
    }
    (*(jni_data->env))->DeleteLocalRef( jni_data->env, lcls2 );
   
    /* create and cache global to string class XXX Is this needed? XXX */
    jclass lcls3 = (*(jni_data->env))->FindClass( jni_data->env,
                                                  STRING_CLASS_NAME );
    jni_data->string_class = (*(jni_data->env))->NewGlobalRef( jni_data->env, lcls3 );
    if ( jni_data->string_class == NULL ) {
        checkException( jni_data->env );
    }
    (*(jni_data->env))->DeleteLocalRef( jni_data->env, lcls3 );

#ifdef _DEBUG
fprintf( stderr, "\n%s:%d::jni_data = %p jvm = %p env = %p rights_class = %p rights_obj = %p action_stat = %p, string class = %p", 
      __FILE__, __LINE__, jni_data, jni_data->jvm, jni_data->env,
      jni_data->rights_class, jni_data->rights_mgr, jni_data->action_stat_class, jni_data->string_class );
#endif /* _DEBUG */
    
    return jni_data;
}

/* 
 * loads jni env. should be called for every call from a new native thread 
 */
static int loadJNIEnv( JNIData *jni_data )
{
    jint ret;
    ret = (*(jni_data->jvm))->AttachCurrentThread( jni_data->jvm, 
                                                    (void **)&(jni_data->env), 
                                                    NULL ); 
    if ( ret < 0 ) {
        fprintf( stderr, "\nCan't get env from jvm" );
        jni_data->env = NULL;
    }

#ifdef _DEBUG
fprintf( stderr, "\n%s:%d::loadJNIEnv ret %d jvm %p", __FILE__, __LINE__, ret,
jni_data->env );
fflush(stderr);
#endif /* _DEBUG */
    
    return ret;
}

/*
 * client/driver is preparing to play media. This function will prepare keys.
 */
void jniPrepareKeys( void *xface_data, char * media_url, int action_id )
{
    JNIData *jni_data = (JNIData *)(xface_data);

    loadJNIEnv( jni_data );

    JNIEnv *env = jni_data->env;
    jobject rights_mgr = jni_data->rights_mgr; 
    

    jstring media_str = (*env)->NewStringUTF( env, (const char*) media_url );
    jmethodID mid = (*env)->GetMethodID( env, jni_data->rights_class,
                            PREPARE_KEYS, PREPARE_KEYS_SIG );

    if( mid == NULL ) {
        checkException( env );
        fprintf( stderr, "\nError getting method id for prepare keys");
        return;
    }
    
    (*env)->CallVoidMethod( env, rights_mgr, mid, (jint)action_id, media_str );

    if( checkException( env ) ) {
        fprintf( stderr, "Fatal exception jniCheckRights: PrepareKeys failed\n" );
        return;
    }
}


void jniReleaseRights( void *xface_data, char* media_url )
{
    unsigned char * keys;
    JNIData *jni_data = (JNIData *)(xface_data);

    loadJNIEnv( jni_data ); 
    
    JNIEnv *env = jni_data->env;
    jobject rights_mgr = jni_data->rights_mgr; 
    
    jstring media_str = (*env)->NewStringUTF( env, (const char*) media_url); 
    
    jmethodID mid = (*env)->GetMethodID( env, jni_data->rights_class, RELEASE_RIGHTS, RELEASE_RIGHTS_SIG );

    if ( mid == NULL ) {
        checkException( env );
        fprintf( stderr, "\ncreateNewJNIObject: no simple constructor found"); 
        return;
    }

    (*env)->CallVoidMethod( env, rights_mgr, mid, media_str );
    
    return;
}

/*
 * takes actionstat and converts it into a java object
 */
static jobject prepareActionStat( JNIData *jni_data, ActionStat *stat ) {
    
    /* Create an action stat object and populate it with various fields from the
     * native version of the action stat */
    jobject stat_obj = createNewJNIObject( jni_data->env,
                                              NULL, jni_data->action_stat_class );

    /* TODO set individual fields of the jActionStat. For now it does not really
     * matter. I am only setting the num field */
     jfieldID  fid = (*(jni_data->env))->GetFieldID(jni_data->env, jni_data->action_stat_class, "num", "I");
     
     (*(jni_data->env))->SetIntField(jni_data->env, stat_obj, fid, stat->cnt);

     return stat_obj;
}

/**
 * Reports usage 
 */
void jniReportUsage( void *xface_data, ActionId id, char *media_url )
{
    JNIData *jni_data = (JNIData *)(xface_data);   

    loadJNIEnv( jni_data );

    JNIEnv *env = jni_data->env;
    jobject rights_mgr = jni_data->rights_mgr;

    jstring media_str = (*env)->NewStringUTF( env, (const char*) media_url); 
    jmethodID mid = (*env)->GetMethodID( env, jni_data->rights_class, REPORT_USAGE, REPORT_USAGE_SIG );
    if ( mid == NULL ) {
        checkException( env );
        fprintf( stderr, "\njniReportUsage: no reportUsage method not found"); 
        return;
    }
    
    (*env)->CallVoidMethod( env, rights_mgr, mid, (jint)id, media_str );

    if( checkException( env ) ) {
        fprintf( stderr, "Fatal exception jniReportUsage: checkAction failed\n" );
    }

}

unsigned char jniCheckRights( void *xface_data, ActionId id, char * media_url, ActionStat *stat )
{
    unsigned char * keys;
    JNIData *jni_data = (JNIData *)(xface_data);

    loadJNIEnv( jni_data ); 
    
    JNIEnv *env = jni_data->env;
    jobject rights_mgr = jni_data->rights_mgr; 

    /* convert it into a java object */
    jobject stat_obj = prepareActionStat( jni_data, stat );

    jstring media_str = (*env)->NewStringUTF( env, (const char*) media_url); 
    
    jmethodID mid = (*env)->GetMethodID( env, jni_data->rights_class, CHECK_ACTION, CHECK_ACTION_SIG );

    if ( mid == NULL ) {
        checkException( env );
        fprintf( stderr, "\ncreateNewJNIObject: no simple constructor found"); 
        return 0;
    }

    jboolean retval = (*env)->CallBooleanMethod( env, rights_mgr, mid,
                                                  (jint)id, media_str, stat_obj );
    if( checkException( env ) ) {
        fprintf( stderr, "Fatal exception jniCheckRights: checkAction failed\n" );
        retval = 0;
    }

    /* if everything is done, lets be prudent and delete the weak global */
    (*env)->DeleteWeakGlobalRef( env, stat_obj );

    /* If there was no exception then we look for keys */
    /*
    jfieldID fid = env->GetFieldID( jni_data->action_stat_class, "keys", "[B" )
    jbyteArray * jba = (jbyteArray *)(*env)->GetObjectField( stat_obj, fid );
    
    jsize size = (*env)->GetArrayLength( env, jba ); 
    jbyte *bytes = (*env)->GetByteArrayElements( env, jba, NULL); 
    */
     
    return (unsigned char)retval;
}
