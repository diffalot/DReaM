
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
 * $(@)Scrambler.h $Revision: 1.1 $ $Date: 2006/07/15 00:26:54 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef DREAM_CAS_SCRAMBLER_H
#define DREAM_CAS_SCRAMBLER_H

#include "mpeg2/Filter.h"


namespace dream {
    namespace cas {

class EcmGenerator;

/**
 * ScramblerFilter manages encryption, key procurement.
 */

class ScramblerFilter: public m2t::Filter
{
public:
    ScramblerFilter( m2t::FilterPtr downStreamFilter, 
                     boost::shared_ptr<EcmGenerator> ecmGenerator );
    ~ScramblerFilter();
    
    virtual int process( m2t::Buffer buffer );

private:
    m2t::FilterPtr  m_outFilter;
    boost::shared_ptr<EcmGenerator> m_keyProvider;
    unsigned int    m_encryptedPackets;
    unsigned int    m_totalPacketsProcessed;
};

typedef boost::shared_ptr<ScramblerFilter> ScramblerFilterPtr;

    }
}
#endif
