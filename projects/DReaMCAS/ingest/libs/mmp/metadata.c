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
 * $(@)metadata.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:39 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * =====================================================================================
 * 
 *        Filename:  metadata.c
 * 
 *     Description:  This has routines for metadata creation and manipulation	 
 * 
 *         Version:  1.0
 *         Created:  09/03/04 19:03:54 PDT
 *        Revision:  none
 *        Compiler:  cc
 * 
 *          Author:   (Deepak Chandra), 
 *         Company:  Sun Microsystems Inc.
 * 
 * =====================================================================================
 */




/*  Metadata file structure :

	Version no: 		1 bytes
	no_streams: 		1 bytes
 	meta_data_offset:	2 bytes  		

	for each stream:  {stream_metadata} 
	speed_of_stream: 4 bytes  :Just to be word aligned

*/

#include "metadata.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


/*
	This function creates the metadata file.
 
  */
metadata *init_metadata(char *meta_name, int speed){

	int ret_code;
	FILE *meta_file;
	metadata *ret;
	int err;
	stream_metadata *stream;
        metadata_header *header;	

	
	meta_file = fopen(meta_name,"wb");
	if (meta_file ==  NULL)
		FATAL_ERROR("cannot open the metadata file");

	header = (metadata_header *)malloc(sizeof(metadata_header));
	stream = (stream_metadata *)malloc(sizeof(stream_metadata));

	header->version = 	METADATA_VERSION;
	header->no_of_streams = 1;	  //this is the first stream
	header->metadata_offset = sizeof(*header) +
		                  1*sizeof(stream_metadata);
	stream->speed = speed;

	// write it to the file
	err = fwrite(header,1,sizeof(*header), meta_file);
	if (err != sizeof(*header))
		FATAL_ERROR("Unable to write to the file");
	err = fwrite(stream,1,sizeof(*stream),meta_file);				
	
	ret = (metadata *)malloc(sizeof(metadata));
	ret->header= header;
	ret->meta_file = meta_file;
	ret->stream_no  = 1; 
	ret->stream = stream;
	return ret;
}	

//
//metadata *open_metadata_file(char* metadata_file_name, int speed ){
//
//	FILE *file, *tmp_file;
//        metadata_header *header;
//   	int stream_no;
//	int no_of_streams;
//        stream_metadata *stream;
//	int i;
//	metadata *meta;	
//	//picture_metadata temp;
//        char *tmp_filename;
//
//	tmp_filename = malloc(L_tmpnam);	
// 	header = (metadata_header *)malloc(sizeof(metadata_header));			 
//	stream = (stream_metadata *)malloc(sizeof(stream_metadata));
//	meta = (metadata *)malloc(sizeof(metadata));
//	meta->header = header;
//	assert(metadata_file_name != NULL);	
//       //we will be writing a temp file and renaming that to this name
//	tmpnam(tmp_filename);
//	//sprintf(tmp_filename,"%s.tmp",metadata_file_name);
//	assert((file = fopen(metadata_file_name,"rb")) !=  NULL);
//	assert((tmp_file = fopen(tmp_filename,"wb")) != NULL);
//	meta->meta_file = file;	
//	meta->tmp_file = tmp_file;
//	meta->meta_filename = metadata_file_name;
//	meta->tmp_filename = tmp_filename;
//	assert(fread(header,1,sizeof(*header),file) != NULL);
//	meta->header = header;
//	
//
//	if(header->version != METADATA_VERSION)	
//		FATAL_ERROR("metadata version not supported");
//	
//	for(i=0, stream_no = -1;i<header->no_of_streams; i++){
//		stream = malloc(sizeof(stream_metadata));
//    		assert(fread(stream,1,sizeof(*stream),file) != NULL);
//		meta->stream[i] = stream;
//	    	if(stream->speed == speed)
//			stream_no = i;
//	}
//	if (stream_no == -1){
//		stream_no = header->no_of_streams++;
//		stream = malloc(sizeof(stream_metadata));
//		meta->stream[stream_no] = stream;
//		stream->speed = speed;
//	}
//	meta->stream_no = stream_no;
//
//	header->metadata_offset = sizeof(*header) +
//		                  header->no_of_streams*sizeof(stream_metadata);
//
//
//
//       // writing the new information into the tmp_file
////	fwrite(meta->header,1,sizeof(metadata_header),tmp_file);
////	for(i=0;i<header->no_of_streams; i++)
////		fwrite(meta->stream[i],1,sizeof(stream_metadata), tmp_file);
//	
//	
//	return meta;
//}

// int  read_frame(metadata *meta, picFrame_metadata *frame){
//
//	FileOffset pos;
//	int no_of_streams;
//	int err;
//
//	assert(meta != NULL);
//	assert(meta->header != NULL);
// 	//read one less-- exclude the new stream 
//	no_of_streams = meta->header->no_of_streams -1;
//	
//      //read the frame number and type info before reading anything else 
//	err = fread(&(frame->frame_number),1,
//		      sizeof(frame->frame_number), meta->meta_file);
//	assert(err ==  sizeof(frame->frame_number));
//	err = fread(&(frame->frametype),1,
//		      sizeof(frame->frametype), meta->meta_file);
//	assert(err ==  sizeof(frame->frametype));
//	
//	err = fread(frame->offsets,1,
//		no_of_streams*sizeof(FileOffset),
//		 meta->meta_file);
//	return err;
//
//}

/*void meta_cleanup(metadata *meta){

	int i, no_of_streams;
	
	assert(meta != NULL);
	assert(meta->header != NULL);
	printf("here \n");
	free(meta->stream);
	fclose(meta->meta_file);
	free(meta);
}
*/