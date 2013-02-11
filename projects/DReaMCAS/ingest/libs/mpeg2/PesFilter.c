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
 * $(@)PesFilter.cc $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "PesFilter.h"
#include <iostream>
#include <bitset>

namespace mpeg2ts {

struct PesHeaderFlags
{
    std::bitset<2>  b10;
    std::bitset<2>  pesScramblingControl;
    std::bitset<1>  pesPriority;
    std::bitset<1>  dataAlignmentIndicator;
    std::bitset<1>  copyright;
    std::bitset<1>  originalOrCopy;
    std::bitset<2>  ptsDtsFlags;
    std::bitset<1>  escrFlag;
    std::bitset<1>  esRateFlag;
    std::bitset<1>  dsmTrickModeFlag;
    std::bitset<1>  additionalCopyInfoFlag;
    std::bitset<1>  pesCrcFlag;
    std::bitset<1>  pesExtensionFlag;
    std::bitset<8>  pesHeaderDataLength;
};


struct PesHeader32bitValue
{
    std::bitset<3>  bits32_30;
    std::bitset<1>  markerBit1;
    std::bitset<15> bits29_15;
    std::bitset<1>  markerBit2;
    std::bitset<15> bits14_0;
    std::bitset<1>  markerBit3;
};

struct PesHeaderPts 
{
    std::bitset<4>  ptsDtsType; /* '0010' or '0011' */
    PesHeader32bitValue timeStamp;
};

struct PesHeader 
{
    std::bitset<24> packetStartCodePrefix;
    std::bitset<8>  streamId;
    std::bitset<16> pesPacketLength;
    bool            flagsValid;
    enum StreamId { 
           ProgramStreamMap = 0xbc,
           PrivateStream1   = 0xbd,
           PaddingStream    = 0xbe,
           PrivateStream2   = 0xbf,
           AudioStreamIdStart = 0xc0,
           AudioStreamIdEnd   = 0xdf,
           VideoStreamIdStart = 0xe0,
           VideoStreamIdEnd   = 0xef,
           EcmStream          = 0xf0,
           EmmStream          = 0xf1,
           DsmCcStream        = 0xf2,
           IsoIec1522Stream   = 0xf3,
           ItuTRecH2221TypeEStream = 0xf8,
           ProgramStreamDirectory = 0xff
    };
    PesHeaderFlags  flags;
};



struct PesHeaderEscr
{
    std::bitset<2>  reserved2;
    PesHeader32bitValue escrBase;
    std::bitset<9>  escrExtension;
    std::bitset<1>  marker;
};

struct PesHeaderEsRate
{
    std::bitset<1> marker1;
    std::bitset<22> esRate;
    std::bitset<1> marker2;
};

InputBitBuffer& operator>>( InputBitBuffer& source, PesHeaderFlags& pesHdrFlags )
{
    source >> pesHdrFlags.b10;
    source >> pesHdrFlags.pesScramblingControl;
    source >> pesHdrFlags.pesPriority;
    source >> pesHdrFlags.dataAlignmentIndicator;
    source >> pesHdrFlags.copyright;
    source >> pesHdrFlags.originalOrCopy;
    source >> pesHdrFlags.ptsDtsFlags;
    source >> pesHdrFlags.escrFlag;
    source >> pesHdrFlags.esRateFlag;
    source >> pesHdrFlags.dsmTrickModeFlag;
    source >> pesHdrFlags.additionalCopyInfoFlag;
    source >> pesHdrFlags.pesCrcFlag;
    source >> pesHdrFlags.pesExtensionFlag;
    source >> pesHdrFlags.pesHeaderDataLength;
    
    return source;
}

std::ostream& operator<<( std::ostream& out, PesHeaderFlags& pesHdrFlags )
{
    out << "\tb10: " << pesHdrFlags.b10;
    out << "\n\tpesScramblingControl: " << pesHdrFlags.pesScramblingControl;
    out << "\n\tpesPriority: " << pesHdrFlags.pesPriority;
    out << "\n\tdataAlignmentIndicator: " << pesHdrFlags.dataAlignmentIndicator;
    out << "\n\tcopyright: " << pesHdrFlags.copyright;
    out << "\n\toriginalOrCopy: " << pesHdrFlags.originalOrCopy;
    out << "\n\tptsDtsFlags: " << pesHdrFlags.ptsDtsFlags;
    out << "\n\tescrFlag: " << pesHdrFlags.escrFlag;
    out << "\n\tesRateFlag: " << pesHdrFlags.esRateFlag;
    out << "\n\tpesHeaderDataLength " << pesHdrFlags.pesHeaderDataLength.to_ulong();
    
    return out;
}

InputBitBuffer& operator>>( InputBitBuffer& source, PesHeader& peshdr )
{
    source >> peshdr.packetStartCodePrefix;
    source >> peshdr.streamId;
    source >> peshdr.pesPacketLength;
    peshdr.flagsValid = false;
    if( peshdr.packetStartCodePrefix.to_ulong() != 0X000001 ) {
        std::cout << "ERROR parsing PES Header: Start Code Prefix not present\n";
        // TODO: Throw InvalidBitstream ?
        return source;
    }
    long streamId = peshdr.streamId.to_ulong();
    if( streamId == PesHeader::PaddingStream ) 
    {
        assert( peshdr.pesPacketLength.to_ulong() != 0 );
        std::bitset<8> dummy;
        for( int i=0; i<peshdr.pesPacketLength.to_ulong(); ++i )
            source >> dummy;
    }
    if( streamId==PesHeader::ProgramStreamMap || streamId==PesHeader::PrivateStream2
        || streamId==PesHeader::EcmStream || streamId==PesHeader::EmmStream
        || streamId==PesHeader::ProgramStreamDirectory || streamId==PesHeader::DsmCcStream
        || streamId==PesHeader::ItuTRecH2221TypeEStream ) 
    {
        // The rest of the buffer is all data...
        NULL;
    }else
    {
        peshdr.flagsValid = true;
        source >> peshdr.flags;
        assert( peshdr.flags.b10 == std::bitset<2>(0x2) );
        // TODO: Fill up the pts/dts and escr fields
        std::bitset<8> dummy;
        for( int i=0; i<peshdr.flags.pesHeaderDataLength.to_ulong(); ++i )
            source >> dummy;
    }
  
    return source;
};

std::ostream& operator<<( std::ostream& out, PesHeader& peshdr )
{
    std::cout << "SC: " << peshdr.packetStartCodePrefix;
    std::cout << "\nStreamId: " << peshdr.streamId.to_ulong();
    std::cout << "\npes_packet_length: " << peshdr.pesPacketLength.to_ulong() << '\n';
    
    if( peshdr.flagsValid )
        out << peshdr.flags;   
    return out;
}

void PesFilter::processPesPacket() const
{
    const  char *data = m_buffer.data();
    int         len = m_buffer.size();
    
    InputBitBuffer buf((const unsigned char *)data, len);
    PesHeader      peshdr;
    
    buf >> peshdr;

    PairBytesBits consumed = buf.dataConsumed();
    
    assert( consumed.first > 0 );
    assert( consumed.second == 0 );
    
    data += consumed.first;
    len -= consumed.first;

    if( m_filter.get() ) {
        m_filter->process( PesBuffer((unsigned char *)const_cast< char *>(data), len,
                           m_mapBufferPos_StreamPos, (const unsigned char *) m_buffer.data()) );
    }
}
    
int PesFilter::process( Buffer pesBytes )
{
    if( pesBytes.isStart == Buffer::UNIT_START) {
        if( m_buffer.size()>0 )
            processPesPacket();
        
        m_mapBufferPos_StreamPos.clear();
        m_mapBufferPos_StreamPos[0] = pesBytes.posInTsStream;
        m_buffer.assign( (const char *)pesBytes.data, pesBytes.len );
    }else{
        m_mapBufferPos_StreamPos[ m_buffer.size() ] = pesBytes.posInTsStream;
        m_buffer.append((const char *)pesBytes.data, pesBytes.len);
    }
    
    return 0;
}

void PesFilter::setDownstreamFilter(EsFilter *filter)
{
    m_filter = std::auto_ptr<EsFilter>(filter);   
}

PesFilter::~PesFilter()
{
    std::cout <<  "~PesFilter: data remaining: " << m_buffer.size() << '\n';
    if( m_buffer.size() > 0 )
        processPesPacket();
}



}
