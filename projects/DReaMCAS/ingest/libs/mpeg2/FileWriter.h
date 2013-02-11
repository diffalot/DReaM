
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
 * $(@)FileWriter.h $Revision: 1.2 $ $Date: 2006/07/15 00:18:06 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_FILE_WRITER_H
#define MPEG2TS_FILE_WRITER_H

#include "mpeg2/Filter.h"

#include <iostream>
#include <boost/shared_ptr.hpp>

namespace mpeg2ts {
    
class FileWriter : public Filter
{
public:
    FileWriter(const char *filename);
    virtual ~FileWriter();
    
    virtual int process(Buffer data);
    
    /**
     * Modules that need to know where the buffered data gets written to,
     * this process() method comes in handy.
     */
    virtual int process(Buffer data, long long& outStreamPos);

private:
    boost::shared_ptr<std::ostream> m_outstrm;
};
    
typedef boost::shared_ptr<FileWriter> FileWriterPtr; 
    
}

#endif
