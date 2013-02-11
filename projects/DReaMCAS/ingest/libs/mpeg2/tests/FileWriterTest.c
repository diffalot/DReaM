
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
 * $(@)FileWriterTest.cc $Revision: 1.2 $ $Date: 2006/07/15 00:18:06 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#include "mpeg2/FileWriter.h"
#include "mpeg2/TsDemuxer.h"

using namespace mpeg2ts;

int main(int argc, char *argv[])
{
    if( argc != 3 ) {
        std::cout << "Usage: fileWriterTest <mpeg ts filename> <dest file>\n";
        return 1;
    }
    
    try
    {
        std::pair<Pat, PmtList> patPmts;
        
        {
            TsDemuxer demuxer( argv[1] );
        
            patPmts = demuxer.getPatProgramList();
        }
        
        TsDemuxer demuxer( argv[1] );
        
        FileWriterPtr writer(new FileWriter(argv[2]));
        
        // Set filewriter as the output filter for handling all PIDs.
        
        demuxer.selectStream( TsFilter::NullPid, writer );
        demuxer.selectStream( TsFilter::PatPid, writer );
        
        for( PatEntryList::iterator it=patPmts.first.entries.begin(); it!=patPmts.first.entries.end(); ++it )
            demuxer.selectStream( (*it).programMapPid.to_ulong(), writer );
        
        for( PmtList::iterator it=patPmts.second.begin(); it!=patPmts.second.end(); ++it )
            for( PmtEntryList::iterator ites=(*it).entries.begin(); 
                    ites!=(*it).entries.end(); ++ites )
            {
                demuxer.selectStream( (*ites).elementaryPid.to_ulong(), writer );
            }
        
        demuxer.processAllData();
        
    }catch(std::exception& e){
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
