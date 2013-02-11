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
 * $(@)InputBitBuffer.cc $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "InputBitBuffer.h"

#include <cassert>
#include <iostream>

namespace mpeg2ts {
 
bool InputBitBuffer::readBits(unsigned int numbits, unsigned int& retval)
{
    retval = 0;
    
    //    std::cout <<  "nbits: " << numbits << "bitp: " << m_readBitPtr << '\n';
    while( numbits >= (8-m_readBitPtr) ) {
        if( m_readBytePtr - m_data == m_len ){
	    std::cout << "readBits: numbits: " << numbits << " m_len: " << m_len
		      << '\n';
            return false;
	}
	//        std::cout <<  "in while: reading " << std::hex << (*m_readBytePtr << (numbits - (8-m_readBitPtr)))
	//            << std::dec << '\n';
        retval |= *m_readBytePtr << (numbits - (8-m_readBitPtr));
	//        std::cout << "in while: retval: " << std::hex << retval << std::dec << '\n';
        numbits -= 8-m_readBitPtr;
        ++m_readBytePtr;
        m_readBitPtr = 0;
    }
    
    assert( m_readBitPtr < 8 );
    
    if( numbits > 0 ) {
        if( m_readBytePtr - m_data == m_len )
            return false;            
	//        std::cout << "out while: reading " << std::hex << (*m_readBytePtr >> ((8-m_readBitPtr) - numbits))
	//            << std::dec << '\n';
        retval |= *m_readBytePtr >> ((8-m_readBitPtr) - numbits);
	//        std::cout << "out while: retval: " << std::hex << retval << std::dec << '\n';
           m_readBitPtr += numbits;
    }
    
    return true;
}
    
    
}
