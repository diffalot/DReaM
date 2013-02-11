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
 * $(@)Filter.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:40 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_FILTER_H
#define MPEG2TS_FILTER_H


#include "InputBitBuffer.h"

#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>


namespace mpeg2ts {
    
/**
 * hdrBegin points to the beginning of the packet header's first byte. The 'data'
 * field points to the payload's first byte. 'len' is the payload length.
 */
struct Buffer
{
    enum StartFlag { UNIT_BODY, UNIT_START };
    Buffer( unsigned char *pdata, int datalen, unsigned char *phdr=NULL, long long startPos=0, 
        StartFlag start = UNIT_START, unsigned int PID=0x000F)
            : data(pdata), len(datalen), hdrBegin(phdr), isStart(start),
            posInTsStream(startPos), pid(PID)
    {}        

    unsigned char *data;
    unsigned char *hdrBegin;
    int        len;
    StartFlag      isStart;
    
    // Metadata
    long long      posInTsStream;
    
    unsigned int pid;
};

class Filter;
typedef boost::shared_ptr<Filter> FilterPtr;
typedef std::vector<FilterPtr> FilterPtrList;

class Filter{
public:
    enum 
    { 
        PatTid = 0,
        CaTid=1,
        PmtTid = 2,
        UserPrivateTidStart=0x40,
        UserPrivateTidEnd=0x4E
    };
    
    enum Logging
    { 
        LOG_NONE=0, 
        LOG_HEADER=1 
    };
    
    
    virtual ~Filter(){}
    
    virtual int process(Buffer data)=0;
};

struct TsPacket{
    unsigned char syncByte;
    unsigned char transportErrorIndicator;
    unsigned char payloadUnitStartIndicator;
    unsigned char transportPriority;
    unsigned int  pid;
    unsigned char transportScramblingControl;
    unsigned char adaptationFieldControl;
    unsigned char continuityCounter;
    
    // 'adaptationFieldLength' is valid only if adaptationFieldControl 
    // has the value: AfcAdaptationOnly or AfcAdaptationPayload

    unsigned char adaptationFieldLength; 
    
    enum { AfcReserved=0,
       AfcPayloadOnly=1,
       AfcAdaptationOnly=2,
       AfcAdaptationPayload=3 };
};

struct AdaptationField 
{
    std::bitset<1> discontinuityIndicator;
    std::bitset<1> randomAccessIndicator;
    std::bitset<1> elementaryStreamPriorityIndicator;
    std::bitset<1> pcrFlag;
    std::bitset<1> opcrFlag;
    std::bitset<1> splicingPointFlag;
    std::bitset<1> transportPrivateDataFlag;
    std::bitset<1> adaptationFieldExtensionFlag;
};

int parseTsPkt( unsigned char *data, int len, struct TsPacket& tspkt);
std::ostream& operator<<(std::ostream& out, TsPacket& tspkt);

class TsFilter : public Filter {
public:
    enum { PktSize=	188 };
    enum { PatPid = 0, CatPid=1, NullPid=0x1fff };
    
    TsFilter(Logging log=LOG_NONE);
    
    virtual int process(Buffer tsdata);
    
    int setFilter(int pid, FilterPtr filter);
    
private:
    std::multimap<int, FilterPtr> filters;
    long long m_curpos;
    Logging m_log;
};


}

namespace m2t=mpeg2ts;

#endif
