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
 * $(@)PesFilter.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_PES_FILTER_H
#define MPEG2TS_PES_FILTER_H

#include "Filter.h"

#include <cassert>
#include <string>
#include <map>

namespace mpeg2ts {

typedef std::map<long, long long> MapBufferPosStreamPos;

struct PesBuffer : public Buffer
{
   // PesBuffer(unsigned char *data, int len, MapBufferPosStreamPos& mappos, const unsigned char *hdrStart)
   PesBuffer( unsigned char *data, int len, const MapBufferPosStreamPos& mappos, const unsigned char *hdrStart)
   : Buffer(data, len), mapBposSpos(mappos), pesHdrStart(hdrStart){}

   const MapBufferPosStreamPos& mapBposSpos;
   const unsigned char *        pesHdrStart;
};

class EsFilter
{
public:
    virtual int process(PesBuffer pesData)=0;
};

class PesFilter : public Filter
{
 public:
    PesFilter(){m_buffer.reserve(100*1024);}
    virtual ~PesFilter();
    
    void setDownstreamFilter( EsFilter *filter );
    
    virtual int process( Buffer pesBytes );
    virtual void processPesPacket() const;
    
private:
    std::string m_buffer;
    MapBufferPosStreamPos m_mapBufferPos_StreamPos;
    std::auto_ptr<EsFilter> m_filter;
};


}

#endif
