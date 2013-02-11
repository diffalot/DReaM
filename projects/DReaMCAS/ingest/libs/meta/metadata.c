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
 * $(@)metadata.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:36 $
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
 *            Bugs:  Known Bug: For a particular stream speed whose metadata is
 *            already in the metadata file, a repeat mux for a stream of that
 *            speed causes meta file corruption.
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
#include <limits.h>
#include <string.h>
#include <stdio.h>

//void initialize_stream(metadata *meta);
/*
	This function creates the metadata file.
 
  */


void initialize_stream(metadata *meta);
int  read_frame(metadata *meta, picFrame_metadata *frame);


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
	
	ret = (metadata*)malloc(sizeof(metadata));
	ret->header= header;
	ret->meta_file = meta_file;
	ret->stream_no  = 1; 
	ret->stream[0] = stream;
	return ret;
}	


metadata *open_metadata_file(char* metadata_file_name ){

	FILE *file;
        metadata_header *header;
   	int stream_no;
	int no_of_streams;
        stream_metadata *stream;
	int i;
	metadata *meta;	


	header = (metadata_header *)malloc(sizeof(metadata_header));			 
	stream = (stream_metadata *)malloc(sizeof(stream_metadata));
	meta = (metadata *)malloc(sizeof(metadata));
	meta->header = header;
	assert(metadata_file_name != NULL);	
        printf("%s",metadata_file_name);
        fflush(stdout);
	assert((file = fopen(metadata_file_name,"rb")) !=  NULL);
	meta->meta_file = file;	
	meta->meta_filename = metadata_file_name;


	assert(fread(header,1,sizeof(*header),file) != NULL);
	meta->header = header;
	

	if(header->version != METADATA_VERSION)	
		FATAL_ERROR("metadata version not supported");
	
	for(i=0;i<header->no_of_streams; i++){
		stream = (stream_metadata*)malloc(sizeof(stream_metadata));
    		assert(fread(stream,1,sizeof(*stream),file) != NULL);
		meta->stream[i] = stream;
	}

	meta->stream_no = -1; //no curr stream
	meta->speed = -1;



	
	
	return meta;
}


int open_temp_metafile(metadata *meta, int speed) {

        char *tmp_filename;
	FILE  *tmp_file;
	int i;
	metadata_header *header;
	int stream_no;
	stream_metadata *stream;
	
	assert(meta != NULL);
	assert(meta->header != NULL);
	assert(meta->stream != NULL);
	tmp_filename = (char*)malloc(strlen(meta->meta_filename)+10);	
	sprintf(tmp_filename,"%s.tmp",meta->meta_filename);
	//tmpnam(tmp_filename);
	assert((tmp_file = fopen(tmp_filename,"wb")) != NULL);
	meta->tmp_file = tmp_file;
	meta->tmp_filename = tmp_filename;
	meta->total_frames = 0;   //to keep track of already seen frames
	
	header = meta->header;
	for(i=0, stream_no = -1;i<header->no_of_streams; i++){
	    	if(meta->stream[i]->speed == speed)
			stream_no = i;
	}

	if (stream_no == -1){
		stream_no = header->no_of_streams++;
		stream = (stream_metadata*)malloc(sizeof(stream_metadata));
		meta->stream[stream_no] = stream;
		stream->speed= speed;
		initialize_stream(meta);

	} else {
        // FIXME
        fprintf(stderr,"\nStream already exists. Cannot handle condition FIXME");
        // exit(-1);
        // return 0;
		header->metadata_offset = sizeof(*header) +
		                  header->no_of_streams*sizeof(stream_metadata);
		//writing to tmp file 

		fwrite(meta->header,1,sizeof(metadata_header),tmp_file);
		for(i=0;i<header->no_of_streams; i++)
			fwrite(meta->stream[i],1,sizeof(stream_metadata), tmp_file);
	}


	meta->speed = speed;
	meta->stream_no = stream_no;

    return 0;
}



void initialize_stream(metadata *meta){
	int i,err;	
	FILE *prev;
    FILE *newfile;	
	picFrame_metadata frame;
	metadata_header *header;

	prev = meta->meta_file;
	newfile = meta->tmp_file;
	header = meta->header;


	header->metadata_offset = sizeof(*header) +
		                  header->no_of_streams*sizeof(stream_metadata);
	
	//write the header into the new file
	fwrite(meta->header,1,sizeof(metadata_header),newfile);
	for(i=0;i<header->no_of_streams; i++)
		fwrite(meta->stream[i],1,sizeof(stream_metadata), newfile);
	fseek(newfile,header->metadata_offset,SEEK_SET);	
	fseek(prev,header->metadata_offset-sizeof(stream_metadata), SEEK_SET);  //fix this. Have an old metadata saved
	//read data from the old file and write it to the new one and fill -1 for the new stream
	while(!feof(prev)){
		err = read_frame(meta, &frame);			
		if(err == 0) break;
		frame.offsets[header->no_of_streams-1] = -1;
		fwrite(&(frame.frame_number),1,sizeof(frame.frame_number),newfile);
		fwrite(&(frame.frametype),1,sizeof(frame.frametype),newfile);
		fwrite(frame.offsets,1,sizeof(FileOffset)*header->no_of_streams,newfile);
	}
	return ;
}

int  read_frame(metadata *meta, picFrame_metadata *frame){

	FileOffset pos;
	int no_of_streams;
	int err;

	assert(meta != NULL);
	assert(meta->header != NULL);
 	//read one less-- exclude the new stream 
	no_of_streams = meta->header->no_of_streams -1;
	
      //read the frame number and type info before reading anything else 
	err = fread(&(frame->frame_number),1,
		      sizeof(frame->frame_number), meta->meta_file);
	if(feof(meta->meta_file)) return 0;
	assert(err =  sizeof(frame->frame_number));
	err = fread(&(frame->frametype),1,
		      sizeof(frame->frametype), meta->meta_file);
	assert(err ==  sizeof(frame->frametype));
	
	err = fread(frame->offsets,1,
		no_of_streams*sizeof(FileOffset),
		 meta->meta_file);
	return err;

}

void meta_cleanup(metadata *meta){

	int i, no_of_streams;
	
	assert(meta != NULL);
	assert(meta->header != NULL);
	no_of_streams = meta->header->no_of_streams;
	for(i=0;i<no_of_streams;i++)	
		free(meta->stream[i]);
	fclose(meta->meta_file);
	fclose(meta->tmp_file);

	//printf("%s\n",meta->tmp_filename);
	
	remove(meta->meta_filename);
	i= rename(meta->tmp_filename, meta->meta_filename);
//	free(meta->tmp_filename);	
//	free(meta->meta_filename);
	free(meta);
}

FileOffset get_pos( metadata *meta, picFrame_metadata * frame, int speed )
{
    int cur_stream_no; 
    FileOffset switch_stream_offset;
    stream_metadata *cur_stream;

    //////////// XXX Since we only support switch speeds of 12 change switch
    //////////// speed greater than 1 to 12
    if ( speed > 1 )
        speed = 12;
    if ( speed < 1 )
        speed = -12;
    ////////////

	assert((cur_stream = get_stream_metadata(meta, speed, &cur_stream_no)) != NULL);
    switch_stream_offset = frame->offsets[cur_stream_no];

#ifdef PRN_DEBUG
    printf("File position is %lld\n", frame->offsets[cur_stream_no]);
    fflush(stdout);
#endif // PRN_DEBUG

    // 23 = number of bytes to back up to get to sequence header. 
    switch_stream_offset -= 23;
    // back up to beginning of the transport stream packet. 
    switch_stream_offset = switch_stream_offset - 
                                  ( switch_stream_offset % 188 );

    
    return switch_stream_offset; 
}

//FileOffset get_switch_offset(metadata *meta,int cur_speed, FileOffset cur_offset,
picFrame_metadata * get_switch_frame(metadata *meta,int cur_speed, FileOffset
cur_offset, int switch_speed, int direction, int curr_direction,
int frame_type){

	long pic_frame_no;
	int size_of_frame, no_of_streams;
	stream_metadata *cur_stream,*switch_stream;
	FileOffset switch_stream_offset;
	long offset;
	picFrame_metadata frame, *ret_frame;
	int cur_stream_no, switch_stream_no;
    	
#ifdef PRN_DEBUG
    printf("\nswitch_stream arguments: meta %x, cur_speed %d, cur_offset %lld, switch_speed %d, direction %d", 
                                       meta, cur_speed, cur_offset, switch_speed, direction);
#endif //PRN_DEBUG

    //////////// XXX Since we only support switch speeds of 12 change switch
    //////////// speed greater than 1 to 12
    if ( switch_speed > 1 )
        switch_speed = 12;
    if ( cur_speed > 1 )
        cur_speed = 12;    
    if ( switch_speed < 1 )
        switch_speed = -12;
    if ( cur_speed < 1 )
        cur_speed = -12;
    ////////////

	//assert(meta != NULL);
    if ( meta == NULL ) {
        fprintf(stderr, "metadata structure is null. Cannot get switch frame!\n");
        return NULL;
    }
        
	//assert(cur_offset < LONG_MAX);
    if ( cur_offset > LONG_MAX ) {
        fprintf( stderr, "Invalid value for curr_offset. Cannot get switch frame\n");
        return NULL;
    }
    
	//assert((cur_stream = get_stream_metadata(meta, cur_speed, &cur_stream_no)) != NULL);	
    cur_stream = get_stream_metadata( meta, cur_speed, &cur_stream_no );
    if ( cur_stream == NULL )  {
        fprintf( stderr, "Cannot get stream metadata for speed = %d. \n", cur_speed);
        return NULL;
    }
    
	// assert((switch_stream = get_stream_metadata(meta, switch_speed, &switch_stream_no)) != NULL);
    switch_stream = get_stream_metadata( meta, switch_speed, &switch_stream_no );
    if ( switch_stream == NULL )  {
        fprintf( stderr, "Cannot get stream metadata for speed = %d. \n", switch_speed);
        return NULL;
    }

	pic_frame_no = cur_offset/cur_stream->avg_frame_length;

	
	no_of_streams = meta->header->no_of_streams;
	size_of_frame = sizeof(frame.frame_number) + sizeof(frame.frametype) +
			 no_of_streams*sizeof(FileOffset);	


	offset = meta->header->metadata_offset + (pic_frame_no-1)*size_of_frame;
    // XXX Hack for finding the correct frame number when offset is in the
    // reverse direction. Currently the frame number returned for DIR_REV is
    // pic_frame_no when it should be pic_frame_no frames from the last since
    // the offset values are in decreasing order of value for the reverse
    // column. This needs to be fixed properly.
    if ( curr_direction == DIR_REV ) {
        fseek(meta->meta_file, -1 * offset + 4, SEEK_END);    
    } else {
	    fseek(meta->meta_file, offset, SEEK_SET);	
    }
    // XXX

//	switch_stream_offset = search_offset(meta,cur_stream_no, switch_stream_no,
//    cur_offset,meta->header->no_of_streams, direction, curr_direction, frame_type);
           
	ret_frame = search_frame(meta,cur_stream_no, switch_stream_no,
    cur_offset,meta->header->no_of_streams, direction, curr_direction, frame_type);
           
    // 23 = number of bytes to back up to get to sequence header. 
//    switch_stream_offset -= 23;
    // back up to beginning of the transport stream packet. 
//    switch_stream_offset = switch_stream_offset - 
//                                  ( switch_stream_offset % 188 );
    
//	return switch_stream_offset;
    return ret_frame;
}

// search frame_offsset > offset 
//FileOffset search_offset(metadata *meta,int stream_no, 
picFrame_metadata * search_frame(metadata *meta,int stream_no, 
    int switch_stream_no,FileOffset offset, int no_of_streams, int direction,
    int curr_direction, int frame_type){

	picFrame_metadata *frame;
	int size_of_frame;
	FileOffset present_offset;
	int err;
	FILE *meta_file;	
	long pos, frame_no, seek_offset;
	err = 1;
	meta_file = meta->meta_file;
	present_offset = -1;
	frame = (picFrame_metadata*)malloc(sizeof(picFrame_metadata));	
	while(present_offset == -1){
		present_offset = read_next_frame(meta_file,no_of_streams, stream_no,frame, &err);
		if (err == 0)
			return (picFrame_metadata *)-1; //will break at the lastframe: change it
	}
//	free(frame);
	pos = ftell(meta_file);

	if( (present_offset > offset && curr_direction == DIR_FWD) || 
        (present_offset < offset && curr_direction == DIR_REV) ){
		if(pos <= meta->header->metadata_offset) {
			return frame;
        }
        free(frame);
		frame =
        search_reverse(meta_file,no_of_streams,stream_no,present_offset,offset,
        curr_direction );
//        frame_no = frame->frame_number;
#ifdef PRN_DEBUG
    printf("\n search_reverse frame details: frame_number %d, frametype %d", frame->frame_number, frame->frametype); 
#endif //PRN_DEBUG
    } else {
        free(frame);
		frame =
        search_forward(meta_file,no_of_streams,stream_no,present_offset,offset,
        curr_direction );

//        frame_no = frame->frame_number - 1;
#ifdef PRN_DEBUG
    printf("\n search_forward frame + 1 details: frame_number %d, frametype %d", frame->frame_number, frame->frametype); 
#endif //PRN_DEBUG
    }	
    
  
    /* if frame type is not specified. */
    if(frame->offsets[switch_stream_no] != -1 && ( frame_type == ANY_FRAME || 
        frame_type == frame->frametype )) {
        present_offset = frame->offsets[switch_stream_no];
#ifdef PRN_DEBUG
        printf("\n (No find_next) found frame details: frame_number %d, frametype %d", frame->frame_number, frame->frametype); 
        printf("\n searched offset is %lld", present_offset);
        fflush(stdout);
#endif
//        free(frame);
//        return present_offset;
        return frame;
    }

    // reseting the meta file to seach for the switch stream offset 	
    frame_no = frame->frame_number;
    
    size_of_frame = sizeof(frame->frame_number) + sizeof(frame->frametype) +
             no_of_streams*sizeof(FileOffset);	
    free(frame);

    seek_offset = meta->header->metadata_offset + (frame_no)*size_of_frame;

    fseek(meta->meta_file,seek_offset,SEEK_SET);
    
    if(direction == DIR_FWD) {
        frame =
        find_next_of_frame_type(meta_file,no_of_streams,switch_stream_no,frame_type);
    } else {
        frame =
        find_prev_of_frame_type(meta_file,no_of_streams,switch_stream_no,frame_type);
    }

    present_offset = frame->offsets[switch_stream_no]; 

#ifdef PRN_DEBUG
    printf("\n found frame details: frame_number %d, frametype %d", frame->frame_number, frame->frametype); 
    printf("\n searched offset is %lld", present_offset);
    fflush(stdout);
#endif
//    free(frame);

//    return present_offset;

    return frame;
    /*
    } else { // if frame_type is specified.

#ifdef PRN_DEBUG
        printf("\n frame_type is specified");
#endif //PRN_DEBUG

        if( frame->offsets[switch_stream_no] != -1 && frame->frametype ==
            frame_type ) {
            present_offset = frame->offsets[switch_stream_no];
            free(frame);
#ifdef PRN_DEBUG
            printf("\n offset found. did not have to search. offset %ld", present_offset);
            printf("\n frame details: frame_number %x, frametype %d",
                    frame->frame_number, frame->frametype); 
#endif //PRN_DEBUG
            return present_offset; 
        }
 
        frame_no = frame->frame_number;
        size_of_frame = sizeof(frame->frame_number) + sizeof(frame->frametype) +
                 no_of_streams*sizeof(FileOffset);	

        seek_offset = meta->header->metadata_offset + (frame_no)*size_of_frame;

        fseek(meta->meta_file,seek_offset,SEEK_SET);
        	
        if(direction == DIR_FWD) {
            frame =
            find_next_of_frame_type(meta_file,no_of_streams,switch_stream_no,
                                    frame_type); 
        } else {
            frame =
            find_prev_of_frame_type(meta_file,no_of_streams,switch_stream_no,
                                    frame_type);
        }

        present_offset = frame->offsets[switch_stream_no]; 
        
#ifdef PRN_DEBUG        
        printf("\n searched offset of frame_type %d, is offset %ld", frame_type,
        present_offset);
        printf("\n frame details: frame_number %x, frametype %d",
        frame->frame_number, frame->frametype); 
        fflush(stdout); 
#endif //PRN_DEBUG

        free(frame);
        return present_offset;
    }
    */
}

picFrame_metadata* find_next_of_frame_type(FILE *file,int no_of_streams,
int stream_no, int frame_type)
{
    int err;
	picFrame_metadata *frame;

#ifdef PRN_DEBUG
    printf("\n Arguments to find_next_of_frame_type file %x, no_of_streams %d, stream no %d, frame_type %d", file, no_of_streams, stream_no, frame_type);
#endif //PRN_DEBUG

	frame = (picFrame_metadata*)malloc(sizeof(picFrame_metadata));

	do {
		read_next_frame(file,no_of_streams,stream_no,frame,&err);
		if(err == 0)
			return frame;
	}while( frame_type != frame->frametype );

	return frame;
}

picFrame_metadata* find_prev_of_frame_type(FILE *file,int no_of_streams,
int stream_no, int frame_type)
{
    int err;
	picFrame_metadata *frame;

#ifdef PRN_DEBUG
    printf("\n Arguments to find_prev_of_frame_type file %x, no_of_streams %d, stream no %d, frame_type %d", file, no_of_streams, stream_no, frame_type);
#endif //PRN_DEBUG

	frame = (picFrame_metadata*)malloc(sizeof(picFrame_metadata));

	do {
		read_previous_frame(file,no_of_streams,stream_no,frame,&err);
		if(err == 0)
			return frame;
	}while( frame_type != frame->frametype );

	return frame;
}

picFrame_metadata *search_reverse(FILE *file,int no_of_streams,int stream_no,
FileOffset last_offset, FileOffset offset, int curr_direction ){

	FileOffset present_offset;
	int err;
    int done;
	picFrame_metadata *frame, *old_frame;

	present_offset =  last_offset;
	frame = (picFrame_metadata*)malloc(sizeof(picFrame_metadata));
    done = (curr_direction == DIR_FWD) ? present_offset < offset :
                                         present_offset >= offset ; 
	while( !done ) {
        old_frame = frame;
		present_offset = read_previous_frame(file,no_of_streams,stream_no,frame,&err);
		if(err == 0)
			return frame;

        done = (curr_direction == DIR_FWD) ? present_offset < offset :
                                       present_offset >= offset ; 

#ifdef PRN_DEBUG
//        printf("search_forward frame->frame_number, present_offset %lld offset %lld\n",
//        frame->frame_number, present_offset, offset);
//        fflush(stdout);
#endif // PRN_DEBUG

	}

	return frame;
	
}

picFrame_metadata *search_forward(FILE* file,int no_of_streams,int stream_no,
FileOffset last_offset, FileOffset offset, int curr_direction ){

	FileOffset tmp_offset,present_offset;
	int err;
    int done;
	picFrame_metadata *frame, *old_frame;	


	present_offset = last_offset;
	frame = (picFrame_metadata*)malloc(sizeof(picFrame_metadata));
	
    done = (curr_direction == DIR_FWD) ? present_offset >= offset :
                                         present_offset < offset ; 
	while( !done ) {
        old_frame = frame;
		tmp_offset= read_next_frame(file,no_of_streams,stream_no,frame,&err);
		if(err == 0) 
			return frame;
		present_offset = tmp_offset;
    
        done = (curr_direction == DIR_FWD) ? present_offset >= offset :
                                         present_offset < offset ; 

#ifdef PRN_DEBUG
//        printf("search_forward frame->frame_number, present_offset %lld offset %lld\n",
//        frame->frame_number, present_offset, offset);
//        fflush(stdout);
#endif // PRN_DEBUG
			
	}

	return frame;

}

FileOffset read_previous_frame(FILE* f,int no_of_streams, int stream_no, picFrame_metadata *frame,int *err){
	
	int size_of_frames;	
	FileOffset offset;
	int size_of_frame;	
	
	size_of_frame = sizeof(frame->frame_number) + sizeof(frame->frametype) +
			 no_of_streams*sizeof(FileOffset);	
    do{
        *err = fseek(f,-2*(size_of_frame),SEEK_CUR);
        if (*err != 0){
            *err = 0;
            return 0;
        }
        *err = fread(&(frame->frame_number),sizeof(frame->frame_number),1,f);
        if(*err != 1){
            *err = 0;
            return 0;
        }
        *err = fread(&(frame->frametype),sizeof(frame->frametype),1,f);
        if(*err != 1){
            *err = 0;
            return 0;
        }
        
        *err = fread(frame->offsets,sizeof(FileOffset),no_of_streams,f);
        if(*err != no_of_streams){
            *err = 0;
            return 0;
        }
        
        offset = frame->offsets[stream_no]; 
        //*err = fseek(f,-1*(size_of_frame),SEEK_CUR);
        //if(*err != 0){
        //	*err = 0;
        //	return 0;
        //}
            

    }while(offset ==-1 && frame->frame_number != 0);

    if(offset == -1) {
	    *err = 0;
	    return 0;
    }
    *err =1;
    return offset;

		
}


FileOffset read_next_frame(FILE* f,int no_of_streams, int stream_no,picFrame_metadata *frame, int *err){

	int size_of_frames;	
	FileOffset offset;
	int size_of_frame;
	
	size_of_frame = sizeof(frame->frame_number) + sizeof(frame->frametype) +
			 no_of_streams*sizeof(FileOffset);	

    do{
    //	*err = fseek(f,(size_of_frame),SEEK_CUR);
    //	if (*err != 0){
    //		*err =0;
    //		return 0;
    //	}
        *err = fread(&frame->frame_number,sizeof(frame->frame_number),1,f);
        if(*err != 1){
            *err = 0;
            return 0;
        }
        *err = fread(&frame->frametype,sizeof(frame->frametype),1,f);
        if(*err != 1){
            *err = 0;
            return 0;
        }
        
        *err = fread(frame->offsets,sizeof(FileOffset),no_of_streams,f);
        if(*err != no_of_streams){
            *err = 0;
            return 0;
        }
        
        offset = frame->offsets[stream_no]; 
	
    }while(offset ==-1 && !feof(f));

    if(offset == -1) {
	    *err = 0;
	    return 0;
    }
    *err = 1;
    return offset;

}

stream_metadata *get_stream_metadata(metadata *meta, int speed, int *stream_no) {

	int i;
	stream_metadata *stream;
	assert(meta != NULL);
	assert(meta->header != NULL);

	stream = NULL;
	*stream_no = -1;
	for(i=0;i<meta->header->no_of_streams; i++){
	    if(meta->stream[i]->speed == speed){
			stream = meta->stream[i];
			*stream_no = i;
			break;
		}
	}
	return stream;	
}
