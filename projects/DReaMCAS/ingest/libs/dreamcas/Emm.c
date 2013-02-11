
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
 * $(@)Emm.cc $Revision: 1.2 $ $Date: 2006/07/25 22:39:10 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "dreamcas/Emm.h"

#include <iostream>

namespace m2t=mpeg2ts;

namespace dream{
    namespace cas{

std::ostream& operator<<(std::ostream& out, const Emm& emm)
{
    out << "\n\tcrypto_mode: 0x" << std::hex << emm.cryptoMode.to_ulong() << std::dec
        << "\n\tentitlement_token_length: " << emm.entitlementTokenLength.to_ulong()
        << "\n\tseed_length: " << emm.seedLength.to_ulong() 
        << '\n';
        
    return out;
}


std::ostream& operator<<(std::ostream& out, const EmmSection& emmSection)
{
    out << "\n\ttable_id: 0x" << std::hex << emmSection.tableId.to_ulong() << std::dec 
        << "\n\tsection_syntax_indicator: " << emmSection.sectionSyntaxIndicator
        << "\n\tDVB_reserved: " << emmSection.dvbReserved
        << "\n\tISO_reserved: " << emmSection.isoReserved
        << "\n\tCA_section_length: " << emmSection.sectionLength.to_ulong();

    for( std::vector<Emm>::const_iterator it=emmSection.emms.begin(); 
            it!=emmSection.emms.end(); ++it )
    {
        out << *it << '\n';
    }
    
    return out;
}

m2t::InputBitBuffer& operator>>(m2t::InputBitBuffer& source, Emm& emm)
{
    source >> emm.cryptoMode >> emm.entitlementTokenLength
           >> emm.seedLength;
           
    for(int i=0; i<emm.entitlementTokenLength.to_ulong(); ++i)
    {
        unsigned int byteval;
        source.readBits( 8, byteval );
        emm.entitlementTokenData.push_back(byteval);
    }
    
    for(int i=0; i<emm.seedLength.to_ulong(); ++i)
    {
        unsigned int byteval;
        source.readBits( 8, byteval );
        emm.seedData.push_back( byteval );
    }
    
    return source;
}

m2t::InputBitBuffer& operator>>(m2t::InputBitBuffer& source, EmmSection& emmSection)
{
    source >> emmSection.tableId >> emmSection.sectionSyntaxIndicator
           >> emmSection.dvbReserved >> emmSection.isoReserved 
           >> emmSection.sectionLength;
           
    std::cout << "EmmSection::sectionLength = " 
              << emmSection.sectionLength.to_ulong() << '\n';
              
    int bytesToParse = emmSection.sectionLength.to_ulong();
    
    while( bytesToParse > 0 ) 
    {
        Emm emm;
        source >> emm;
        emmSection.emms.push_back(emm);
        bytesToParse -= emm.sizeInBytes();
    }
    
    return source;
}

m2t::OutputBitBuffer& operator<<( m2t::OutputBitBuffer& dest, const Emm& emm )
{
    dest << emm.cryptoMode << emm.entitlementTokenLength << emm.seedLength
         << emm.entitlementTokenData << emm.seedData;
         
    return dest;
}   

m2t::OutputBitBuffer& operator<<(m2t::OutputBitBuffer& dest, 
                                 const CaMessageSection& caSection)
{    
    dest << caSection.tableId << caSection.sectionSyntaxIndicator
         << caSection.dvbReserved << caSection.isoReserved << caSection.sectionLength;
         
    return dest;			
} 

m2t::OutputBitBuffer& operator<<( m2t::OutputBitBuffer& dest, 
                                  EmmSection& emmSection )
{

    int nbytes = 0;
    
    for( std::vector<Emm>::const_iterator it=emmSection.emms.begin(); 
            it != emmSection.emms.end(); ++it ) 
    {
        nbytes += (*it).sizeInBytes();
    }

	std::cout << "Size of EMM Section: " << nbytes << std::endl;
	
	emmSection.sectionLength = nbytes;

    dest << (const CaMessageSection&) emmSection;

    for( std::vector<Emm>::const_iterator it=emmSection.emms.begin(); 
            it != emmSection.emms.end(); ++it ) 
    {
       dest << (*it);
    }  
    	
    return dest;
}



   
   } 
}

