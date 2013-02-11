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
 * $(@)conv_meta_bin_ascii.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * =====================================================================================
 * 
 *        Filename:  conv_meta_bin_ascii.c
 * 
 *     Description:  utility to convert binary file metadata files to ascii
		     For debugging purposes.
 * 
 *         Version:  1.0
 *         Created:  09/09/04 19:05:39 PDT
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:   (Deepak Chandra), 
 *         Company:  
 * 
 * =====================================================================================
 */

#include "metadata.h"
#include <assert.h>
#include <stdio.h>


void print_header(metadata_header *head, FILE* f){

	fprintf(f,"Version :%d\n", head->version);
	fprintf(f, "No of streams %d\n", head->no_of_streams);
	fprintf(f, "starting of metadata %ld\n", head->metadata_offset);
	fprintf(f, "-----------------------------------------------\n");
	fprintf(f , "-----------------------------------------------\n");
}


void print_stream_info(stream_metadata stream[], int stream_no, FILE *f){

	int i;
	fprintf(f, "streams and speeds\n");
	fprintf(f,"frame_no\tframetype\t");
	for(i =0; i< stream_no; i++)
		fprintf(f, "\t%d\t", stream[i].speed);

	fprintf(f,"\n");

}


void print_frame(picFrame_metadata *picFrame, int stream_no, FILE *f){

	
	int i;
	fprintf(f, "%5ld\t\t%u\t\t", picFrame->frame_number, picFrame->frametype);
	for(i =0; i< stream_no; i++)
		fprintf(f, "%6lld\t", picFrame->offsets[i]);

	fprintf(f, "\n");
}

void usage(char *progName){

	printf("usage: %s  binary_metafile  ascii_metafile",progName );

}
void main(int argc, char** argv) {

	metadata meta;
	metadata_header header;
	stream_metadata stream[MAX_NO_STREAMS];
	FileOffset  frame[MAX_NO_STREAMS];
	const char* bin_metafile, *ascii_metafile;
	FILE *bin_file,*asc_file;
	long frame_no;
	picFrame_metadata picFrame;

	if(argc != 3) usage(argv[0]);
	bin_metafile = argv[1];
	ascii_metafile = argv[2];

	assert(bin_file = fopen(bin_metafile,"rb"));
	assert(asc_file = fopen(ascii_metafile, "w"));

	//reading header 
	fread(&header,1,sizeof(metadata_header),bin_file);	
	print_header(&header, asc_file);
	meta.header = &header;
	//reading stream info 
	fread(&stream,1, sizeof(stream_metadata)*header.no_of_streams,bin_file);
	print_stream_info(stream,header.no_of_streams, asc_file);
	fseek(bin_file, header.metadata_offset, SEEK_SET);
	while(!feof(bin_file)){
		fread(&(picFrame.frame_number), 1, sizeof(long), bin_file);
		fread(&(picFrame.frametype),1,sizeof(int), bin_file);
		fread(picFrame.offsets,sizeof(FileOffset), header.no_of_streams,bin_file);
	 	print_frame(&picFrame, header.no_of_streams,asc_file);
	}

	fclose(bin_file);
	fclose(asc_file);
	
}


