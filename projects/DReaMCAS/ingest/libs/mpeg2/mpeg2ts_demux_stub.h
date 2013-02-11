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
 * $(@)mpeg2ts_demux_stub.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:43 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_DEMUX_STUB_H
#define MPEG2TS_DEMUX_STUB_H

struct PmtHeader;

typedef struct 
{
    unsigned char data[1024];
    int      length;
} Buffer1k;

typedef struct 
{
    unsigned char tag;
    /*unsigned char length;
    unsigned char *data;
    */
    Buffer1k buffer;
}OpaqueDescriptor ;


typedef Buffer1k PsiBytes;


typedef struct
{
    unsigned char data[188];
}TsBytes;

#ifdef __cplusplus
extern "C" {
#endif
    void stringify( const unsigned char *data, int nbytes );

    struct PmtHeader *createProgramMapSection( int programNumber, int versionNumber,
					       int pcrPid );
    void addProgramMapDescriptor( struct PmtHeader *pmthdr, 
				  OpaqueDescriptor *descriptor );

    void addPmtEntry( struct PmtHeader *pmthdr, unsigned char streamType, int esPid,
		      OpaqueDescriptor *descriptorArray, int numDescriptors );

    PsiBytes getPmtBytes( struct PmtHeader *pmthdr );

    TsBytes createTransportPacket( int pid, int continuityCounter,
				   PsiBytes payload );

    TsBytes createTransportPacketForPsi( int pid, int continuityCounter,
							 PsiBytes psibytes );

#ifdef __cplusplus
}
#endif

#endif
