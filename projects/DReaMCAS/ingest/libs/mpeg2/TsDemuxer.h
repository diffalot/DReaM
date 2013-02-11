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
 * $(@)TsDemuxer.h $Revision: 1.2 $ $Date: 2006/07/15 00:02:42 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
#ifndef MPEG2TS_DEMUXER
#define MPEG2TS_DEMUXER

#include "Filter.h"
#include "PsiFilter.h"

#include <fstream>
#include <vector>


namespace mpeg2ts {

typedef std::auto_ptr<std::fstream> FstreamPtr;

class ProgramInfo;

/**
 * TsDemuxer simplifies the usage of all the other mpeg2ts filters in this package.
 * It installs the PAT and PMT filters and Callbacks, so the user can focus
 * on handling the elementary streams.
 */
 
class TsDemuxer
{
public:
    TsDemuxer( std::string filename, Filter::Logging log=Filter::LOG_NONE );
    
    virtual ~TsDemuxer(){};
    
    PmtList getProgramList();
    std::pair<Pat, PmtList> getPatProgramList();
    
    Filter *selectStream( unsigned long pid, FilterPtr streamProcessor );
    
    int processAllData();
    
    /*
     * The following methods are used by the PAT and PMT callbacks. 
     * TODO: Hide them from other users by making these methods friends of
     * the PAT and PMT callbacks.
     */
    void setPatParsingDone(bool);
    void setPmtParsingDone(bool);
    
private:
    FstreamPtr m_file;
    bool       m_patParsingDone;
    int        m_numPmtsParsed;
    int        m_numPmts;
    TsFilter   m_tsFilter;
};

}

#endif
