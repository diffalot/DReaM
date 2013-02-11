
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
 * $(@)EcmWriter.cc $Revision: 1.2 $ $Date: 2006/07/25 22:39:10 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "EcmWriter.h"
#include "Ecm.h"

#include <stdexcept>
#include <fstream>
#include <cerrno>

namespace dream {
    namespace cas {

/* structure of the ECM meta file */
/*
    ECMfile_header
    ECMfile_data_item
*/
typedef struct  {
	int version;
 	int no_of_ECMs;	
 	int ecm_len;
	uint64_t metadata_offset;
} ECMfile_header;

#define ECM_FILE_HEADER_SIZE (sizeof(int) + sizeof(int) + sizeof(int) + sizeof(uint64_t))

typedef struct  {
/* positions of */
	uint64_t insert_offset; /* with respect to the file without ECM */
	uint64_t new_offset;   /* with resepct to the new file */
 	unsigned char *ecm_payload; /* note: length of the payload only, each ECM may contain multiple TS packets */
} ECMfile_data_item;

typedef struct  {
	ECMfile_header header;
	int item_len;
    ECMfile_data_item ecmItem;
	
} ECMfile_info;


void static writeEcmFileHeader( std::ostream& out, ECMfile_info& info )
{
    out.seekp(0, std::ios::beg);
    
    out.write( (char*)&info.header.version, sizeof(info.header.version) );
    out.write( (char*)&info.header.no_of_ECMs, sizeof(info.header.no_of_ECMs) );
    out.write( (char*)&info.header.ecm_len, sizeof(info.header.ecm_len) );
    out.write( (char*)&info.header.metadata_offset, sizeof(info.header.metadata_offset) );
 
    if( !out ) {
        std::cout << "writeEcmFileHeader: write() failed: " << std::strerror(errno)
                  << std::endl;
        throw std::runtime_error("File Write failed" );
    }
}

ECMfile_info getEcmFileInfo( std::istream& in )
{
    in.seekg(0, std::ios::beg);
    
    ECMfile_info info;
 
    in.read( (char*)&info.header.version, sizeof(info.header.version) );
    in.read( (char*)&info.header.no_of_ECMs, sizeof(info.header.no_of_ECMs) );
    in.read( (char*)&info.header.ecm_len, sizeof(info.header.ecm_len) );
    in.read( (char*)&info.header.metadata_offset, sizeof(info.header.metadata_offset) );
    
    if( !in ) {
        std::cout <<  "ERROR: getEcmFileInfo: Read failed: " << std::strerror(errno) << std::endl;
        throw std::runtime_error("File read failed");
    }
    
    return info;
}

EcmWriter::EcmWriter(const char *filename): m_numEcms(0)
{
    m_strm = boost::shared_ptr<std::iostream>( new std::fstream( filename,
                                                          std::ios::in|std::ios::out
                                                          |std::ios::binary
                                                          |std::ios::trunc ) );
                                                          
    if( ! *m_strm ) {
        m_strm.reset();
        std::cout << "EcmWriter::EcmWriter(): Could not open " << filename 
                  << ": " << std::strerror(errno) << std::endl;
        throw std::runtime_error("FileOpen failed");
    }
    
    ECMfile_info ecmFileInfo;
    ecmFileInfo.header.version = 1;
    ecmFileInfo.header.no_of_ECMs = 0;
    ecmFileInfo.header.ecm_len = Ecm::SIZE;
    ecmFileInfo.header.metadata_offset = ECM_FILE_HEADER_SIZE;
    ecmFileInfo.item_len = sizeof(uint64_t) + sizeof(uint64_t) 
                            + ecmFileInfo.header.ecm_len;
    
    writeEcmFileHeader( *m_strm, ecmFileInfo );                                
}


EcmWriter::~EcmWriter()
{
    ECMfile_info fileInfo =  getEcmFileInfo( *m_strm );
    
    fileInfo.header.no_of_ECMs = m_numEcms;
    
    writeEcmFileHeader( *m_strm, fileInfo );
}

int EcmWriter::process(m2t::Buffer ecmbytes)
{
    uint64_t insertPos = -1; // don't know yet
    uint64_t newPos = ecmbytes.posInTsStream;
    
    m_strm->write( (char*)&insertPos, sizeof(insertPos) );
    m_strm->write( (char *)&newPos, sizeof(newPos) );
    m_strm->write((const char *)ecmbytes.hdrBegin, 
                   ecmbytes.len + (ecmbytes.data - ecmbytes.hdrBegin));
                   
    if( ! *m_strm ) {
        std::cout << "EcmWriter::process(): Error writing to file: "
                  << std::strerror(errno) << std::endl;
        throw std::runtime_error("File Write failed");
    }
    
    ++m_numEcms;
    
    return 0;
}


    }
}

