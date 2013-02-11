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
 * $(@)meta.c $Revision: 1.2 $ $Date: 2006/07/15 00:02:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform TS parsing

 * Created:   August, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/

/* bugs: not memory allocation testing */
#include "ecmfile.h"

#include <errno.h>
#include <string.h>

ECMfile_info *GetECMFileInfo(FILE *hECMFile)
{
    int error = 0;

    /* read the header information */
    ECMfile_info *info = (ECMfile_info *)malloc(sizeof(ECMfile_info));
    if (info == NULL)
    	return NULL;
    
    assert(hECMFile != NULL);

    fseek(hECMFile, 0, SEEK_SET);
    //    error = fread(&info->header,1,sizeof(ECMfile_header),hECMFile);
    //    assert( error == sizeof(ECMfile_header));
    int nread = fread( &info->header.version, 1, sizeof(info->header.version),
                       hECMFile );
    nread += fread( &info->header.no_of_ECMs, 1, sizeof(info->header.no_of_ECMs),
                    hECMFile );
    nread += fread( &info->header.ecm_len, 1, sizeof(info->header.ecm_len), 
                    hECMFile);
    nread += fread( &info->header.metadata_offset, 1, 
                    sizeof(info->header.metadata_offset), hECMFile );

    if( nread != ECM_FILE_HEADER_SIZE ) {
        printf("ERROR: GetECMFileInfo(): Reading from Ecm file failed: %s\n",
               strerror(errno) );
        free(info);
        return NULL;
    }

    info->item_len = info->header.ecm_len + sizeof(FileOffset) +  sizeof(FileOffset);
    info->ecmItem.ecm_payload = (unsigned char *)malloc(info->header.ecm_len);

    return info;
}

void CleanECMInfo(ECMfile_info *info)
{
    if (info != NULL) 
    {
    	if (info->ecmItem.ecm_payload != NULL)
            free(info->ecmItem.ecm_payload);
        free(info);
    }
    return;
}


void WriteOneECMItem(FILE *hECMFile, ECMfile_info *info, 
	                     int index, unsigned char *buffer, int len, FileOffset pos)
{
    long position = 0L;
    FileOffset insert_pos = -1; /* don't know */
    FileOffset new_pos = (FileOffset)pos;
	assert(hECMFile != NULL);
    position = ftell(hECMFile);
    fseek(hECMFile, info->header.metadata_offset + info->item_len * index, SEEK_SET);
 	
	fwrite(&insert_pos, 1,sizeof(FileOffset),hECMFile); 
	fwrite(&pos, 1,sizeof(FileOffset),hECMFile);
	fwrite(buffer, 1, len, hECMFile);
    /* restore the file indicator */
    fseek(hECMFile, position, SEEK_SET);
    return;
}

void WriteECMFileHeader(FILE *hECMFile, ECMfile_info *info)
{
    assert(hECMFile != NULL);

    fseek(hECMFile, 0, SEEK_SET);

    //    int size = sizeof(ECMfile_header);    
    //fwrite(&info->header, 1, size, hECMFile);

    fwrite( &info->header.version, 1, sizeof(info->header.version), hECMFile );
    fwrite( &info->header.no_of_ECMs, 1, sizeof(info->header.no_of_ECMs), hECMFile);
    fwrite( &info->header.ecm_len, 1, sizeof(info->header.ecm_len), hECMFile );
    fwrite( &info->header.metadata_offset, 1, sizeof(info->header.metadata_offset), hECMFile);

    return;
}

void ReWrOneECMItem(FILE *hECMFile, ECMfile_info *info, 
	                     int index, bool b_read)
{
    long position = 0L;
	assert(hECMFile != NULL);
    position = ftell(hECMFile);
    fseek(hECMFile, info->header.metadata_offset + info->item_len * index, SEEK_SET);
 	assert(info->ecmItem.ecm_payload);
 	
    if (b_read)
    {
	    fread(&info->ecmItem.insert_offset,1,sizeof(FileOffset),hECMFile);
	    fread(&info->ecmItem.new_offset, 1,sizeof(FileOffset),hECMFile);
	    fread(info->ecmItem.ecm_payload, 1, info->header.ecm_len, hECMFile);
    }
    else
    {
	    fwrite(&info->ecmItem.insert_offset,1,sizeof(FileOffset),hECMFile);
	    fwrite(&info->ecmItem.new_offset, 1,sizeof(FileOffset),hECMFile);
	    fwrite(info->ecmItem.ecm_payload, 1, info->header.ecm_len, hECMFile);
    }
    /* restore the file indicator */
    fseek(hECMFile, position, SEEK_SET);
    return;
}

void ConvertECMfile(FILE *hECMFile, FILE *ascii)
{
    ECMfile_info *info;
    int i = 0;
    info = GetECMFileInfo(hECMFile);
    if (info == NULL)
    	return;
    fprintf(ascii, "Version: %d\n", info->header.version);
    fprintf(ascii, "Number of ECMs: %d\n", info->header.no_of_ECMs);
    fprintf(ascii, "Length of ECM: %d\n", info->header.ecm_len);
    fprintf(ascii, "Offset: %d\n", info->header.metadata_offset);
    
    for (i = 0; i < info->header.no_of_ECMs; i++)
    {
    	ReWrOneECMItem(hECMFile, info, i, TRUE);
	    fprintf(ascii, "ECM %10d, InsertOffset: %10d, NewOffset: %10d\n", 
	    	i, (int)(info->ecmItem.insert_offset), (int)(info->ecmItem.new_offset));
    }
    CleanECMInfo(info);
    
    return;
}

static void ResetInsertPosition(FILE *hECMFile, ECMfile_info *info)
{
    int i = 0;
    for (i = 0; i < info->header.no_of_ECMs; i++)
    {
    	ReWrOneECMItem(hECMFile, info, i, TRUE);
    	info->ecmItem.insert_offset = -1;
    	ReWrOneECMItem(hECMFile, info, i, FALSE);
    }
    return;
}


static int ReadUpdateMetaDataItem(metadata *p_meta, picFrame_metadata *picFrame, int frame_no, bool b_read)
{
    long position = ftell(p_meta->meta_file);
    long offset = sizeof(picFrame->frame_number) + sizeof(picFrame->frametype) 
    	        + p_meta->header->no_of_streams*sizeof(FileOffset);

    long seek_offset = p_meta->header->metadata_offset + (frame_no)*offset;

	fseek(p_meta->meta_file, seek_offset, SEEK_SET);

    if (b_read)
    {
		fread(&picFrame->frame_number,1,
			sizeof(picFrame->frame_number),p_meta->meta_file);
		fread(&picFrame->frametype,1,
			sizeof(picFrame->frametype),p_meta->meta_file);
		fread(picFrame->offsets,1,
			 p_meta->header->no_of_streams*sizeof(FileOffset),p_meta->meta_file);
    }
    else
    {
		fwrite(&picFrame->frame_number,1,
			sizeof(picFrame->frame_number),p_meta->meta_file);
		fwrite(&picFrame->frametype,1,
			sizeof(picFrame->frametype),p_meta->meta_file);
		fwrite(picFrame->offsets,1,
			 p_meta->header->no_of_streams*sizeof(FileOffset),p_meta->meta_file);
    }

	fseek(p_meta->meta_file, position, SEEK_SET);
    return 1;
}

metadata *OpenMetaDataFile(FILE *hMetaFile)
{
    metadata *p_meta;
    int i, size;
    long end_pos, pos;
    picFrame_metadata picFrame;
    /* meta data file */
	p_meta = (metadata *)malloc(sizeof(metadata));
	p_meta->header = (metadata_header *)malloc(sizeof(metadata_header));
	p_meta->meta_file = hMetaFile;
	p_meta->stream_no = -1; //no curr stream
	p_meta->speed = -1;

	fseek(hMetaFile,0,SEEK_SET);
	assert(fread(p_meta->header,1,sizeof(metadata_header),hMetaFile) != NULL);

	if(p_meta->header->version != METADATA_VERSION)	
		FATAL_ERROR("metadata version not supported");
	
	for(i=0;i<p_meta->header->no_of_streams; i++)
	{
		stream_metadata *stream = (stream_metadata *)malloc(sizeof(stream_metadata));
    	assert(fread(stream,1,sizeof(stream_metadata),hMetaFile) != NULL);
		p_meta->stream[i] = stream;
	}
        
    size = sizeof(picFrame.frame_number) + sizeof(picFrame.frametype) 
    	        + p_meta->header->no_of_streams*sizeof(FileOffset);
    pos = ftell(hMetaFile);
    fseek(hMetaFile,0,SEEK_END);
    end_pos = ftell(hMetaFile);
    fseek(hMetaFile, pos, SEEK_SET);
    p_meta->total_frames = (end_pos - p_meta->header->metadata_offset)/size;
    return p_meta;
}

void CleanMetaData(metadata *p_meta)
{
    if (p_meta)
    {
		int i;
		assert(p_meta->header != NULL);
		for(i=0;i<p_meta->header->no_of_streams;i++)
		{
			if (p_meta->stream[i])
			{
			    free(p_meta->stream[i]);
			}
		}
		free(p_meta->header);
		free(p_meta);
    }
    return;
}

void UpdateECMandMetaFile(FILE *hECMFile, FILE* hMetaFile, int stream_no, int ecm_ts_len)
{
    metadata *p_meta;
    ECMfile_info *info;
    int ecm_index = 0;
    int from;
    int total_ecm_ts_inserted = 0;
    int frame_no = 0;
    int frame_start, frame_end;
    bool b_reverse;
	assert(hECMFile != NULL && hMetaFile != NULL);

    info = GetECMFileInfo(hECMFile);
    if (info->header.no_of_ECMs < 1)
    {
    	return; /* no need to update the meta data file */
    }

    ResetInsertPosition(hECMFile, info);
    assert(info->header.ecm_len > 0);

    p_meta = OpenMetaDataFile(hMetaFile);
    assert(p_meta->header->no_of_streams >= stream_no);
    
    b_reverse = p_meta->stream[stream_no]->speed > 0 ? FALSE : TRUE;
    from = b_reverse? info->header.no_of_ECMs - 1 : 0;
    
    frame_start = b_reverse? p_meta->total_frames - 1 : 0;
    frame_end   = b_reverse? 0 : p_meta->total_frames - 1;
    
	for (frame_no = frame_start; frame_no != frame_end; frame_no += b_reverse?-1:1) 
	{
	    picFrame_metadata picFrame;
		int err = ReadUpdateMetaDataItem(p_meta,&picFrame, frame_no, TRUE);
		if(err == 0)
		{
			break;
		}
		
		if (picFrame.offsets[stream_no] == -1) // not availabe in this stream
		{
			continue;
		}

		/* search for the ECM used for this frame */
		if (!b_reverse)
		{
			for (ecm_index = from; ecm_index < info->header.no_of_ECMs; ecm_index ++)
			{
				ReWrOneECMItem(hECMFile, info, ecm_index, TRUE);
				if ( info->ecmItem.new_offset > picFrame.offsets[0])
				{
					break;
				}
			}
			ecm_index --;
		}
		else
		{
			for (ecm_index = from; ecm_index >= 0; ecm_index --)
			{
				ReWrOneECMItem(hECMFile, info, ecm_index, TRUE);
				if ( info->ecmItem.new_offset < picFrame.offsets[0])
				{
					break;
				}
			}
		}
		
		if (ecm_index < 0)
		{
			/* no ECM for this frame, no need to update */
            from = b_reverse? info->header.no_of_ECMs - 1 : 0;
			continue;
		}
		else
		{
	        ReWrOneECMItem(hECMFile, info, ecm_index, TRUE);
	        if (info->ecmItem.insert_offset == -1) // not calculated
	        {
	        	/* need to insert this ECM in front of this frame, update ECM file */
	        	/* at TS boundary */
	        	info->ecmItem.insert_offset = picFrame.offsets[stream_no] / TP_LENGTH * TP_LENGTH;
	        	ReWrOneECMItem(hECMFile, info, ecm_index, FALSE);
	        	total_ecm_ts_inserted += ecm_ts_len;
	        	
			     /* if the ECM is inserted right in front of the beginning of one frame */
			     /* adjust the position to be the position of the ECM packet */
			     picFrame.offsets[stream_no] -= TP_LENGTH;
	        }
	        /* update the meta data file */
	        picFrame.offsets[stream_no] += total_ecm_ts_inserted;
            ReadUpdateMetaDataItem(p_meta,&picFrame, frame_no, FALSE);
	        from = ecm_index;
		}
	}

    CleanMetaData(p_meta);
    CleanECMInfo(info);
	return;
}

int CheckReverse(FILE* hMetaFile, int stream_no)
{
    metadata *p_meta;
    int reverse = 0;
	assert(hMetaFile != NULL);
    p_meta = OpenMetaDataFile(hMetaFile);
    assert(p_meta->header->no_of_streams >= stream_no);
    reverse = p_meta->stream[stream_no]->speed > 0 ? FALSE : TRUE;
    CleanMetaData(p_meta);
    return reverse;

}

