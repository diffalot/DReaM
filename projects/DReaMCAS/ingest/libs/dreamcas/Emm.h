
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
 * $(@)Emm.h $Revision: 1.2 $ $Date: 2006/07/25 22:39:11 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef MPEG2TS_EMM_H
#define MPEG2TS_EMM_H

#include "mpeg2/InputBitBuffer.h"
#include "mpeg2/OutputBitBuffer.h"

#include<bitset>
#include<vector>

namespace dream {
    namespace cas {
    
struct Emm
{
    Emm(){}
    Emm( int crypto_mode, std::string key, std::string iv )
        : cryptoMode(crypto_mode), entitlementTokenData(key),
        entitlementTokenLength(key.size()), seedData( iv ), seedLength(iv.size())
    {}
    
    enum { SIZE = 35 }; // 1 + 1 + 1 + 16 + 16
    std::bitset<8> cryptoMode;
    std::bitset<8> entitlementTokenLength;
    std::bitset<8> seedLength;
    std::string entitlementTokenData;
    std::string seedData;
    int sizeInBytes() const { return 3 + entitlementTokenData.size() + seedData.size(); }
};

struct CaMessageSection
{
    CaMessageSection(){}
	CaMessageSection(unsigned int tableid) 
        : tableId(tableid), sectionSyntaxIndicator(0), dvbReserved(0), isoReserved(0xf)
    {}
    std::bitset<8> tableId;
    std::bitset<1> sectionSyntaxIndicator;
    std::bitset<1> dvbReserved;
    std::bitset<2> isoReserved;
    std::bitset<12> sectionLength;
};

struct EmmSection: public CaMessageSection
{
   
	EmmSection(): CaMessageSection(0x82){}
    std::vector<Emm> emms;
};


std::ostream& operator<<(std::ostream& out, const Emm& emm);
std::ostream& operator<<(std::ostream& out, const EmmSection& emmSection);

mpeg2ts::InputBitBuffer& operator>>(mpeg2ts::InputBitBuffer& source, Emm& emm);
mpeg2ts::InputBitBuffer& operator>>(mpeg2ts::InputBitBuffer& source, EmmSection& emmSection);

mpeg2ts::OutputBitBuffer& operator<<( mpeg2ts::OutputBitBuffer& dest, 
                                      const CaMessageSection& caSection);
                                      
mpeg2ts::OutputBitBuffer& operator<<( mpeg2ts::OutputBitBuffer& dest, const Emm& emm );
mpeg2ts::OutputBitBuffer& operator<<( mpeg2ts::OutputBitBuffer& dest, 
                                      EmmSection& emmSection );
}
}

#endif
