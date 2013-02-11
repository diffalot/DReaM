
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
 * $(@)Ecm.cc $Revision: 1.1 $ $Date: 2006/07/15 00:26:51 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "dreamcas/Ecm.h"

#include <iostream>

namespace dream {
    namespace cas {

        
namespace m2t=mpeg2ts;

m2t::OutputBitBuffer& operator<<(m2t::OutputBitBuffer& dest, const Ecm& ecm)
{
    dest << ecm.streamKeyLength << ecm.ivLength << ecm.streamKeyData << ecm.ivData;
    
    return dest;
}

m2t::OutputBitBuffer& operator<<(m2t::OutputBitBuffer& dest, 
                                 const EcmSection& ecmSection)
{
	dest << (const CaMessageSection&) ecmSection;

    for(std::vector<std::string>::const_iterator it=ecmSection.encryptedEcms.begin();
            it != ecmSection.encryptedEcms.end(); ++it )
    {
        dest << (*it);
    }
    
    return dest;
}



    }
}
