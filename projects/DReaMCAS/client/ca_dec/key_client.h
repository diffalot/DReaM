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
 * $(@)key_client.h $Revision: 1.1.1.1 $ $Date: 2006/03/15 13:12:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef KEY_CLIENT_H
#define KEY_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <curl/curl.h>

#ifdef WIN32

#include <iostream.h>
#include <winsock2.h>
#include <windows.h>
#define close closesocket
#define sleep Sleep

#else

#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#endif
//#define TRACE_MSG

#define LICENSE_SERVER_PORT 8105   
#define HEADER_NAME_SIZE 32
#define HEADER_FIELD_SIZE 2048
#define ENTITY_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 512

#define URL_SIZE 4000
#define COOKIE_SIZE 1024

typedef struct http_field
{
    char field_name [HEADER_NAME_SIZE];
    char field_value[HEADER_FIELD_SIZE];
    struct http_field *next_field;
}http_field;

typedef struct 
{
    http_field *field_list;
    int        entity_length;
    char       *entity_data;
	int        entity_buffer_len;
}http_response_s;


#ifdef __cplusplus
extern "C" {
#endif

char *GetLicense(char* URI, int port, char* file, int *license_size);
int SendRequest(int server_sock, char *command);
int ConnectTo(char* URI, int port);
int GetHTTPResponse(int server_sock, http_response_s *response);

#ifdef __cplusplus
}
#endif

#endif
