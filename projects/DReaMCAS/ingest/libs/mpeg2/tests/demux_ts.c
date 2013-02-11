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
 * $(@)demux_ts.cc $Revision: 1.1 $ $Date: 2006/07/15 00:08:40 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "mpeg2/TsDemuxer.h"
#include "mpeg2/PesFilter.h"
#include "mpeg2/VideoEsFilter.h"
#include "mpeg2/FileWriter.h"
#include "mpeg2/Descriptors.h"

#include<unistd.h>

#include <iostream>
#include <exception>

#include<boost/lexical_cast.hpp>

using namespace mpeg2ts;
using namespace std;
using boost::lexical_cast;

std::string usage( " [-v] <ts filename> [-p <pid> -f <filename_base>] " 
                   "\n\t Options: \n\t\t -v : verbose"
                   "\n\t\t -p : Demux the stream with <pid>"
                   "\n\t\t -f : writes the demuxed ts packets to <filename_base>.pid\n" );

int main(int argc, char *argv[])
{

    Filter::Logging log = Filter::LOG_NONE;
    int c;
    std::string outfile;
    int pid = -1;
    
    while( (c=getopt( argc, argv, "vp:f:")) != -1 )
    {
        switch(c)
        {
            case 'v':
                log = Filter::LOG_HEADER;
                break;
            case 'f':
                outfile = optarg;
                break;
            case 'p':
                pid = lexical_cast<int>(optarg);
                break;
            default:
                std::cout << argv[0] << usage << std::endl;
                return 1;
        }
    }
    
    if( pid != -1 && outfile.size()==0 ) {
        std::cout << "ERROR: Options -p and -f must be specified together\n";
        return 1;
    }
    
    // Read filename argument.
    if( optind >= argc ) {
        std::cout << argv[0] << usage << std::endl;
        return 1;
    }
    
    std::string infile( argv[optind] );
    
    try 
    {
        
        TsDemuxer demuxer( infile.c_str(), log );
        
        PmtList pmts = demuxer.getProgramList();
        
        for( PmtList::const_iterator it=pmts.begin(); it!=pmts.end(); ++it )
        {
            std::cout << (*it) << '\n';
        }
        
        assert( pmts.size() > 0 );
        
        std::cout << "Pmt descriptors: " << pmts[0].descriptors.size() << std::endl;
        for(std::vector<DescriptorPtr>::iterator it=pmts[0].descriptors.begin();
                it!=pmts[0].descriptors.end(); ++it )
        {
            if( (*it)->descriptorTag == Descriptor::CaTag ) {
                boost::shared_ptr<CaDescriptor> caDesc = boost::dynamic_pointer_cast<CaDescriptor>(*it);
                if( caDesc )			
                    std::cout << *caDesc << std::endl;
            }
        }
        
        
        if( pid!=-1 && outfile.size()!=0 )
        {
            FileWriterPtr writer( new FileWriter(outfile.c_str()) );
        
            demuxer.selectStream( pid, writer );
        }
        demuxer.processAllData();
        
    } catch( std::exception& e )
    {
        std::cout << argv[0] << ":\n\t Exception in main(): " << e.what() << '\n';
    }
}
