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
 * $(@)key_client.c $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "key_client.h"

#define MAX_REFRESH_TRIES 10
#define SLEEP_BEFORE_REFRESH 2

#if 0
int parse_redirect_request_for_URL(char* server_name, char *server_port, char* parameters, char* server_response_msg);
void make_GET_command(char *request_msg, char *parameters);

http_field *CreateHTTPField(char *fieldname)
{
    http_field *elem = (http_field *)calloc(1, sizeof(http_field));
    if (fieldname)
    {
        strcpy(elem->field_name, fieldname);
    }
    return elem;
}

http_field * AppendHTTPField(http_field *list, http_field *elem)
{
  if( list == NULL )
    return(elem);

  list->next_field = AppendHTTPField(list->next_field,elem);
  return(list);
}

void DeleteHTTPFieldList(http_field *list)
{
	if (list->next_field == NULL)
	{
		free(list);
		return;
	}
    DeleteHTTPFieldList(list->next_field);
    return;
}
	
http_field *SearchForField(http_field *list, char *field_name)
{
    if (field_name == NULL)
    {
        return NULL;
    }
    else
    {
        http_field *elem = list;
        while (elem)
        {
            if (strcmp(field_name, elem->field_name) == 0)
            {
                break;
            }
            elem = elem->next_field;
        }
        return elem;
    }
}


// The ConnectTo method connects to the key server
// running at location URI. The URI is communicated
// to the player by means of the SDP.

int ConnectTo(char* URI, int port)
{
	int connect_result;
	struct sockaddr_in server_addr; 
	struct hostent *he;
	int sockfd;

    if ((he=gethostbyname(URI)) == NULL) { 
        #ifdef TRACE_MSG
         fprintf(stderr,"Connect to returned [hostname] %d\n",-1);
        #endif

        return(-1);
    }


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        #ifdef TRACE_MSG
         fprintf(stderr,"Connect to  returned [socket]%d\n",-1);
        #endif

        return(-1);
    }

	server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), '\0', 8); 

    connect_result = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

   if (connect_result == -1) {
    perror("Cannot establish a server connection");
    return(-1);
   }

    #ifdef TRACE_MSG
     fprintf(stderr,"Connected to %s:%d\n",URI,port);
    #endif

    return(sockfd);
}

int GetHTTPResponse(int server_sock, http_response_s *response)
{
    char data[RESPONSE_BUFFER_SIZE+1]  ={'\0'};
    http_field *elem = NULL;
    enum {
    EXPECTING_FIELD_NAME,
    EXPECTING_FIELD_VALUE,
    EXPECTING_ENTITY_BODY,
    }expecting = EXPECTING_FIELD_NAME;
    
    int field_value_index = 0;
    int field_name_index = 0;
    int index = 0, total_bytes = 0;
    
    int numbytes = recv(server_sock, data, RESPONSE_BUFFER_SIZE, 0);
    
	total_bytes = numbytes;
    if (response->field_list)
    {
        DeleteHTTPFieldList(response->field_list);
        response->field_list = NULL;
    }
    if (response->entity_data)
    {
        free(response->entity_data);
        response->entity_data = NULL;
        
    }
    
    response->entity_length = 0;
    if (numbytes <= 0)
        return -1;

    response->entity_data = (char *)malloc(ENTITY_BUFFER_SIZE);
	if (response->entity_data == NULL)
	{
		return -1;
	}
    response->entity_buffer_len = ENTITY_BUFFER_SIZE;

    if (data[0] == 'H' && data[1] == 'T' && data[2] == 'T' &&
     data[3] == 'P' && data[4] == '/')  
    {
        /* full response */
        /* retrieve the status line first */
        elem = CreateHTTPField("StatusLine");
        expecting = EXPECTING_FIELD_VALUE;
    }
    else
    {
        /* simple response, only entity data */
        expecting = EXPECTING_ENTITY_BODY;
    }
    
#ifdef TRACE_MSG    
	    printf("Got response :\n");
#endif

    do
    {
        total_bytes += numbytes;
#ifdef TRACE_MSG  
	        printf("%s",data);
#endif
        while (index < numbytes)
        {
            switch (expecting)
            {
                case EXPECTING_FIELD_NAME:
                    elem->field_name[field_name_index++] = data[index++];
                    break;
                case EXPECTING_ENTITY_BODY:
					if (response->entity_buffer_len <= response->entity_length)
					{
	                    response->entity_data = (char *)realloc(response->entity_data, 
							                     response->entity_buffer_len + ENTITY_BUFFER_SIZE);
                        response->entity_buffer_len += ENTITY_BUFFER_SIZE;
					}

                    response->entity_data[response->entity_length ++] = data[index++];
                    continue;
                    break;
                case EXPECTING_FIELD_VALUE:
                    elem->field_value[field_value_index++] = data[index++];
                    break;
                default:
                     break;
            }
            /* check the next token */
            if (data[index] == ':' && expecting == EXPECTING_FIELD_NAME)
            {
                expecting = EXPECTING_FIELD_VALUE;
                elem->field_name[field_name_index] = '\0';
                field_name_index = 0;
                index ++; //skip ':'
            }
            else if (data[index] == '\r')
            {
                if (expecting == EXPECTING_FIELD_VALUE)
                {
                    elem->field_value[field_value_index] = '\0';
                    field_value_index = 0;
                    expecting = EXPECTING_FIELD_NAME;
                    response->field_list = AppendHTTPField(response->field_list, elem);
                    elem = CreateHTTPField(NULL);
                }
                
                index += 2; //skip CR LF
                if (data[index] == '\r')
                {
                    index += 2; //skip CR LF
                    expecting = EXPECTING_ENTITY_BODY;
                    if (elem)
                    {
                        DeleteHTTPFieldList(elem);
                    }
                }
            }
        }
        index = 0;
		sleep(5);
		numbytes = recv(server_sock, data, RESPONSE_BUFFER_SIZE, 0);
        
    } while (numbytes);
    
#ifdef TRACE_MSG    
	    printf("\n");
#endif
            /* simple response, only the entity */
    return(total_bytes);
}

void make_GET_command(char *request_msg, char *parameters)
{
    request_msg[0] = '\0';
    strcpy(request_msg,"GET ");
    strcat(request_msg,parameters);
    strcat(request_msg," HTTP/1.0");
    strcat(request_msg,"\n\n"); 
}

int SendRequest(int server_sock, char *command)
{
    int numbytes = 0;

#ifdef TRACE_MSG    
	    printf("Send request (len=%d)\n%s",strlen(command), command);
#endif

    numbytes = send(server_sock,command ,strlen(command),0);
	return(numbytes);
}

#endif

typedef struct ParseRet {
    char * contentBody;
    int    contentBodyLen;
} ParseRet;

int parse_redirect_request_for_URL(char* server_name, char *server_port, char* parameters, char* server_response_msg)
{

	char *ptr;
	int index= 0;
	int i=0;
    parameters[0]='\0';
	ptr = strstr(server_response_msg,"http://");
	if (ptr == NULL)
	{   
		fprintf(stderr,"URL does not contain http");
		return -1;
	}
	
	ptr += strlen("http://");
	index = 0;

    while (ptr[index] != ':' && ptr[index] != '/')
    {
		server_name[index] = ptr[index];
		index++;
    }
	server_name[index] = '\0';
	
	i = 0;
	server_port[0] = '8', server_port[1] = '0', server_port[2] = '\0';
	if (ptr[index] == ':')
	{
		index++; //skip ":"

		while (ptr[index] != '/')
		{ 
			server_port[i] = ptr[index];
			i++; 
			index++;
		}   
		server_port[i] = '\0';
	}
	
	i = 0;
	while (ptr[index] != '\0' && ptr[index] != '\n' && ptr[index] != '\r')
	{
		parameters[i] = ptr[index];
		index++;
		i++;
	}
	parameters[i] = '\0';

#ifdef TRACE_MSG    
    fprintf(stderr,"Redirected URL %s : %s and parameters are %s\n",server_name,server_port,parameters);
#endif
	
	return(1);

}

int myitoa(int port, char * portStr)
{
    return sprintf(portStr, "%d", port);
}

size_t parseContentBody(void *data, size_t size, size_t nmemb,
                            void * ret)
{
    int remLen = size * nmemb;
    ParseRet * parseRet = (ParseRet *) ret;
    parseRet->contentBody = (char *)realloc( parseRet->contentBody,
                    size * nmemb + parseRet->contentBodyLen );
    memcpy(parseRet->contentBody + parseRet->contentBodyLen, data, remLen);
    parseRet->contentBodyLen += size * nmemb;

    fprintf(stderr, "DEBUG: Got data %d", parseRet->contentBodyLen);
    return size * nmemb; 
}

char *GetLicense(char * URI, int port, char * file, int * license_size )
{
    char *retVal = NULL; 
    char url[URL_SIZE];
    char *tempUrl = NULL;
    char serverName[256]; 
    char portStr[20];
    char parameters[URL_SIZE];

    
    int tryAgain = MAX_REFRESH_TRIES;
    CURLcode curlCode = 0;
    
    /* init curl */
    CURL * curl = curl_easy_init();
    
    ParseRet *parseRet = (ParseRet *)malloc(sizeof(ParseRet));
    parseRet->contentBody = NULL;
    parseRet->contentBodyLen = 0;
    
    myitoa(port, portStr);

    *license_size = 0;
    
    /* step 1 connect to license server keep following redirectionsc till we get
     * a license */ 
    strcpy(url, URI);
    strcat(url, ":");
    strcat(url, portStr);
    strcat(url, file); /* file is really url parameters */ 
   
    /* maybe this should be in an if condition checking for verbosity */ 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_PORT, port);
   
    /* curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1); */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
   
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parseContentBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, parseRet);
    
    curlCode = curl_easy_perform(curl);
 
    /* Ping the operaServer with a refresh until we get a license 
       or until timeout */
    do {
        tryAgain --; 
        if ( curlCode !=0 ) {
            goto curlError;
        }
        
        /* got license? */
        if ( parseRet->contentBodyLen > 0 ) 
        {
            fprintf(stdout, "Got data - Probably license. size: %d", parseRet->contentBodyLen);
            *license_size = parseRet->contentBodyLen; 
            retVal = parseRet->contentBody;
            
            goto curlDone;

        } else { /* no license, send refresh command */
            
            sleep( SLEEP_BEFORE_REFRESH );
             
            curlCode = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &tempUrl); 
            
            if ( curlCode != 0 ) {
                goto curlError;
            }
            
            parse_redirect_request_for_URL( serverName, portStr, parameters,
                                             tempUrl);

            /* create the refresh url */
            strcpy(url, "http://");
            strcat(url, serverName);
            strcat(url, ":");
            strcat(url, portStr);
            strcat(url, "/opera/operaserver?action=refresh");

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curlCode = curl_easy_perform(curl); 
        }
    } while ( tryAgain );
    
    if( !tryAgain && retVal != NULL ) {
        fprintf(stderr, "Opera Server not responding with license. Retries maxed!");
    }
    
curlDone:
curlError:
    curl_easy_cleanup(curl);
    fprintf( stderr, curl_easy_strerror(curlCode) );
    return retVal; 
}
