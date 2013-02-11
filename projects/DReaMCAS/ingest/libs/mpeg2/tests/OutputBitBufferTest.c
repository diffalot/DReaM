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
 * $(@)OutputBitBufferTest.cc $Revision: 1.1 $ $Date: 2006/07/15 00:08:38 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "mpeg2/OutputBitBuffer.h"
#include <cassert>
#include <iostream>

using namespace std;
using namespace mpeg2ts;

static void test8bitWrites();
static void test3bitWrites();
static void test11bitWrites();

int main(int argc, char *argv[])
{
    test8bitWrites();
    test3bitWrites();
    test11bitWrites();
    
    return 0;
}

static void test8bitWrites()
{
    unsigned char dest[3];
    
    OutputBitBuffer outb( dest, sizeof(dest) );   
    
    assert( outb.writeBits( 8, 0xff ) == true );
    assert( outb.writeBits( 8, 0x00 ) == true );
    assert( outb.writeBits( 8, 0xff ) == true );
    
    std::cout << "Output should be: FF 00 FF\n";
    std::cout << "test8bitWrites(): Output Bytes: " << std::hex << (int)dest[0]
        << " " << (int)dest[1] << " " << (int)dest[2] << std::dec << '\n';
}

static void test3bitWrites()
{
    unsigned char dest[3];
    
    OutputBitBuffer outb( dest, sizeof(dest) );
    
    assert( outb.writeBits( 3, 0x7 ) == true );
    assert( outb.writeBits( 3, 0x7 ) == true );
    assert( outb.writeBits( 3, 0x6 ) == true );
    assert( outb.writeBits( 3, 0x0 ) == true );
    assert( outb.writeBits( 3, 0x0 ) == true );
    assert( outb.writeBits( 3, 0x3 ) == true );
    assert( outb.writeBits( 3, 0x7 ) == true );
    assert( outb.writeBits( 3, 0x7 ) == true );

    std::cout << "Output should be: FF 00 FF\n";
    std::cout << "test3bitWrites(): Output Bytes: " << std::hex << '\n';
    for(int i=0; i<sizeof(dest); ++i )
    {
        std::cout << " " << (int)dest[i];
    }
    std::cout << std::dec << '\n';
}

static void test11bitWrites()
{
    unsigned char dest[5];
    
    OutputBitBuffer outb( dest, sizeof(dest) );
    
    assert( outb.writeBits( 11, 0x7ff ) );
    assert( outb.writeBits( 11, 0x00 ) );
    assert( outb.writeBits( 11, 0x7ff ) );
    assert( outb.writeBits( 7, 0x00 ) );

    std::cout << "Output should be: FF E0 03 FF 80\n";
    
    std::cout << "test11bitWrites(): Output Bytes: " << std::hex << '\n';
    for(int i=0; i<sizeof(dest); ++i )
    {
        std::cout << " " << (int)dest[i];
    }
    std::cout << std::dec << '\n';    
    
}

