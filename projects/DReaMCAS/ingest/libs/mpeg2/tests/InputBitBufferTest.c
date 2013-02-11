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
 * $(@)InputBitBufferTest.cc $Revision: 1.1 $ $Date: 2006/07/15 00:08:37 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
// InputBitBufferTest.cc

#include "mpeg2/InputBitBuffer.h"
#include <iostream>
#include <cassert>

namespace m2t=mpeg2ts;

static void test8bitGets();

int main( int argc, char *argv[] )
{
    unsigned char data[]={ 0x00, 0x01, 0xff, 0xfe, 0xf0 };
 
#if 0
    m2t::InputBitBuffer bitsrc( data, sizeof(data) );
    
    
    // Test getting < 8 bits at a time
    std::cout << "Getting 3 bits\n";
    std::bitset<3> bits3;
    bitsrc = m2t::InputBitBuffer( data, sizeof(data) );
    for( int i=0; i<sizeof(data)*8; i+=3 ) {
        bitsrc >> bits3;
        std::cout << bits3 << std::endl;
    }
    
    // Get > 8 bits at a time
    std::cout << "Getting 11 bits\n";
    bitsrc = m2t::InputBitBuffer(data, sizeof(data));
    std::bitset<11> bits11;
    for( int i=0; i<sizeof(data)*8; i+= 11 ) {
        std::cout << "i=" << i << "\n";
        bitsrc >> bits11;
        std::cout << bits11 << std::endl;
    }
#endif    
    test8bitGets();
    
	return 0;
}

static void test8bitGets()
{
    unsigned char data[]={ 0xff, 0x00, 0xff };
    
    m2t::InputBitBuffer bits( data, sizeof(data) );
    
    unsigned int val1, val2, val3;
    
    bits.readBits( 8, val1 );
    bits.readBits( 8, val2 );
    bits.readBits( 8, val3 );
    
    std::cout <<  "test8bitGets(): Vals=" << std::hex << val1 << " " << val2
        << " " << val3 << std::dec << '\n';

    assert( val1==data[0] );
    assert( val2==data[1] );
    assert( val3==data[2] );
    
    bits = m2t::InputBitBuffer(data, sizeof(data));
    
    std::bitset<8> bset1, bset2, bset3;
    
    bits >> bset1 >> bset2 >> bset3;
    
    std::cout << "test8bitGets(): BitSets=" << std::hex << bset1 << " " << bset2
        << " " << bset3 << '\n';
        
    assert( bset1.to_ulong() == data[0] );
    assert( bset2.to_ulong() == data[1] );
    assert( bset3.to_ulong() == data[2] );
    
}
