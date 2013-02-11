
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
 * $(@)FileWriter.cc $Revision: 1.3 $ $Date: 2006/07/25 22:39:11 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "mpeg2/FileWriter.h"

#include <stdexcept>
#include <fstream>
#include <cerrno>


namespace mpeg2ts {

FileWriter::FileWriter(const char *filename)
{
    boost::shared_ptr<std::fstream> outstream (new std::fstream( filename, std::ios::out | std::ios::binary ));
    if( !*outstream ) {
        std::cout << "FileWriter::FileWriter(): Could not open " 
            << filename << ": " << std::strerror(errno) << std::endl;
        throw std::runtime_error("File Open failed");
    }
    
    m_outstrm = outstream;
};

FileWriter::~FileWriter()
{
}

int FileWriter::process(Buffer bytes)
{
    m_outstrm->write( (const char *)bytes.hdrBegin, bytes.len 
                    + (bytes.data-bytes.hdrBegin));
                    
    if( ! *m_outstrm ) {
        std::cout << "FileWriter::process(): Error Writing to file: "
            << std::strerror(errno) << std::endl;
        throw std::runtime_error("File Write Error");
    }
    
    return 0;   
}

int FileWriter::process(Buffer data, long long& outStreamPos)
{
    outStreamPos = m_outstrm->tellp();
    
    return process(data);
}


}
