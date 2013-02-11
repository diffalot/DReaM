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
 * $(@)PsiFilter.cc $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "InputBitBuffer.h"
#include "OutputBitBuffer.h"
#include "PsiFilter.h"
#include "Crc.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <iterator>

#define CRC32_LEN 4

namespace mpeg2ts {
    

InputBitBuffer& operator>>(InputBitBuffer& source, PatEntry& patEntry)
{
    source >> patEntry.programNumber;
    source >> patEntry.reserved;
    source >> patEntry.programMapPid;
    
    return source;
}

InputBitBuffer& operator>>(InputBitBuffer& source, PsiBase& psiBase)
{
    source >> psiBase.tableId;
    source >> psiBase.sectionSyntaxIndicator;
    source >> psiBase.privateIndicator;
    source >> psiBase.reserved2a;
    source >> psiBase.sectionLength;
    source >> psiBase.transportStreamId;
    source >> psiBase.reserved2b;
    source >> psiBase.versionNumber;
    source >> psiBase.currentNextIndicator;
    source >> psiBase.sectionNumber;
    source >> psiBase.lastSectionNumber;
    
    return source;
}

InputBitBuffer& operator>>(InputBitBuffer& source, Pmt& pmt)
{
    source >> pmt.tableId;
    source >> pmt.sectionSyntaxIndicator;
    source >> pmt.privateIndicator;
    source >> pmt.reserved2a;
    source >> pmt.sectionLength;
    source >> pmt.programNumber;
    source >> pmt.reserved2b;
    source >> pmt.versionNumber;
    source >> pmt.currentNextIndicator;
    source >> pmt.sectionNumber;
    source >> pmt.lastSectionNumber;
    source >> pmt.reserved3;
    source >> pmt.pcrPid;
    source >> pmt.reserved4;
    source >> pmt.programInfoLength;
    
    for( int i=0; i<pmt.programInfoLength.to_ulong(); ) 
    {
    DescriptorPtr desc = Descriptor::createDescriptor( source );
    i += desc->sizeInBytes();
    pmt.descriptors.push_back( desc );
    }

    return source;
}

InputBitBuffer& operator>>(InputBitBuffer& source, PmtEntry& pmtEntry)
{
    source >> pmtEntry.streamType;
    source >> pmtEntry.reserved3;
    source >> pmtEntry.elementaryPid;
    source >> pmtEntry.reserved4;
    source >> pmtEntry.esInfoLength;

    for( int i=0; i<pmtEntry.esInfoLength.to_ulong(); ){
        DescriptorPtr desc = Descriptor::createDescriptor( source );
    i += desc->sizeInBytes();
    pmtEntry.descriptors.push_back( desc );
    }
        
    return source;
}

OutputBitBuffer& operator<<( OutputBitBuffer& dest, PmtEntry& pmtEntry )
{
    pmtEntry.sizeInBytes(); // update size

    dest << pmtEntry.streamType << pmtEntry.reserved3 << pmtEntry.elementaryPid
         << pmtEntry.reserved4 << pmtEntry.esInfoLength;

    for( std::vector<DescriptorPtr>::iterator it=pmtEntry.descriptors.begin(); 
                it!=pmtEntry.descriptors.end(); ++it )
    (*it)->write(dest);

    return dest;
};

OutputBitBuffer& operator<<( OutputBitBuffer& dest, Pmt& pmt )
{
    pmt.sizeInBytes(); // update size

    dest << pmt.tableId << pmt.sectionSyntaxIndicator << pmt.privateIndicator
        << pmt.reserved2a << pmt.sectionLength << pmt.programNumber  
        << pmt.reserved2b << pmt.versionNumber << pmt.currentNextIndicator 
        << pmt.sectionNumber << pmt.lastSectionNumber << pmt.reserved3 
        << pmt.pcrPid << pmt.reserved4 << pmt.programInfoLength;
        
    for( std::vector<DescriptorPtr>::iterator it=pmt.descriptors.begin();
            it!=pmt.descriptors.end(); ++it )
    {
        (*it)->write(dest);
    }
    
    for( PmtEntryList::iterator it=pmt.entries.begin(); it!=pmt.entries.end(); ++it )
    {
        dest << (*it);
    }

    PairBytesBits pmtStrmSize = dest.dataAvailable();
          
    pmt.crc32 = crc32( dest.data(), pmtStrmSize.first );
    
    dest << pmt.crc32;
    
    return dest;
}
    

std::ostream& operator<<(std::ostream& out, PsiBase& psiBase)
{
    out << "\n\ttableId: " << psiBase.tableId.to_ulong()
        << "\n\tsection_syntax_indicator: " << psiBase.sectionSyntaxIndicator 
        << "\n\tprivate_indicator: " << psiBase.privateIndicator
        << "\n\treserved: " << psiBase.reserved2a
        << "\n\tsectionLength: " << psiBase.sectionLength.to_ulong()
        << "\n\ttransport_stream_id: " << psiBase.transportStreamId.to_ulong()
        << "\n\treserved: " << psiBase.reserved2b
        << "\n\tversion_number: " << psiBase.versionNumber.to_ulong()
        << "\n\tcurrent_next_indicator: " << psiBase.currentNextIndicator 
        << "\n\tsection_number: " << psiBase.sectionNumber.to_ulong()
        << "\n\tlast_section_number: " << psiBase.lastSectionNumber.to_ulong();
        
    return out;
}


std::ostream& operator<<(std::ostream& out, const Pmt& pmt)
{
    out << "\n\ttableId: " << pmt.tableId.to_ulong()
        << "\n\tsection_syntax_indicator: " << pmt.sectionSyntaxIndicator 
        << "\n\tprivate_indicator: " << pmt.privateIndicator
        << "\n\treserved: " << pmt.reserved2a
        << "\n\tsectionLength: " << pmt.sectionLength.to_ulong()
        << "\n\ttransport_stream_id: " << pmt.programNumber.to_ulong()
        << "\n\treserved: " << pmt.reserved2b
        << "\n\tversion_number: " << pmt.versionNumber.to_ulong()
        << "\n\tcurrent_next_indicator: " << pmt.currentNextIndicator 
        << "\n\tsection_number: " << pmt.sectionNumber.to_ulong()
        << "\n\tlast_section_number: " << pmt.lastSectionNumber.to_ulong()
        << "\n\treserved: " << pmt.reserved3
        << "\n\tPCR_PID: " << pmt.pcrPid.to_ulong()
        << "\n\treserved: " << pmt.reserved4
        << "\n\tprogram_info_length: " << pmt.programInfoLength;
        
    for( std::vector<DescriptorPtr>::const_iterator it=pmt.descriptors.begin();
         it != pmt.descriptors.end(); ++it )
    {
    out << "\n\t\tDescriptor Tag: " << (*it)->descriptorTag.to_ulong() 
        << " length: " << (*it)->descriptorLength.to_ulong() << '\n';
    }

    std::copy( pmt.entries.begin(), pmt.entries.end(),
               std::ostream_iterator<PmtEntry>( std::cout, "\n"));
        
    return out;
}


std::ostream& operator<<(std::ostream& out, const PatEntry& patEntry)
{
    out << "\n\t\tprogram_number: " << patEntry.programNumber.to_ulong()
        << "\n\t\treserved: " << patEntry.reserved
        << "\n\t\tprogram_map_PID: " << patEntry.programMapPid.to_ulong();
        
    return out;
}

std::ostream& operator<<(std::ostream& out, const Pat& pat)
{
    out << (PsiBase&) pat << '\n';
        
    std::copy( pat.entries.begin(), pat.entries.end(),
               std::ostream_iterator<PatEntry>( std::cout, "\n"));
        
    return out;
}

std::ostream& operator<<(std::ostream& out, const PmtEntry& pmtEntry)
{
    out << "\n\t\tstream_type: " << pmtEntry.streamType.to_ulong()
        << "\n\t\treserved: " << pmtEntry.reserved3
        << "\n\t\telementary_PID: " << pmtEntry.elementaryPid.to_ulong()
        << "\n\t\treserved: " << pmtEntry.reserved4
        << "\n\t\tES_info_length: " << pmtEntry.esInfoLength.to_ulong();

    std::vector<DescriptorPtr>::const_iterator it=pmtEntry.descriptors.begin();
    for(; it!=pmtEntry.descriptors.end(); ++it ) 
    {
    out << "\n\t\t\tDescriptor: Tag: " << (*it)->descriptorTag.to_ulong()
        << " length: "  << (*it)->descriptorLength.to_ulong();
    }

    return out;
}

static void collectSectionData(std::string& destination, unsigned char *srcData,
                               int len, Buffer::StartFlag isStart )
{
    if( isStart == Buffer::UNIT_START ) {
        int pointerField = *srcData++;
        --len;
        srcData += pointerField;
        len -= pointerField;
        destination = std::string((const char *)srcData, len);
    } else {
        destination.append((const char *)srcData, len);
    }
}

///////////////////// PAT Filter //////////////////////////////////////

int PatFilter::process(Buffer patbytes)
{
    assert( patbytes.pid==TsFilter::PatPid );
    
    collectSectionData(m_buffer, patbytes.data, patbytes.len, patbytes.isStart);

    PsiBase psi;

    assert( psi.sizeInBytes()==8 );
    
    if( m_buffer.size() >= psi.sizeInBytes() ) { 
        const unsigned char *sectionData = (const unsigned char *)m_buffer.data();
        int sectionDataLen = m_buffer.size();
        
        InputBitBuffer patbuf(sectionData, sectionDataLen);

        patbuf >> psi;
        
        if(psi.reserved2a.to_ulong() != 0x3 || psi.reserved2a.to_ulong() != 0x3)  {
            std::cout << "ERROR: PatFilter::process(): Reserved field in PSI != 0x3\n";
            return -1;
        }
        if( psi.sectionLength.to_ulong() > 1021 ) {
            std::cout << "ERROR: PatFilter::process(): PSI SectionLength exceeds max of 1021\n";
            return -1;
        }

        //std::cout << psi;
        
        // Do we have all of PAT data ?
        if( sectionDataLen >= (psi.sectionLength.to_ulong() +
                3 /* fields from tableId to sectionLength*/))
        {
             int bytesToParse = psi.sectionLength.to_ulong() 
                    - (psi.transportStreamId.size() + psi.reserved2b.size() + psi.versionNumber.size()
                        + psi.currentNextIndicator.size() + psi.sectionNumber.size()
                        + psi.lastSectionNumber.size() )/8
                    - CRC32_LEN;
             
             Pat  pat;
             
             while(bytesToParse > 0) {
                PatEntry patEntry;
                patbuf >> patEntry;
                if( patEntry.reserved != std::bitset<3>(0x7) ) {
                    std::cout << "ERROR: PatFilter::process(): Reserved field in PAT table != 0x7\n";
                    return -1;
                }
                
                pat.entries.push_back(patEntry);
                
                bytesToParse -= patEntry.sizeInBytes();
             }
             
             if( m_patCallback )
                 m_patCallback->operator()( pat, patbytes.pid );

        } // else return and try again when more data arrives.
    }
    return 0;
}

void PatFilter::setCallback( PatCallback *cb )
{
    m_patCallback = cb;
    
    return;
}

////////////////////////////// PMT Filter ////////////////////////////////////


int PmtFilter::process( Buffer pmtBytes )
{
    collectSectionData(m_buffer, pmtBytes.data, pmtBytes.len, pmtBytes.isStart);
    
    Pmt pmt;
    
    if( m_buffer.size() >= pmt.sizeInBytes() ) 
    {
        InputBitBuffer pmtbuf((const unsigned char *)m_buffer.data(), m_buffer.size());
        
        pmtbuf >> pmt;
        
        //std::cout << pmt;
        
        if(pmt.reserved2a != std::bitset<2>(0x3) 
          || pmt.reserved2b != std::bitset<2>(0x3)
          ||  pmt.reserved3 != std::bitset<3>(0x7)
          || pmt.reserved4  != std::bitset<4>(0xf)) 
    {
            std::cout << "ERROR: PmtFilter::process(): Incorrect Reserved fields\n";
            return -1;
        }
        
        if( m_buffer.size() >= pmt.sectionLength.to_ulong() + 3
        /*tableId to sectionLength*/) 
        {
            int bytesToParse = pmt.sectionLength.to_ulong() 
              - (pmt.sizeInBytes()-3) - pmt.programInfoLength.to_ulong() - CRC32_LEN;
              
            while( bytesToParse > 0 ) {
                PmtEntry pmtEntry;

                pmtbuf >> pmtEntry;
                pmt.entries.push_back( pmtEntry );
                bytesToParse -= pmtEntry.sizeInBytes();
            }
            
            if( m_pmtCallback )
                (*m_pmtCallback)( pmt, pmtBytes.pid );
        }
    }
    
    return 0;
}
    
void PmtFilter::setCallback(PmtCallback *cb)
{
    m_pmtCallback = cb;
}

int PmtEntry::sizeInBytes()
{
    int nbytes=0;
    for(std::vector<DescriptorPtr>::iterator it=descriptors.begin(); it!=descriptors.end(); ++it )
    {
    nbytes += (*it)->sizeInBytes();
    }

    esInfoLength = nbytes;
    return (streamType.size()+reserved3.size()+elementaryPid.size()
        +reserved4.size()+esInfoLength.size())/8 + esInfoLength.to_ulong();
}    

int Pmt::sizeInBytes()
{
    int nbytes = 0;
    for( std::vector<DescriptorPtr>::iterator it=descriptors.begin(); it!=descriptors.end(); ++it )
    {
        nbytes += (*it)->sizeInBytes();
    }

    programInfoLength = nbytes;

    for( std::vector<PmtEntry>::iterator it=entries.begin(); it!=entries.end(); ++it )
    {
        nbytes += (*it).sizeInBytes();
    }

    nbytes += (tableId.size() + sectionSyntaxIndicator.size() + privateIndicator.size()
                    + reserved2a.size() + sectionLength.size() + programNumber.size()
                    + reserved2b.size() + versionNumber.size() + currentNextIndicator.size()
                    + sectionNumber.size() + lastSectionNumber.size()
                    + reserved3.size() + pcrPid.size() + reserved4.size() 
                 + programInfoLength.size() + crc32.size())/8;

    sectionLength = nbytes - 3;

    return nbytes;
}

}
