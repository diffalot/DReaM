
#include "mpeg2/Filter.h"
#include "dreamcas/Emm.h"
#include<cerrno>
#include<iostream>
#include<fstream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

using namespace mpeg2ts;

class EmmFilter : public Filter
{
public:
    EmmFilter(){}
    EmmFilter(std::string outfile);
    virtual int process(Buffer tspayload );
private:
    boost::shared_ptr<std::fstream> outstrm;
};

int main( int argc, char *argv[] )
{
    if( argc<2 ) {
        std::cout << "Usage: " << argv[0] << " <EMM-TS filename> [<output emm file>]\n";
        return 1;
    }
    
    try
    {
        std::fstream infile( argv[1], std::ios::in|std::ios::binary );
        if( !infile ) {
            std::cout << "ERROR opening " << argv[1] << ": " 
                      << std::strerror(errno) << std::endl;
            return 1;
        }
        
        TsFilter tsFilter(Filter::LOG_HEADER);
        
        if( argc==3 )
            tsFilter.setFilter( 600, FilterPtr(new EmmFilter( argv[2] )));
        else
            tsFilter.setFilter( 600, FilterPtr(new EmmFilter) );
        
        char tspkt[188];
        
        while( infile && infile.read( tspkt, sizeof(tspkt) ) )
        {
            tsFilter.process( Buffer((unsigned char *)tspkt, infile.gcount() ) );
        }
        
    }catch(...)
    {
        std::cout << "Exception in main...\n";
        return 1;
    }
    
    return 0;
}

EmmFilter::EmmFilter(std::string outfile)
{
    outstrm = boost::shared_ptr<std::fstream>(new std::fstream(outfile.c_str(),
                                                               std::ios::out
                                                               |std::ios::binary ));
    
    if( !*outstrm ) {
        std::cout << "EmmFilter: Could not open " << outfile << ": " 
            << std::strerror(errno) << std::endl;
        throw std::runtime_error("Could not open output file");
    }
}


int EmmFilter::process(Buffer tspayload)
{
    InputBitBuffer emmBytes((const unsigned char *)tspayload.data+1, tspayload.len-1);
    dream::cas::EmmSection emmSection;
    
    emmBytes >> emmSection;
    
    std::cout << emmSection << std::endl;
    
    if( outstrm!=NULL && *outstrm ) 
    {
        char outbuf[1024];
        
        OutputBitBuffer outbytes((unsigned char *) outbuf, sizeof(outbuf) );
        
        for( std::vector<dream::cas::Emm>::const_iterator it=emmSection.emms.begin();
                    it!=emmSection.emms.end(); ++it )
            outbytes << (*it);
            
        outstrm->write( (const char *)outbytes.data(), outbytes.dataAvailable().first );
        if( ! *outstrm ) {
            std::cout <<  "Error: EmmFilter::process(): writing to output file failed:"
                << std::strerror(errno) << std::endl;
            
            return 1;
        }
    
    }
    
    return 0;
}
