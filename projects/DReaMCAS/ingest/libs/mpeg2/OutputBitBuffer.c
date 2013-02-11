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
 * $(@)OutputBitBuffer.cc $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "OutputBitBuffer.h"

#include <cassert>
#include <iostream>

namespace  mpeg2ts {

unsigned char mask[] = { // 0 ... 32 bits
    0x00,      
    0x01,      0x03,      0x07,      0x0f,     0x1f,      0x3f,      0x7f,      0xff,
    0x1ff,     0x3ff,     0x7ff,     0xfff,    0x1fff,    0x3fff,    0x7fff,    0xffff,
    0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,  0x1fffff,  0x3fffff,  0x7fffff,  0xffffff, 
    0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};
    

bool OutputBitBuffer::writeBits( unsigned int numbits, unsigned int value)
{
    assert(numbits <= 8*sizeof(unsigned int));
    
    int availSpace = 8-m_writeBitPtr;
    while( numbits >= (availSpace) ) {
        if( m_writeBytePtr - m_data >= m_len )
            return false;        
        *m_writeBytePtr++ |= (value >> (numbits - availSpace)) & mask[availSpace];
        *m_writeBytePtr=0;
        numbits -= availSpace;
        m_writeBitPtr = 0;
        availSpace = 8;
    }
    
    assert( numbits < 8 );
    
    if( numbits > 0 ) {
        if( m_writeBytePtr - m_data >= m_len )
            return false;        
        assert( (8-m_writeBitPtr-numbits) >= 0 );
        *m_writeBytePtr |= (value & mask[numbits]) << (8-m_writeBitPtr-numbits);
        m_writeBitPtr += numbits;
    }
    
    return true;
}
    
    
    
}
