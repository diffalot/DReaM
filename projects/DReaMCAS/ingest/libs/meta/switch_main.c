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
 * $(@)switch_main.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * =====================================================================================
 * 
 *        Filename:  switch_main.c
 * 
 *     Description:  Test program for checking switch_stream
 * 
 *         Version:  1.0
 *         Created:  09/11/04 15:48:08 PDT
 *        Revision:  none
 *        Compiler:  cc
 * 
 *          Author:   (Deepak Chandra), 
 *         Company:  
 * 
 * =====================================================================================
 */


#include<stdio.h>
#include "metadata.h"



void main(int argc , char* argv[]){

	FileOffset offset, switch_offset;
	metadata *meta;	
	

	if(argc != 2){
		printf("usage:%s meta_file\n",argv[0]);
		exit(0);
	}
		
	
	offset = 0x815345;
	meta= open_metadata_file(argv[1]);
	
	printf("Switching from 1X stream offset%llx to 12X stream\n", offset);
	switch_offset = switch_stream(meta,12,offset,-12,FF);
	printf("switched offset is %llx", switch_offset);

}
