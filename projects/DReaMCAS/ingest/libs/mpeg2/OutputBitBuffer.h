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
 * $(@)OutputBitBuffer.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:41 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_OUTPUT_BIT_BUFFER_H
#define MPEG2TS_OUTPUT_BIT_BUFFER_H

#include <stdlib.h>

#include <cassert>
#include <bitset>

namespace mpeg2ts {
    
    
class OutputBitBuffer
{
 public:
	OutputBitBuffer(unsigned char *data, unsigned int len):
        m_data(data), m_len(len), m_writeBytePtr(data), m_writeBitPtr(0)
    { *m_writeBytePtr=0; }	
	
    virtual ~OutputBitBuffer(){}
	
    /*
     * write 'numbits' least significant bits from 'value'
     */
    bool writeBits(unsigned int numbits, unsigned int value);

    const unsigned char *data() const { return m_data; }
    
    /** get the number of Bytes and Bits of data present (not empty
        space ) in BitBuffer */
        
    std::pair<int, int> dataAvailable() const {
	return std::make_pair(m_writeBytePtr + m_writeBitPtr/8 - m_data, 
			      m_writeBitPtr%8);
    }
    
 private:
    unsigned char *m_data;
    unsigned int m_len;
    unsigned char *m_writeBytePtr;
    int m_writeBitPtr;
};


template <size_t NBITS> /* NBITS must be <= 32 */
OutputBitBuffer& operator<<(OutputBitBuffer& dest, const std::bitset<NBITS>& value)
{
    if(dest.writeBits(NBITS, value.to_ulong())) {
        return dest;
    }else{ // TODO: throw exception here ?
        assert(0);
        NULL;
    }
    
    return dest;
}

inline OutputBitBuffer& operator<<(OutputBitBuffer& dest, const std::string& value)
{
    for( int i=0; i<value.size(); ++i )
    {
        if(!dest.writeBits( 8, value[i] )) { 
            assert(0);
            NULL;
        }
    }
     
    return dest;
}

}

#endif
