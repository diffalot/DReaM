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
 * $(@)TsDemuxer.cc $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "TsDemuxer.h"
#include "Filter.h"
#include "PsiFilter.h"
#include <iostream>
#include <stdexcept>
#include <cerrno>

namespace mpeg2ts {
    

class PatCallbackImpl: public PatCallback
{
public:
    PatCallbackImpl( TsDemuxer& demuxer ) : m_demuxer( demuxer ) {}
    
    virtual void operator()( Pat pat, unsigned int pid )
    {
        m_pat = pat;
        m_demuxer.setPatParsingDone(true);
    }
    
    Pat getPat() const 
    {
        return m_pat;
    }
    
private:
    TsDemuxer& m_demuxer;
    Pat        m_pat;
};

class PmtCallbackImpl: public PmtCallback
{
public:
    PmtCallbackImpl( TsDemuxer& demuxer ) : m_demuxer( demuxer ) {}
    
    virtual void operator()( Pmt pmt, unsigned int pid )
    {
        m_pmt = pmt;
        m_demuxer.setPmtParsingDone(true);
    }
    
    Pmt getPmt() const
    {
        return m_pmt;
    }
    
private:
    TsDemuxer& m_demuxer;
    Pmt        m_pmt;
};

TsDemuxer::TsDemuxer( std::string filename, Filter::Logging log )
    : m_patParsingDone( false ), m_tsFilter(log)
{
    std::fstream *infile = new std::fstream( filename.c_str(), 
                                             std::ios::in|std::ios::binary );
                                             
    if( ! *infile ) {
        std::cout << "TsDemuxer::TsDemuxer(): Error opening " << filename
                  << ": " << std::strerror(errno) << '\n';
        throw std::runtime_error( "FileOpenError" );   
    }
    
    m_file = FstreamPtr( infile );
    
}

static void feedData( std::fstream& strm, TsFilter& tsfilter )
{
    char tspkt[188];
    strm.read( tspkt, sizeof(tspkt) );
    int nread = strm.gcount();
    
    if( nread != sizeof(tspkt)  ) {
        if( strm.eof() ) {
            std::cout << "TsDemuxer feedData: fstream::read() End of File" << '\n';
            return;
        }
        else
            std::cout << "fstream::read(): Error: " << std::strerror(errno) << '\n';
        throw std::runtime_error( "Insufficient Data" );
    }
    
    if( !strm ) {
        std::cout << "ERROR: fstream::read() failed: " << std::strerror(errno) << '\n';
        throw std::runtime_error( "Bad Input Stream" );
    }
    
    tsfilter.process( m2t::Buffer( (unsigned char *)tspkt, nread ) );
}

std::pair<Pat, PmtList> TsDemuxer::getPatProgramList()
{
    m_numPmtsParsed = 0;
    
    PatCallbackImpl *patcb = new PatCallbackImpl( *this );
    boost::shared_ptr<PatFilter> patFilter( new PatFilter );
    m_tsFilter.setFilter( m2t::TsFilter::PatPid, patFilter );
    patFilter->setCallback( patcb );
    
    // PAT Callback code will set the m_patParsingDone flag after parsing PAT.
    while( !m_patParsingDone && *m_file  )
        feedData( *m_file, m_tsFilter );

    if( ! *m_file ) 
        throw std::runtime_error("Insufficient Data");
    
    m_patParsingDone = false;
    
    Pat pat = patcb->getPat();
    std::vector<PmtCallbackImpl *> pmtCbs;
    
    for( PatEntryList::iterator it=pat.entries.begin(); it!=pat.entries.end(); ++it ) 
    {
        std::cout << "ProgramNumber: " << (*it).programNumber.to_ulong()
            << "\tPID: " << (*it).programMapPid.to_ulong() << '\n';
            
        boost::shared_ptr<PmtFilter> pmtFilter( new PmtFilter );
        PmtCallbackImpl *pmtcb = new PmtCallbackImpl( *this );
        pmtFilter->setCallback( pmtcb );
        
        pmtCbs.push_back( pmtcb );
        
        m_tsFilter.setFilter( (*it).programMapPid.to_ulong(), pmtFilter );
    }
    
    m_numPmts = pat.entries.size();
        
    while( m_numPmtsParsed != m_numPmts && *m_file ) 
        feedData( *m_file, m_tsFilter );
    
    if( ! *m_file )
        throw std::runtime_error("Insufficient Data");
    
    m_numPmtsParsed = 0;
    
    PmtList pmts;
    
    for(std::vector<PmtCallbackImpl *>::iterator it = pmtCbs.begin();
            it!=pmtCbs.end(); ++it )
    {
        pmts.push_back( (*it)->getPmt() );
    }
   
    return std::make_pair(pat, pmts);
}

PmtList TsDemuxer::getProgramList()
{
    std::pair<Pat, PmtList> patPmt = getPatProgramList();
    return patPmt.second;
}
    
Filter *TsDemuxer::selectStream( unsigned long pid, FilterPtr streamProcessor )
{
    m_tsFilter.setFilter( pid, streamProcessor );
    return NULL;
}

int TsDemuxer::processAllData()
{
    while( *m_file ) {
        feedData( *m_file, m_tsFilter );
    }
    return 0;
}

void TsDemuxer::setPatParsingDone(bool value)
{
    m_patParsingDone = value;
}

void TsDemuxer::setPmtParsingDone(bool value)
{
    if( value==true )
        m_numPmtsParsed++;
}

}
