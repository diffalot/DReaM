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
 * $(@)mpeg2ts_demux_stub.cc $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "mpeg2ts_demux_stub.h"
#include "PsiFilter.h"
#include "Descriptors.h"
#include "OutputBitBuffer.h"

#include <string>
#include <boost/shared_ptr.hpp>

namespace m2t=mpeg2ts;

void stringify( const unsigned char *data, int nbytes )
{
    for( int i=0; i<nbytes; ++i )
    printf("%02x ", data[i] );

    printf("\n");

}

struct PmtHeader *createProgramMapSection( int programNumber, int versionNumber,
               int pcrPid )
{
    m2t::Pmt *pmt = new m2t::Pmt( programNumber, pcrPid );
    pmt->versionNumber = versionNumber;
    
    return (struct PmtHeader *)pmt;
}

void addProgramMapDescriptor( struct PmtHeader *pmthdr, OpaqueDescriptor *descriptor )
{
    m2t::Pmt *pmt=(m2t::Pmt *)pmthdr;
    m2t::DescriptorPtr udescriptor( new m2t::UnknownDescriptor(
                                          descriptor->tag,
                                          std::string( (const char *)descriptor->buffer.data,
                                          (int)descriptor->buffer.length ) ) );
    pmt->descriptors.push_back( udescriptor );

    return;
}

void addPmtEntry( struct PmtHeader *pmthdr, unsigned char streamType, int esPid,
      OpaqueDescriptor *descriptorArray, int numDescriptors )
{
    m2t::Pmt *pmt=(m2t::Pmt *)pmthdr;

    m2t::PmtEntry pmtEntry( streamType, esPid );
    
    for( int i=0; i<numDescriptors; ++i ) 
    {
        OpaqueDescriptor *desc = &descriptorArray[i];

        m2t::DescriptorPtr descriptor( new m2t::UnknownDescriptor(
                                         desc->tag,
			                             std::string( (const char *)desc->buffer.data,
				                         (int)desc->buffer.length ) ) );

        pmtEntry.descriptors.push_back( descriptor );
    }
    pmt->entries.push_back( pmtEntry );
}



PsiBytes getPmtBytes( struct PmtHeader *pmthdr )
{
    PsiBytes retval;
    retval.length=0;
    m2t::Pmt& pmt = *(m2t::Pmt *)pmthdr;

    m2t::OutputBitBuffer outb( retval.data, sizeof(retval.data) );

    outb << pmt;

    retval.length=pmt.sizeInBytes();

    return retval;
    
}



static void createTsPktHeader( int pid, int continuityCounter, 
           m2t::OutputBitBuffer& outb )
{
    m2t::TsPacket tspkt;
    
    tspkt.syncByte = 0x47;
    tspkt.transportErrorIndicator=0;
    tspkt.payloadUnitStartIndicator = 1;
    tspkt.transportPriority = 0;
    tspkt.pid = pid;
    tspkt.transportScramblingControl = 0;
    tspkt.adaptationFieldControl = m2t::TsPacket::AfcPayloadOnly;
    tspkt.continuityCounter = continuityCounter;

    outb.writeBits( 8, tspkt.syncByte );
    outb.writeBits( 1, tspkt.transportErrorIndicator );
    outb.writeBits( 1, tspkt.payloadUnitStartIndicator );
    outb.writeBits( 1, tspkt.transportPriority );
    outb.writeBits( 13, tspkt.pid );
    outb.writeBits( 2, tspkt.transportScramblingControl );
    outb.writeBits( 2, tspkt.adaptationFieldControl );
    outb.writeBits( 4, tspkt.continuityCounter );

    return;
}

TsBytes createTransportPacketForPsi( int pid, int continuityCounter,
             PsiBytes psibytes )
{


    TsBytes retval;
    m2t::OutputBitBuffer outb( retval.data, sizeof(retval.data) );

    createTsPktHeader( pid, continuityCounter, outb );

    unsigned char *p = &retval.data[4];

    *p++=0; /* pointer_field */

    assert( psibytes.length <= 183 );

    for( int i=0; i<psibytes.length; ++i )
    *p++=psibytes.data[i];

    int remainder = 183 - psibytes.length;
    for( int i=0; i<remainder; ++i ) 
    *p++ = 0xff;

    return retval;
}

TsBytes createTransportPacket( int pid, int continuityCounter,
           PsiBytes payload )
{


    TsBytes retval;
    m2t::OutputBitBuffer outb( retval.data, sizeof(retval.data) );

    createTsPktHeader( pid, continuityCounter, outb );

    assert( payload.length <= 184 );

    unsigned char *p = &retval.data[4];

    for( int i=0; i<payload.length; ++i )
    *p++ = payload.data[i];

    int remainder = 183 - payload.length;
    for( int i=0; i<remainder; ++i ) 
    *p++ = 0xff;

    return retval;
}    
