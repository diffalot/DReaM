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
 * $(@)ecmfile.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 2005 by Sun Microsystems, Inc.
 * Functions to perform TS parsing

 * Created:   August, 2005
 * Author:    Yongfang Liang
 *---------------------------------------------------------------*/
#ifndef ECM_FILE_H
#define ECM_FILE_H

#include  "meta/metadata.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define bool int
#define TP_LENGTH 188
#define TRUE 1
#define FALSE 0


/* structure of the ECM meta file */
/*
    ECMfile_header
    ECMfile_data_item
*/
typedef struct  {
	int version;
 	int no_of_ECMs;	
 	int ecm_len;
	FileOffset metadata_offset;
} ECMfile_header;

#define ECM_FILE_HEADER_SIZE (sizeof(int) + sizeof(int) + sizeof(int) + sizeof(FileOffset))

typedef struct  {
/* positions of */
	FileOffset insert_offset; /* with respect to the file without ECM */
	FileOffset new_offset;   /* with resepct to the new file */
 	unsigned char *ecm_payload; /* note: length of the payload only, each ECM may contain multiple TS packets */
} ECMfile_data_item;

typedef struct  {
	ECMfile_header header;
	int item_len;
    ECMfile_data_item ecmItem;
	
} ECMfile_info;


ECMfile_info *GetECMFileInfo(FILE *hECMFile);
void CleanECMInfo(ECMfile_info *info);
void ReWrOneECMItem(FILE *hECMFile, ECMfile_info *info, 
	                     int index, bool b_read);
void ConvertECMfile(FILE *hECMFile, FILE *ascii);
void UpdateECMandMetaFile(FILE *hECMFile, FILE* hMetaFile, int stream_no, int ecm_ts_len);
void WriteOneECMItem(FILE *hECMFile, ECMfile_info *info, 
	                     int index, unsigned char *buffer, int len, FileOffset pos);
void WriteECMFileHeader(FILE *hECMFile, ECMfile_info *info);
int CheckReverse(FILE* hMetaFile, int stream_no);

#endif

