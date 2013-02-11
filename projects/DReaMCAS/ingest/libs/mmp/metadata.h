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
 * $(@)metadata.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * =====================================================================================
 * 
 *        Filename:  metadata.h
 * 
 *     Description:  header files for metadata
 * 
 *         Version:  1.0
 *         Created:  09/03/04 19:17:23 PDT
 *        Revision:  none
 *        Compiler:  cc
 * 
 *          Author:   (Deepak Chandra)
 *         Company:  Sun Microsystem Inc.
 *           Email:  deepak.chandra@sun.com
 * 
 * =====================================================================================
 */
#include <stdio.h>

#define METADATA_VERSION	1
#define MAX_NO_STREAMS		6
#define ERROR_PREFIX fprintf(stderr,"\nERROR in %s line %d:\n",__FILE__,__LINE__);
#define FATAL_ERROR(a) {ERROR_PREFIX;fprintf(stderr,a);exit(-1);}



typedef long long FileOffset; 

typedef struct  {
	int version;
 	int no_of_streams;	
	long metadata_offset;
} metadata_header;


typedef struct {
	int speed;
	long frame_length;
} stream_metadata;	

typedef struct {

	metadata_header *header;
	int stream_no;
	int speed;
	stream_metadata *stream;
	FILE *meta_file;
	FILE *tmp_file;
	char *tmp_filename;
	char *meta_filename;
} metadata;

typedef struct{

	long frame_number;
	int frametype;
	FileOffset offset;
	
} picture_metadata; 




metadata *open_metadata_file(char* metadata_file_name, int speed );

long calc_position_to_write(metadata *meta, long frame_number);

