
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
 * $(@)EncryptTs.cc $Revision: 1.5 $ $Date: 2006/09/29 21:50:22 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "dreamcas/EcmGenerator.h"
#include "dreamcas/Scrambler.h"
#include "mpeg2/TsDemuxer.h"
#include "mpeg2/FileWriter.h"
#include "mpeg2/mpeg2ts_demux_stub.h"

#include <stdexcept>
#include <stdio.h>

#include "ca_crypto/crypto.h"

#include <cerrno>
#include <fstream>

#include <boost/progress.hpp>

using namespace std;
namespace m2t=mpeg2ts;

using boost::shared_ptr;
using namespace dream::cas;

std::string usage("encryptTs [-d ] [-c <Ecm file>] [-m <Emm file>] [-t <ECM Inserted TS file>] <mpeg2 ts file> <output ts file>"
                  "\n\t -d    Use Debug Encryption Keys"
                  "\n\t -c    file ECMs are written to"
                  "\n\t -m    file to write EMM into"
                  "\n\t -t    temporary TS file with ECM TS inserted (no scrambling on Elementary streams)\n");

static unsigned getPmtPid( std::string tsfile );
static m2t::PmtList getPmts( std::string tsfile );
static unsigned long long fileSize( std::string& filename );
static void generateEcmInsertedTsFile( std::string inputTsFilename,
                                       std::string outputTsFilename,
                                       Emm emm, 
                                       EcmGenerator::EncryptionDebugKey encKeyType );
static  TsBytes wrapEmmInTs( const Emm& emm );


int main(int argc, char *argv[])
{
    if( argc < 3 ) {
        std::cout << usage << std::endl;
        return 1;
    }

    char *ecmFile = NULL;
    char *emmFile = NULL;
    char *tmpTsFileWithEcm = NULL;
    EcmGenerator::EncryptionDebugKey encKeyType = EcmGenerator::GenerateNewKey;
    Emm emm(MODE_CTR, KeyGenerator::generateKey(), 
                      KeyGenerator::generateKey());
    int c;

    while( (c=getopt(argc, argv, "dc:m:t:")) != -1 )
    {
        switch(c)
        {
        case 'd':
            encKeyType = EcmGenerator::UseDebugKey;
            emm = Emm( MODE_CTR, std::string("2222222222222222"),
                                 std::string("0123456789012345"));
            break;
        case 'c':
            ecmFile = optarg;
            break;
        case 'm':
            emmFile = optarg;
            break;
        case 't':
            tmpTsFileWithEcm = optarg;
            break;
        default:
            std::cout << usage << std::endl;
            return 1;
        }
    }

    try 
    {
        std::string infile( argv[optind] );
        
        if( emmFile ) {
        	TsBytes emmTs = wrapEmmInTs( emm );
        	std::fstream emmStrm( emmFile, std::ios::out|std::ios::binary );
        	if( !emmStrm ) {
        	    std::cout << "ERROR Opening file " << emmFile << std::endl;
        	    return -1;
        	}
        	emmStrm.write( (const char*)emmTs.data, sizeof(emmTs.data) );
        }
        	
        if( tmpTsFileWithEcm != NULL )
            generateEcmInsertedTsFile( infile, tmpTsFileWithEcm, emm, encKeyType );
        
        m2t::TsFilter tsfilter;
        
        m2t::FileWriterPtr writer( new m2t::FileWriter(argv[optind+1]) );

        unsigned pmtPid = getPmtPid(infile);
        m2t::PmtList pmts = getPmts(infile);

        EcmGeneratorPtr ecmGenerator;
        
        ecmGenerator = EcmGeneratorPtr( new EcmGenerator(writer, emm, ecmFile,
                                            encKeyType) );
        
        ecmGenerator->encryptProgram( EcmGenerator::ALL_PROGRAMS );
        
        m2t::PmtFilterPtr pmtFilter( new m2t::PmtFilter );
        pmtFilter->setCallback( ecmGenerator.get() );
        m2t::PatFilterPtr patFilter( new m2t::PatFilter );
        patFilter->setCallback( ecmGenerator.get() );
        
        tsfilter.setFilter( m2t::TsFilter::NullPid, writer );
        tsfilter.setFilter( m2t::TsFilter::PatPid, patFilter );
        tsfilter.setFilter( pmtPid, pmtFilter );
        tsfilter.setFilter( m2t::TsFilter::PatPid, writer );

        // TODO:Make sure this ts stream isn't CAS'ed already.
        
        ScramblerFilterPtr scrambler(new ScramblerFilter( writer, ecmGenerator ) );
        
        // No need to set the 'writer' as the filter for PMT data. The 
        // EcmGenerator, which acts as the PMT callback re-writes the PMT data
        // and sends it to the writer.
        
        // The following pipelines are set up:
        //
        // tsfilter | Scrambler | FileWriter  (for elementary stream PIDs)
        // tsfilter | PatFilter | EcmGenerator | FileWriter
        //                                                 EcmFileWriter
        //                                          Ecms < 
        //                                       /         FileWriter
        // tsfilter | PmtFilter | EcmGenerator < 
        //                                       ModifiedPmts | FileWriter
        
        for(m2t::PmtList::const_iterator itpmt=pmts.begin(); itpmt!=pmts.end(); ++itpmt )
            for(m2t::PmtEntryList::const_iterator it=(*itpmt).entries.begin();
                    it!=(*itpmt).entries.end(); ++it )
            {
                tsfilter.setFilter( (*it).elementaryPid.to_ulong(), scrambler );
            }
            
        std::fstream instrm( infile.c_str(), std::ios::in|std::ios::binary );
        if( !instrm ) {
            std::cout << "ERROR Opening file: " << infile << ": " << strerror(errno) << std::endl;
            return 1;
        }

        std::cout << "\n\n     Encrypting TS file\n\n";
        
        boost::progress_display progressBar( fileSize(infile) );
        
        while(1)
        {
            char tspkt[188];
            
            instrm.read( tspkt, sizeof(tspkt) );
            if( instrm.eof() ) {
                break;
            }
            
            tsfilter.process( m2t::Buffer((unsigned char*)tspkt, instrm.gcount() ) );
            
            progressBar += instrm.gcount();
        }
        
    }catch(std::exception& e){
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

static void generateEcmInsertedTsFile( std::string infile, std::string outputTsFilename,
                                       Emm emm, EcmGenerator::EncryptionDebugKey encKeyType )
{
    m2t::TsFilter tsfilter;
    
    m2t::FileWriterPtr writer( new m2t::FileWriter(outputTsFilename.c_str()) );
    
    
    unsigned pmtPid = getPmtPid(infile);
    m2t::PmtList pmts = getPmts(infile);
    
    EcmGeneratorPtr ecmGenerator( new EcmGenerator(writer, emm, NULL,
    encKeyType) );
    
    ecmGenerator->encryptProgram( EcmGenerator::ALL_PROGRAMS );
    
    m2t::PmtFilterPtr pmtFilter( new m2t::PmtFilter );
    pmtFilter->setCallback( ecmGenerator.get() );
    m2t::PatFilterPtr patFilter( new m2t::PatFilter );
    patFilter->setCallback( ecmGenerator.get() );
    
    tsfilter.setFilter( m2t::TsFilter::NullPid, writer );
    tsfilter.setFilter( m2t::TsFilter::PatPid, patFilter );
    tsfilter.setFilter( pmtPid, pmtFilter );
    tsfilter.setFilter( m2t::TsFilter::PatPid, writer );
    
    // TODO:Make sure this ts stream isn't CAS'ed already.
    
    
    // No need to set the 'writer' as the filter for PMT data. The 
    // EcmGenerator, which acts as the PMT callback re-writes the PMT data
    // and sends it to the writer.
    
    // The following pipelines are set up:
    //
    // tsfilter | FileWriter  (for elementary stream PIDs)
    // tsfilter | PatFilter | EcmGenerator | FileWriter
    //                                                 EcmFileWriter
    //                                          Ecms < 
    //                                       /         FileWriter
    // tsfilter | PmtFilter | EcmGenerator < 
    //                                       ModifiedPmts | FileWriter
    
    for(m2t::PmtList::const_iterator itpmt=pmts.begin(); itpmt!=pmts.end(); ++itpmt )
        for(m2t::PmtEntryList::const_iterator it=(*itpmt).entries.begin();
            it!=(*itpmt).entries.end(); ++it )
        {
            tsfilter.setFilter( (*it).elementaryPid.to_ulong(), writer );
        }
        
    std::fstream instrm( infile.c_str(), std::ios::in|std::ios::binary );
    if( !instrm ) {
        std::cout << "ERROR Opening file: " << infile << ": " << strerror(errno) << std::endl;
        throw std::runtime_error("Could not open input file");
    }
    
    std::cout << " \n\n    Inserting ECM into Temporary MPEG TS file\n\n";
    
    boost::progress_display progressBar( fileSize(infile) );
    
    while(1)
    {
        char tspkt[188];
        
        instrm.read( tspkt, sizeof(tspkt) );
        if( instrm.eof() ) {
            break;
        }
        
        tsfilter.process( m2t::Buffer((unsigned char*)tspkt, instrm.gcount() ) );
        
        progressBar += instrm.gcount();
    }
}

unsigned getPmtPid( std::string tsfile )
{
    m2t::TsDemuxer demuxer( tsfile.c_str() );
    std::pair<m2t::Pat, m2t::PmtList> patPmts = demuxer.getPatProgramList();
        
    if( patPmts.first.entries.size() != 1 ) {
        std::cout << "ERROR: Exactly 1 PMT PID is expected in PAT. Found "
                  << patPmts.first.entries.size() << std::endl;
        throw std::runtime_error("PMT not found");
    }
        
    return patPmts.first.entries[0].programMapPid.to_ulong();
    
}

static m2t::PmtList getPmts( std::string tsfile )
{
    m2t::TsDemuxer demuxer( tsfile );
    
    return demuxer.getProgramList();
    
}

static unsigned long long fileSize( std::string& filename )
{
    std::fstream strm( filename.c_str(), std::ios::binary|std::ios::in );
    if( !strm ){
        std::cout << "ERROR opening file: " << filename << ": " << strerror(errno) << std::endl;
        throw std::runtime_error( strerror(errno) );
    }
    
    strm.seekg( 0, std::ios::end );
    
    return strm.tellg();
    
}

TsBytes wrapEmmInTs( const Emm& emm )
{
    std::cout << "wrapEmmInTs: " << std::endl;
    PsiBytes emmPsiBytes;
    
    dream::cas::EmmSection emmSection;
    emmSection.emms.push_back( emm );
    
    m2t::OutputBitBuffer outb( emmPsiBytes.data, sizeof(emmPsiBytes.data) );
    
    outb << emmSection;
    
    assert( outb.dataAvailable().second == 0 );
    
    emmPsiBytes.length = outb.dataAvailable().first;
    
    TsBytes tspkt = createTransportPacketForPsi( 600 /*FIXME: hard coded pid*/, 1, emmPsiBytes );
    return tspkt;
}

