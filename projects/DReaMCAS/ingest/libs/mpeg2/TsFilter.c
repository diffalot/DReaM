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
 * $(@)TsFilter.cc $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "Filter.h"
#include "InputBitBuffer.h"

#include <string.h>

#include <cassert>
#include <bitset>
#include <iostream>
#include <ios>


namespace mpeg2ts {



std::ostream& operator<<(std::ostream& out, TsPacket& tspkt)
{
    out << "\nsync_byte: 0x" << std::hex << (int)tspkt.syncByte << std::dec
    << "\ntransport_error_indicator: " << (int)tspkt.transportErrorIndicator
    << "\npayload_unit_start_indicator: " << (int)tspkt.payloadUnitStartIndicator
    << "\ntransport_priority: " << (int)tspkt.transportPriority
    << "\nPID: " << (int)tspkt.pid 
    << "\ntransport_scrambling_control: " << (int)tspkt.transportScramblingControl
    << "\nadaptation_field_control: " << (int)tspkt.adaptationFieldControl
    << "\ncontinuity_counter: " << (int)tspkt.continuityCounter;

    if( tspkt.adaptationFieldControl==TsPacket::AfcAdaptationOnly
	|| tspkt.adaptationFieldControl==TsPacket::AfcAdaptationPayload )
    {
	out << "\nadaptation_field_length: " << (int)tspkt.adaptationFieldLength;
    }
     
    out << "\n";

    return out;
}


int parseTsPkt( unsigned char *data, int len, struct TsPacket& tspkt)
{
    unsigned char *p = data;
    tspkt.syncByte = *p++;
    tspkt.transportErrorIndicator = (*p>>7) & 0x1;
    tspkt.payloadUnitStartIndicator = (*p>>6) & 0x1;
    tspkt.transportPriority = (*p>>5) & 0x1;
    tspkt.pid =  (*p & 0x1f) << 8 |  *(p+1) ;
    p += 2;
    tspkt.transportScramblingControl = (*p >> 6) & 0x3;
    tspkt.adaptationFieldControl = (*p >> 4) & 0x3;
    tspkt.continuityCounter = *p & 0xf;
    p += 1;
    
    if( tspkt.adaptationFieldControl == 2 || tspkt.adaptationFieldControl == 3 )
    {
    tspkt.adaptationFieldLength = *p;
    p += (1 + tspkt.adaptationFieldLength);
    if( tspkt.adaptationFieldControl==0x2 )
        assert( tspkt.adaptationFieldLength == 183 );
    assert( (p-data) <= len );
    }
    return p-data;
}


TsFilter::TsFilter(Logging log)
    : m_curpos( 0LL ), m_log(log)
{
}

int TsFilter::process( Buffer buffer )
{
    assert( buffer.data[0] == 0x47 );
    assert( buffer.len == 188 );
    
    TsPacket tspkt;
    int consumed = parseTsPkt( buffer.data, buffer.len, tspkt );
    
    if( m_log==LOG_HEADER )
        std::cout << tspkt << '\n';
    
    assert(consumed>=4);
    
    typedef std::multimap<int, FilterPtr>::iterator iterator;
    
    // Get all Filters for processing current packet's pid.
    std::pair<iterator, iterator> pidFilters= filters.equal_range(tspkt.pid);
    
    m_curpos += consumed;
    
    for( ; pidFilters.first!=pidFilters.second; ++pidFilters.first )
    {
        (*pidFilters.first).second->process( Buffer(buffer.data+consumed, 
                                                    buffer.len-consumed,
                                                    buffer.data, m_curpos,
                      Buffer::StartFlag(tspkt.payloadUnitStartIndicator), 
                                                     tspkt.pid) );
    }
    
    m_curpos += buffer.len - consumed;

    return 0;
}


int TsFilter::setFilter(int pid, FilterPtr filter)
{
/*     if( filters[pid] )
    filters[pid]->push_back(filter); */
    
    filters.insert(std::multimap<int, FilterPtr>::value_type(pid, filter));
    
    return 0;
}

    
}
