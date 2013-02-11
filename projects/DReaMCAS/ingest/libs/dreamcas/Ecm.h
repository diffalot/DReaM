
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
 * $(@)Ecm.h $Revision: 1.2 $ $Date: 2006/07/25 22:39:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef DREAM_CAS_ECM_H
#define DREAM_CAS_ECM_H

#include "mpeg2/OutputBitBuffer.h"
#include "dreamcas/Emm.h"

#include <boost/shared_ptr.hpp>

namespace dream{
    namespace cas {
        
struct Ecm
{
    Ecm(){}
    Ecm(std::string key, std::string iv)
        : streamKeyLength(key.size()), ivLength(iv.size()), streamKeyData(key),
          ivData(iv)
    {}
    enum { SIZE = 1 + 1 + 16 + 16 };  
    std::bitset<8> streamKeyLength;
    std::bitset<8> ivLength;
    std::string streamKeyData;
    std::string ivData;
    int sizeInBytes() const { return (streamKeyLength.size() + ivLength.size())/8
                                        + streamKeyData.size() + ivData.size(); }
};

mpeg2ts::OutputBitBuffer& operator<<(mpeg2ts::OutputBitBuffer&, const Ecm&);

struct EcmSection: public CaMessageSection
{
    EcmSection(): CaMessageSection(0x80) {}
    std::vector<Ecm> ecms;
	std::vector<std::string> encryptedEcms;
};

mpeg2ts::OutputBitBuffer& operator<<( mpeg2ts::OutputBitBuffer&, const EcmSection& );

    }
}

#endif
