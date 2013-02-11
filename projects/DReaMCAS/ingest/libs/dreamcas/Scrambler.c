
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
 * $(@)Scrambler.cc $Revision: 1.1 $ $Date: 2006/07/15 00:26:54 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "Scrambler.h"
#include "dreamcas/Ecm.h"

#include <stdio.h>

#include "ca_crypto/crypto.h"
#include "dreamcas/EcmGenerator.h"

#include <cstring>
#include <iostream>

namespace dream {
    namespace cas {
        
static void counterAddUInt32( uint8_t *op1, uint32_t op_size, uint32_t op2);

ScramblerFilter::ScramblerFilter(m2t::FilterPtr outFilter, EcmGeneratorPtr ecmGenerator)
        : m_outFilter(outFilter), m_keyProvider(ecmGenerator),
          m_encryptedPackets(0), m_totalPacketsProcessed(0)
{        
}

ScramblerFilter::~ScramblerFilter()
{
    std::cout << "ScramblerFilter: # TS Packets encrypted: " << m_encryptedPackets
        << "/" << m_totalPacketsProcessed << std::endl;
}

int ScramblerFilter::process( m2t::Buffer buffer )
{
    unsigned char streamIv[16]; // todo: Initialize Scrambler with IV size
    
    assert( buffer.hdrBegin[0]==0x47 );
    

    int continuityCounter=buffer.hdrBegin[3] & 0x0f;
    
    const Ecm ecm =  m_keyProvider->getCurrentEcm();
    std::memcpy( streamIv, ecm.ivData.data(), ecm.ivData.size() );
    counterAddUInt32(streamIv, sizeof(streamIv), continuityCounter*12);
  
    m_totalPacketsProcessed++;
    
    // turn on scrambling control bit
    if( ecm.streamKeyLength.to_ulong() > 0 )
        buffer.hdrBegin[3] |= 0x40;
    
    if( buffer.len > 0 && ecm.streamKeyLength.to_ulong() > 0 ) 
    {
        encryptBufferData( buffer.data, buffer.len, buffer.data, buffer.len,
                          (unsigned char *)ecm.streamKeyData.data(), 
                          ecm.streamKeyData.size(),
                          streamIv, sizeof(streamIv),
                          MODE_CTR /* todo: remove this hard-coded aes_mode */ );
                
        m_encryptedPackets++;
    }
                       
    return m_outFilter->process(buffer);   
}
   
static void counterAddUInt32( uint8_t *op1, uint32_t op_size, uint32_t op2)
{
    uint32_t len;
    uint8_t  op2_8; 
    
    len = op_size - 1;
    op2_8 = op2 & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 2;
    op2_8 = (op2 >> 8 )& 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 3;
    op2_8 = (op2 >> 16 ) & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }

    len = op_size - 4;
    op2_8 = (op2 >> 24 ) & 0xFF;
    while ( len >= 0 )
    {
        op1[len] = op1[len] + op2_8;
    	if ( op1[len] )
    		break;
    	len--;
    }
    
    return;  	
}

    }
}
