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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


#define METADATA_VERSION	1
#define MAX_NO_STREAMS		6
#define ERROR_PREFIX fprintf(stderr,"\nERROR in %s line %d:\n",__FILE__,__LINE__);
#define FATAL_ERROR(a) {ERROR_PREFIX;fprintf(stderr,a);exit(-1);}

#define DIR_FWD	1
#define DIR_REV	2

#define ANY_FRAME     -1
#define I_FRAME       1
#define P_FRAME       2
#define B_FRAME       3
#define UNKNOWN_FRAME 4

typedef long long FileOffset; 

typedef struct  {
	int version;
 	int no_of_streams;	
	long metadata_offset;
} metadata_header;


typedef struct {
	int speed;
	long avg_frame_length;
} stream_metadata;	

typedef struct {

	metadata_header *header;
	int stream_no;
	int speed;
	stream_metadata *stream[MAX_NO_STREAMS];
	FILE *meta_file;
	FILE *tmp_file;
	char *tmp_filename;
	char *meta_filename;
	FileOffset last_offset;
	long total_frames;
} metadata;

typedef struct{

	long frame_number;
	int frametype;
	int no_of_streams;
	FileOffset offsets[MAX_NO_STREAMS];
	
} picFrame_metadata; 




metadata *open_metadata_file(char* metadata_file_name);
int open_temp_metafile(metadata *meta, int speed) ;
long calc_position_to_write(metadata *meta, long frame_number);
FileOffset get_pos( metadata *meta, picFrame_metadata * frame, int speed );
picFrame_metadata* get_switch_frame(metadata *meta,int cur_speed, FileOffset cur_offset,
int switch_speed, int direction, int curr_direction, int frame_type);
stream_metadata *get_stream_metadata(metadata *meta, int speed, int *stream_no) ;
FileOffset read_next_frame(FILE* f,int no_of_streams, int stream_no,picFrame_metadata *frame, int *err);
FileOffset read_previous_frame(FILE* f,int no_of_streams, int stream_no, picFrame_metadata *frame,int *err);
picFrame_metadata *search_reverse(FILE *file,int no_of_streams,int stream_no,
FileOffset last_offset, FileOffset offset, int curr_direction);
picFrame_metadata *search_forward(FILE* file,int no_of_streams,int stream_no,
FileOffset last_offset, FileOffset offset, int curr_direction);
picFrame_metadata* search_frame(metadata *meta,int stream_no, 
    int switch_stream_no,FileOffset offset, int no_of_streams, int direction,
    int curr_direction, int frame_type);
picFrame_metadata* find_next_of_frame_type(FILE *file,int no_of_streams,
    int stream_no, int frame_type);
picFrame_metadata* find_prev_of_frame_type(FILE *file,int no_of_streams,
    int stream_no, int frame_type);

#ifdef __cplusplus
}
#endif
