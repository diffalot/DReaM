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
 * $(@)file_switching.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform TS parsing

 * Created:   August, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
#include "ecmfile.h"

#define NO_FRAMES 500
#define READ_WRITE_BUFFER_LEN 8192
int  my_read_frame(metadata *meta, picFrame_metadata *frame);
void CleanMetaData(metadata *p_meta);
metadata *OpenMetaDataFile(FILE *hMetaFile);

void Copyfile(FILE *in, FILE *out, int len)
{
    static unsigned char buffer[READ_WRITE_BUFFER_LEN];
    int i = 0;
    int end = len / READ_WRITE_BUFFER_LEN;
    for (i = 0; i < end; i++)
    {
        int size = fread(buffer, 1, READ_WRITE_BUFFER_LEN, in);
        size = fwrite(buffer, 1, READ_WRITE_BUFFER_LEN, out);
    }
    
    fread(buffer, 1, len % READ_WRITE_BUFFER_LEN, in);
    fwrite(buffer, 1, len % READ_WRITE_BUFFER_LEN, out);
	return;
}

/* test File1 --> File2 --> File1 
from file1 switch to file2, then back to file1 */

void StitchFile(char *chFile1, char* chFile2, char *chMetaFile, char *chOutFile, 
                int stream1_no, int stream2_no)
{
    metadata *p_meta;
    FILE *hFile1, *hFile2,  *hOutFile, *hMetaFile;
    int i = 0, frame_no = 0;
    FileOffset start_pos = 0, end_pos = 0;
	picFrame_metadata picFrame;
    
    hFile1 = fopen(chFile1, "rb");
    hFile2 = fopen(chFile2, "rb");
    hMetaFile = fopen(chMetaFile, "rb");
    hOutFile = fopen(chOutFile, "wb");
    
	assert(hFile1 != NULL && hFile2 != NULL 
		   && hMetaFile != NULL && hOutFile != NULL);

    p_meta = OpenMetaDataFile(hMetaFile);
    assert(p_meta->header->no_of_streams >= 2); 
    assert(stream1_no < p_meta->header->no_of_streams 
    	   && stream2_no < p_meta->header->no_of_streams); // stream index verification

    printf(" Start ...\n");
    	
	/* read file 1 */
	do 
	{
		int err = my_read_frame(p_meta,&picFrame);
		if(err == 0)
		{
			return;
		}
		if (picFrame.offsets[stream1_no] == -1)
		{
			continue;
		}
		else
		{
            frame_no++;
		}
	}while ( frame_no < NO_FRAMES);

    end_pos = picFrame.offsets[stream1_no] / TP_LENGTH * TP_LENGTH;
    Copyfile(hFile1, hOutFile, (int)(end_pos - start_pos));

	/* read file 2 */
    while ( picFrame.offsets[stream2_no] == -1)
    {
		int err = my_read_frame(p_meta,&picFrame);
		if(err == 0)
		{
			return;
		}

    };

	frame_no = 0;
    start_pos = picFrame.offsets[stream2_no] / TP_LENGTH * TP_LENGTH;
    fseek(hFile2, start_pos, SEEK_SET);
	do 
	{
		int err = my_read_frame(p_meta,&picFrame);
		if(err == 0)
		{
			return;
		}
		if (picFrame.offsets[stream2_no] == -1)
		{
			continue;
		}
		else
		{
            frame_no++;
		}
	}while ( frame_no < 300);

    end_pos = picFrame.offsets[stream2_no] / TP_LENGTH * TP_LENGTH;
    Copyfile(hFile2, hOutFile, (int)(end_pos - start_pos));

    /* back to file 1 */
    while ( picFrame.offsets[stream1_no] == -1)
    {
		int err = my_read_frame(p_meta,&picFrame);
		if(err == 0)
		{
			return;
		}

    };
	/* read file 1 */
	frame_no = 0;
    start_pos = picFrame.offsets[stream1_no] / TP_LENGTH * TP_LENGTH;
    fseek(hFile1, start_pos, SEEK_SET);
	do 
	{
		int err = my_read_frame(p_meta,&picFrame);
		if(err == 0)
		{
			return;
		}
		if (picFrame.offsets[stream1_no] == -1)
		{
			continue;
		}
		else
		{
            frame_no++;
		}
	}while ( frame_no < NO_FRAMES);

    end_pos = picFrame.offsets[stream1_no] / TP_LENGTH * TP_LENGTH;
    Copyfile(hFile1, hOutFile, (int)(end_pos - start_pos));
    printf(" Done !\n");
    fclose(hFile1);
    fclose(hFile2);
    fclose(hMetaFile);
    fclose(hOutFile);
    CleanMetaData(p_meta);
    
	return;
}

static void usage(const char *progName) 
{
	printf("usage: %s 1Xfile nXfile metafile outfile", progName);	
	exit(0);
}

int main(int argc, char **argv)
{
    if(argc != 5) 
    {
    	usage(argv[0]);
    }   

    StitchFile(argv[1], argv[2], argv[3], argv[4], 0 , 1);
    
    return 0;
}


