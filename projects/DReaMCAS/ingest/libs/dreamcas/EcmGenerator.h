
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
 * $(@)EcmGenerator.h $Revision: 1.1 $ $Date: 2006/07/15 00:26:52 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef DREAM_CAS_MANAGER_H
#define DREAM_CAS_MANAGER_H

#include "dreamcas/Ecm.h"

#include "mpeg2/Filter.h"
#include "mpeg2/PsiFilter.h"
#include "dreamcas/Emm.h"

#include <boost/shared_ptr.hpp>

struct ca_sys_t;

namespace mpeg2ts {
class FileWriter;
}

namespace dream {
    namespace cas {
    
class EcmWriter;


class KeyGenerator
{
public:
    static std::string generateKey( int size=16 );
};

/**
 * EcmGenerator.   Generates ECM for encrypting elementary streams.
 *
 * Listens to PMTs, inserts Conditional Access Descriptors into the PMT and sends
 * out ECMs on a new ecm pid. 
 */

class EcmGenerator: public mpeg2ts::PmtCallback, public mpeg2ts::PatCallback
{
public:
    enum EncryptionDebugKey { UseDebugKey, GenerateNewKey };
    enum { ALL_PROGRAMS = 0xf000 }; 
    
    /**
     * @param filter  Downstream filter that handles the ECM-TS packets.
     *
     * @param emm  Provides the encryption parameters for encrypting  ECM.
     * @param ecmfile  Unencrypted ECMs are written to this file (for use in 
     *                 trickplay generation)
     * @param keytype  This defaults to 'GenerateNewKey' for every ecm. When set 
     *                 to 'UseDebugKey', each generated ecm has the same
     *                 encryption key and IV.
     */
     
    EcmGenerator( boost::shared_ptr<mpeg2ts::FileWriter> writer,
                  const Emm& emm, const char *ecmfile=NULL, 
                  EncryptionDebugKey keytype = GenerateNewKey );
                  
    ~EcmGenerator();
    
    void encryptProgram( unsigned int programNumber=ALL_PROGRAMS );
    
    virtual void operator()( mpeg2ts::Pmt pmt, unsigned int pmtPid );
    
    virtual void operator()( mpeg2ts::Pat pat, unsigned int patPid );
    
    const Ecm& getCurrentEcm() const;
    
private:
    ca_sys_t           *m_pCas;
    boost::shared_ptr<mpeg2ts::FileWriter> m_outfilter;
    Ecm                m_ecm;
    const Emm          m_emm;
    EncryptionDebugKey m_debugKeyType;
    boost::shared_ptr<EcmWriter> m_ecmOut;
};

typedef boost::shared_ptr<EcmGenerator> EcmGeneratorPtr;

    }
}

#endif
