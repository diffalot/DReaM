
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
 * $(@)EcmWriter.h $Revision: 1.1 $ $Date: 2006/07/15 00:26:53 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef DREAM_CAS_ECM_WRITER_H
#define DREAM_CAS_ECM_WRITER_H

#include "mpeg2/Filter.h"

#include <iostream>

#include <boost/shared_ptr.hpp>

namespace dream {
    namespace cas {
        
class EcmWriter
{
public:
    EcmWriter(const char *filename);
    ~EcmWriter();
    
    /**
     * Pass One ECM to the ECM Writer with each process() call.
     */
    virtual int process(m2t::Buffer data);
    
private:
    boost::shared_ptr<std::iostream> m_strm;
    unsigned int m_numEcms;
};

typedef boost::shared_ptr<EcmWriter> EcmWriterPtr;
        
    }
}

#endif
