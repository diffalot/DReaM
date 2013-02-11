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
 * $(@)PsiFilter.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_PSI_FILTER_H
#define MPEG2TS_PSI_FILTER_H


#include "Filter.h"
#include "Descriptors.h"
#include <string>
#include <vector>
#include <bitset>
#include <functional>

namespace mpeg2ts {


struct PatEntry 
{
    std::bitset<16> programNumber;
    std::bitset<3>  reserved;
    std::bitset<13> programMapPid;
    int sizeInBytes(){
    return (programNumber.size() + reserved.size() + programMapPid.size())/8;
    }
};


struct PmtEntry 
{
    PmtEntry(){}
    
    PmtEntry( uint8_t streamtype, uint16_t elementarypid )
    :streamType( streamtype ), elementaryPid(elementarypid), reserved3(0xff),
    reserved4(0xff), esInfoLength(0)
    {}
    
    std::bitset<8> streamType;
    std::bitset<3> reserved3;
    std::bitset<13> elementaryPid;
    std::bitset<4> reserved4;
    std::bitset<12> esInfoLength;
    std::vector<DescriptorPtr>  descriptors;

    int sizeInBytes();

};

struct PsiBase
{
    std::bitset<8> tableId;
    std::bitset<1> sectionSyntaxIndicator;
    std::bitset<1> privateIndicator;
    std::bitset<2> reserved2a;
    std::bitset<12> sectionLength;
    std::bitset<16> transportStreamId;
    std::bitset<2> reserved2b;
    std::bitset<5> versionNumber;
    std::bitset<1> currentNextIndicator;
    std::bitset<8> sectionNumber;
    std::bitset<8> lastSectionNumber;
    int sizeInBytes(){ 
    return (tableId.size() + sectionSyntaxIndicator.size() + privateIndicator.size()
            + reserved2a.size() + sectionLength.size() + transportStreamId.size()
            + reserved2b.size() + versionNumber.size() + currentNextIndicator.size()
            + sectionNumber.size() + lastSectionNumber.size() )/8;
    }
            
};

class Pmt;

typedef std::vector<PatEntry> PatEntryList;
typedef std::vector<PmtEntry> PmtEntryList;
typedef std::vector<Pmt>      PmtList;

struct Pmt
{
    Pmt(){}
    
    Pmt( int programNumbr, int pcrPID )
    :tableId( Filter::PmtTid ), programNumber(programNumbr),
    sectionSyntaxIndicator(1), privateIndicator(0), sectionNumber(0),
    lastSectionNumber(0), reserved2a(0xff), reserved2b(0xff), versionNumber(1),
    currentNextIndicator(1), reserved3(0xff), reserved4(0xff),
    programInfoLength(0), pcrPid(pcrPID)
    {}
    
    std::bitset<8> tableId;
    std::bitset<1> sectionSyntaxIndicator;
    std::bitset<1> privateIndicator;
    std::bitset<2> reserved2a;
    std::bitset<12> sectionLength;
    std::bitset<16> programNumber;
    std::bitset<2> reserved2b;
    std::bitset<5> versionNumber;
    std::bitset<1> currentNextIndicator;
    std::bitset<8> sectionNumber;
    std::bitset<8> lastSectionNumber;
    std::bitset<3> reserved3;
    std::bitset<13> pcrPid;
    std::bitset<4> reserved4;
    std::bitset<12> programInfoLength;
    std::vector<DescriptorPtr>  descriptors;
    PmtEntryList    entries;
    std::bitset<32> crc32;
    int sizeInBytes();
};

struct Pat : public PsiBase
{
    PatEntryList entries;
};

struct ConditionalAccessSection
{
    std::bitset<8> tableId;
    std::bitset<1> sectionSyntaxIndicator;
    std::bitset<1> privateIndicator;
    std::bitset<2> reserved2;
    std::bitset<12> sectionLength;
    std::bitset<18> reserved18;
    std::bitset<5> versionNumber;
    std::bitset<1> currentNextIndicator;
    std::bitset<8> sectionNumber;
    std::bitset<8> lastSectionNumber;
    int sizeInBytes(){ 
    return (tableId.size() + sectionSyntaxIndicator.size() + privateIndicator.size()
            + reserved2.size() + sectionLength.size() + reserved18.size()
            + versionNumber.size() + currentNextIndicator.size()
            + sectionNumber.size() + lastSectionNumber.size() )/8;
    }
};

class OutputBitBuffer;
OutputBitBuffer& operator<<( OutputBitBuffer& dest, Pmt& pmt );
OutputBitBuffer& operator<<( OutputBitBuffer& dest, PmtEntry& pmtEntry );

std::ostream& operator<<(std::ostream& out, const Pat& pat);
std::ostream& operator<<(std::ostream& out, const PatEntry& patEntry);
std::ostream& operator<<(std::ostream& out, const Pmt& pmt);
std::ostream& operator<<(std::ostream& out, const PmtEntry& pmtEntry);


template<typename ARG1>
class Callback
{
public:
    virtual void operator()(ARG1 arg, unsigned int pid /*PAT or PMT Pid*/) = 0;
};

typedef Callback< Pat > PatCallback;
typedef Callback< Pmt > PmtCallback;


class PatFilter : public Filter
{
 public:
    PatFilter() : m_patCallback(NULL), m_filter(NULL){}
    virtual int process(Buffer buffer);
    void setCallback( PatCallback *cb );
 
 private:
    std::string m_buffer;
    PatCallback *m_patCallback;
    Filter      *m_filter;
};

class PmtFilter : public Filter
{
public:
    enum StreamType { MPEG1_VIDEO=0x01, MPEG2_VIDEO=0x02, MPEG1_AUDIO=0x03,
              MPEG2_AUDIO=0x04, PRIVATE_SECTIONS=0x05,
              PES_PRIVATE_SECTIONS=0x06, 
              USER_PRIVATE_START=0x80, USER_PRIVATE_END=0xFF };
public:
    PmtFilter(){} 
    virtual int process( Buffer pmtBytes );
    void setCallback(PmtCallback *cb);

private:
    std::string m_buffer;
    PmtCallback *m_pmtCallback;
};

typedef boost::shared_ptr<mpeg2ts::PatFilter> PatFilterPtr;
typedef boost::shared_ptr<mpeg2ts::PmtFilter> PmtFilterPtr;

}

#endif
