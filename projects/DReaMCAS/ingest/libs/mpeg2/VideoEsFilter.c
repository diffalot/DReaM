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
 * $(@)VideoEsFilter.cc $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "VideoEsFilter.h"
#include <iostream>

namespace mpeg2ts
{


int VideoEsFilter::process( PesBuffer pespkt )
{
    const unsigned char *data = pespkt.data;
    
    for( int i=0; i<pespkt.len-3; ++i ) {
        if( data[0]==0 && data[1]==0 && data[2]==1 && data[3]==0 ) {// picture header
            MapBufferPosStreamPos::const_iterator it
            = pespkt.mapBposSpos.upper_bound(data - pespkt.pesHdrStart);
            
            --it;
            assert( it != pespkt.mapBposSpos.end() );
            
            std::cout << "PICTURE_START_CODE " << (data[5]>>3 & 0x7) 
            << " at " <<  (*it).second + (data - pespkt.pesHdrStart) - (*it).first
            << '\n';
        }
        ++data;
    }
    return 0;    
}




}
