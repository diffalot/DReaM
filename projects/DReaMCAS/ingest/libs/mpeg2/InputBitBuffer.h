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
 * $(@)InputBitBuffer.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */


 
#ifndef MPEG2TS_INPUT_BIT_BUFFER_H
#define MPEG2TS_INPUT_BIT_BUFFER_H

#include <stdlib.h>

#include <cassert>
#include <bitset>
#include <utility>

namespace mpeg2ts {
    
typedef std::pair<int, int> PairBytesBits;

/**
 * InputBitBuffer is based on the C/C++ Users Journal, Oct '05 article:
 *   'Bitstream parsing in C++'
 *
 * The Bit buffer is initialized with a bunch of bytes. The readBits() method
 * is used to read 'n' bits at a time from the bit buffer.
 * A stream operator>>() helper method  allows iostream-like parsing of the data.
 *
 *  InputBitBuffer   inbits( data, 32 );
 *  std::bitset<3>   bits3;
 *  
 *  inbits >> bits3;
 */
 
class InputBitBuffer
{
 public:
	InputBitBuffer(const unsigned char *data, unsigned int len):
        m_data(data), m_len(len), m_readBytePtr(data), m_readBitPtr(0) { }	
	
    virtual ~InputBitBuffer(){}
	
    bool readBits(unsigned int numbits, unsigned int& retval);
    
    PairBytesBits dataConsumed(){
        return std::make_pair( m_readBytePtr + m_readBitPtr/8 - m_data, 
                               m_readBitPtr%8); 
    }
    
 private:
    const unsigned char *m_data;
    unsigned int m_len;
    const unsigned char *m_readBytePtr;
    int m_readBitPtr;
};

template <size_t NBITS> /* NBITS must be <= 32 */
InputBitBuffer& operator>>(InputBitBuffer& source, std::bitset<NBITS>& dest)
{
    unsigned int tmp;
    if(source.readBits(NBITS, tmp)) {
        dest = tmp;
    }else{ // TODO: throw exception here ?
        assert(0);
        NULL;
    }
    
    return source;
}

}


#endif
