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
 * $(@)config.c $Revision: 1.2 $ $Date: 2006/07/25 18:38:20 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "key_client.h"
#include "ca_dec.h"
/*
Temporay file for reading the configuration file
*/
#define MAX_ITEMS_TO_PARSE 10

char *GetConfigFileContent (char *Filename)
{
    long FileSize;
    FILE *file;
    char *buffer;

    if (NULL == (file = fopen (Filename, "r")))
    {
        printf ("Cannot open configuration file %s.\n", Filename);
        return NULL;
    }

    if (0 != fseek (file, 0, SEEK_END))
    {
        printf ("Cannot fseek in configuration file %s.\n", Filename);
        return NULL;
    }

    FileSize = ftell (file);
    if (0 != fseek (file, 0, SEEK_SET))
    {
        printf ("Cannot fseek in configuration file %s.\n", Filename);
        return NULL;
    }

    if ((buffer = malloc (FileSize + 1))==NULL)
    {
        printf("Memory allocation failed: GetConfigFileContent\n");
    }

    FileSize = fread (buffer, 1, FileSize, file);
    buffer[FileSize] = '\0';

    fclose (file);
    return buffer;
}


int ParseContent (char *buf, int bufsize, char *items[MAX_ITEMS_TO_PARSE])
{
    int item = 0;
    int beginString = 1;
    char *p = buf;
    char *bufend = &buf[bufsize];

    while (p < bufend)
    {
        switch (*p)
        {
            case 13:
                p++;
                break;
            case '#':                 // comment
                *p = '\0';              
                while (*p != '\n' && p < bufend)  
                  p++;
                beginString = 1;
                break;
            case '\n':
                beginString = 1;
                *p++='\0';
                break;

            default:
                if (beginString)
                {
                    items[item++] = p;
                    beginString = !beginString;
                }
                p++;
        }
    }
    
    return item;
}

char *GetFieldValue(char *field, char *items[MAX_ITEMS_TO_PARSE], int item_number)
{
	int i = 0;
	char *tmpstr = NULL; 
	for (i = 0; i < item_number; i++)
	{
        if (strstr(items[i], field) != NULL)
        {
        	tmpstr = strchr(items[i],'=');
			tmpstr ++;
        	while( *tmpstr == ' ' || *tmpstr == '\t')
        		tmpstr ++;
        	break;
        }
	}
	return tmpstr ;
}

char *GetConfigurationValue(char *configuration_name, char *field)
{
    char *content;
    char *value = NULL;
    content = GetConfigFileContent(configuration_name);
    if (content)
    {
    	char *items[MAX_ITEMS_TO_PARSE];
    	char *result;
    	int item_no = 0;
    	item_no = ParseContent (content, strlen(content), items);
    	result = GetFieldValue(field, items, item_no);
    	if (result != NULL)
    	{
    	    value = strdup(result);
    	}
    }

    return value; 

}

void ProcessRTSPString(char *target)
{
	const char *tmpstr = NULL; 

     tmpstr = strchr(target,'&');
     if(tmpstr != NULL)
     {
     	if (strstr(target,"cas=1") != NULL)
     	{
     		// we need a license
	     	char *lic_server = GetConfigurationValue(CONFIG_FILE_NAME, "LicenseServer");
	     	char *port = GetConfigurationValue(CONFIG_FILE_NAME, "PortNumber");
	     	char *action = GetConfigurationValue(CONFIG_FILE_NAME, "Action");
	     	char *parameters = NULL;
	     	int len = 0;
	     	
	     	if (lic_server && port)
	     	{
	     		int license_size = 0;
		     	if (action)
		     	{
		            len = strlen(action) + strlen(tmpstr);
		            parameters = malloc(len + 1);
		            parameters[0] = '\0';
		            strcat(parameters, action);
		            strcat(parameters, tmpstr); 
		     	}
		     	else
		     	{
		     		len = strlen(tmpstr);
		     		parameters = malloc(len + 1);
		     		parameters[0] = '\0';
		     		strcat(parameters, tmpstr); 
		     	}
	     		char *file_name = GetConfigurationValue(CONFIG_FILE_NAME, "LicenseName");
	     		if (file_name)
	     		{
			     	printf("Requesting a license: %s:%s%s\n",lic_server, port, parameters);
	     		    char *license = GetLicense(lic_server, atoi(port), parameters, &license_size);
                    fprintf(stderr, "Got license in config.c %s %p %d",
                    file_name, license, license_size); fflush(stderr);
	     		    if (license)
	     		    {
		     		    FILE *file = fopen(file_name, "wb");
                        int ret = 0;
                        if ( !file ) {
                            perror("Cannot open local file for storing license");
                            free(license);
                            return;
                        }
		     		    ret = fwrite(license, 1, license_size, file);
		     		    fprintf(stderr,"Get a license from the server (size: %d) ret %d\n",license_size, ret );
                        fflush(stderr);
		     		    fclose(file);
		     		    free(license);
	     		    }
	     		    else
	     		    {
	     		    	fprintf(stderr,"Couldn't get a license\n");
	     		    }
		     		    
	     		}
	     		if (file_name)
		     		free(file_name);
	     		
	     	}
	     	
	     	if (lic_server)
	     	    free(lic_server);
	     	if (port)
	     	    free(port);
	     	if (action)
	     	    free(action);
	     	if (parameters)
	     		free(parameters);
	     }

         target[tmpstr-target] = '\0';
     }
     return;
   
}
void vlcGetUserInfo( int i_argc, char *ppsz_argv[] )
{
    int i = 0;
    for (i = 1; i < i_argc; i++)
    {
        if (strstr(ppsz_argv[i], "rtsp:") != NULL ||
        	strstr(ppsz_argv[i], "RTSP:") != NULL  )
        {
        	ProcessRTSPString(ppsz_argv[i]);
     		break;
        }
    }
    return;
}

#if 0
int main(int argc, char **argv)
{
	vlcGetUserInfo( argc, argv );
    
    return 0;
}
#endif

