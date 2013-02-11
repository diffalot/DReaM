
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
 * $(@)EcmGenerator.cc $Revision: 1.2 $ $Date: 2006/07/25 22:39:09 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include <stdexcept>
#include <stdio.h>

#include "dreamcas/EcmGenerator.h"
#include "ca_crypto/crypto.h"
#include "mpeg2/OutputBitBuffer.h"
#include "mpeg2/FileWriter.h"

#include "dreamcas/Ecm.h"
#include "dreamcas/EcmWriter.h"
#include "mpeg2/mpeg2ts_demux_stub.h"

#include <string.h>

#include <fstream>
#include <string>

namespace m2t=mpeg2ts;

#define CA_SYS_ID  0x222
#define FREQUENCY 5
#define FOR_ANY_PROGRAM -1
#define ANY_STREAM_TYPE -1

#define TEST_MEMORY(p) if(p==NULL){ fprintf(stderr, "Memory check failed in %s line %d\n", __FILE__, __LINE__);exit(-1);}

typedef enum encrypt_type_s 
{
	ENCRYPT_ALL, 
	ENCRYPT_VIDEO_ONLY, 
	ENCRYPT_AUDIO_ONLY,
	ENCRYPT_UNKNOWN
}encrypt_type;

typedef struct encrypt_key_s
{
    encrypt_key_s(int programNumber, encrypt_type type, unsigned char *p_payload, int i_length)
    {
        e_type = type;
        pn     = programNumber;
        ecm_pid = 0; /* no . will be assigned later in the parser*/
        key_length = 0;
        /* get the key content (the ECM payload) */
        key_buf = new unsigned char[i_length];
        memcpy(key_buf, p_payload, i_length);
        key_length = i_length;
    }
    
    ~encrypt_key_s()
    {
        delete[] key_buf;
    }
    
    int                pn;           /* program number we are encrypting */
    encrypt_type       e_type;       /* encrypt type for this key */
    short              ecm_pid;      /* pid of the ecm containing the key */
    unsigned char *    key_buf;          /* key payload */
    int                key_length;       /* payload length */
    
private:
    encrypt_key_s( const encrypt_key_s& );
    encrypt_key_s& operator=( const encrypt_key_s& );
    
} encrypt_key_t;
 
typedef boost::shared_ptr<encrypt_key_s> EncryptionKeyPtr;
typedef encrypt_key_s *encrypt_key_p;

typedef struct
{
    bool       b_pmt_pid; /* is it a pck pid? */
    bool       b_seen;    /* being used ? */
    int           pn;        /* program number */
    encrypt_key_p p_key;     /* encryption key for this pid. chosen from the keys in sys */
    unsigned char  continuity_counter; /* TS packet counter */
} ts_pid_t;

/* structure of the ca descriptor,one ca descriptor corresponds to one key only */
struct ts_ca_descriptor_s 
{
    ts_ca_descriptor_s(encrypt_type encryption_type, int ca_system_id,
                       int ca_pid, unsigned char * p_private_data, int i_private_length)
    {
        stream_type = ANY_STREAM_TYPE;
        en_type     = encryption_type;
                
        desc = boost::shared_ptr<m2t::CaDescriptor>(new m2t::CaDescriptor);
        desc->caSystemId = ca_system_id;
        desc->caPid = ca_pid;
        desc->privateData = std::string( (char *)p_private_data, i_private_length );    
    }
    
    int                                  stream_type;/* the ca is used for this stream type,  */
    encrypt_type                         en_type;
    boost::shared_ptr<m2t::CaDescriptor> desc;       /*  the PSI descriptor */
    encrypt_key_p                        p_key;      /* encryption key this CA from */
    
private:
    ts_ca_descriptor_s( const ts_ca_descriptor_s& );
    ts_ca_descriptor_s& operator=( const ts_ca_descriptor_s& );
};

typedef boost::shared_ptr< ts_ca_descriptor_s > TsCaDescriptorPtr;
typedef std::vector< TsCaDescriptorPtr > TsCaDescriptorList;
typedef std::vector< EncryptionKeyPtr > EncryptionKeyList;

struct ca_sys_t
{
    ca_sys_t(int sys_id, unsigned int seconds)
    {
        /* setup data members */
        ecm_frequency = seconds;
        b_outputECMafterPMT = true;
        i_pmt_version_number = 0;
        
        /* pid initialization */
        for( int i = 0; i < 8192; i++ )
        {
            pid[i].b_seen     = false;
            pid[i].b_pmt_pid  = false;
            pid[i].pn         = 0;
            pid[i].p_key      = NULL;
            pid[i].continuity_counter = 0;
        }
        /* reserved pids */
        for( int i = 0x0; i < 0x1F4; i++ )
        {
            pid[i].b_seen   = true;
        }
        
        i_ca_system_id = sys_id;
    }

    ~ca_sys_t()
    {
    }
    
    int                 b_outputECMafterPMT;
    ts_pid_t            pid[8192];      /* pid arrays  */
    EncryptionKeyList   keys; /* a list of keys of the CAS */
    int                 ecm_frequency;  /* frequence of sending ECMs, seconds */
    int                 i_ca_system_id;
    int                 i_pmt_version_number;
};


static void addKey2CAS(ca_sys_t *p_sys, int pn, encrypt_type type, unsigned char *p_payload, int length)
{
	if (p_sys)
	{
		EncryptionKeyPtr pkey ( new encrypt_key_s(pn, type, p_payload, length) );
		p_sys->keys.push_back(pkey);
	}
	return;
}

namespace dream {
    namespace cas {

EcmGenerator::EcmGenerator(m2t::FileWriterPtr writer, const Emm& emm,
                           const char *ecmfile, EncryptionDebugKey keytype )
        : m_outfilter(writer), m_emm(emm), m_debugKeyType(keytype)
{
    assert( emm.entitlementTokenData.size()>0 && "EMM Key size should be > 0" );
    assert( emm.seedData.size() > 0 && "EMM IV size should be > 0" );
    
    if( ecmfile != NULL )
        m_ecmOut = EcmWriterPtr( new EcmWriter( ecmfile ) );
    
    m_pCas = new ca_sys_t(CA_SYS_ID, FREQUENCY);
}

EcmGenerator::~EcmGenerator()
{
   delete m_pCas;
}

void EcmGenerator::encryptProgram( unsigned int programNumber )
{
    assert( m_pCas->keys.size() == 0 );
    
    int pn = programNumber;
    
    if( programNumber == ALL_PROGRAMS )
        pn = FOR_ANY_PROGRAM;
    
    unsigned char dummyKey[16];
    
    addKey2CAS( m_pCas, pn, ENCRYPT_ALL, dummyKey, sizeof(dummyKey) );
    
    return;
}

void EcmGenerator::operator()(m2t::Pat pat, unsigned int patPid)
{
    for(m2t::PatEntryList::iterator it=pat.entries.begin(); it!=pat.entries.end(); ++it)
    {
        unsigned int pid = (*it).programMapPid.to_ulong(); 
        m_pCas->pid[ pid ].b_seen = true;
        m_pCas->pid[ pid ].pn = (*it).programNumber.to_ulong();
        m_pCas->pid[ pid ].b_pmt_pid = true;
    }
    
    return;
}

static unsigned int getFreePid( ca_sys_t *pcas, unsigned long programNumber );

static TsBytes injectCaDescriptorToPmt( ca_sys_t *pcas, m2t::Pmt& pmt, 
                                      unsigned int pmtPid,
                                      TsCaDescriptorList& caList );

/*                                      
 * Encrypt ECM data with the Key.IV in EMM and wrap the encrypted ECM into
 * an MPEG2 TS Program Map Section 
 */
PsiBytes generateCaMessageSection( const dream::cas::Ecm& ecm,
                                   const dream::cas::Emm& emm );


                                           
static TsBytes tsPacketizeCaMessage( ca_sys_t *pcas, unsigned int ecmPid, 
                                     PsiBytes& caBytes );


/******************************************************************************/

std::string KeyGenerator::generateKey( int size )
{
    unsigned char key[256];
    
    assert( size <= sizeof(key));
    
    generate_key(key, size);
    
    return std::string((const char *)key, size);
}

/******************************************************************************/

void EcmGenerator::operator()(m2t::Pmt pmt, unsigned int pmtPid)
{
    unsigned pcrPid = pmt.pcrPid.to_ulong();
    
    m_pCas->pid[pcrPid].b_seen = true;
    m_pCas->pid[pcrPid].pn = pmt.programNumber.to_ulong();
    
    assert( pmt.descriptors.size()==0 && "No CA Descriptors expected" );
    
    for(m2t::PmtEntryList::iterator it=pmt.entries.begin(); it!=pmt.entries.end(); ++it)
    {
        unsigned elementaryPid = (*it).elementaryPid.to_ulong();

        m_pCas->pid[elementaryPid].b_seen = true;
        m_pCas->pid[elementaryPid].pn = pmt.programNumber.to_ulong();
    }
    
    /* look up the keys. do we need CA injection for this PMT ? */
       
    TsCaDescriptorList tsCaDescriptorList;

    for( EncryptionKeyList::iterator it=m_pCas->keys.begin(); 
            it != m_pCas->keys.end(); ++it )
    {
        EncryptionKeyPtr pkeys = *it;
        int keyCount = 0;
        if( pkeys->pn==pmt.programNumber.to_ulong() || pkeys->pn==FOR_ANY_PROGRAM )
        {
            keyCount++;
            
            if( pkeys->ecm_pid == 0 )
                pkeys->ecm_pid = getFreePid( m_pCas, pmt.programNumber.to_ulong() );
            
            TsCaDescriptorPtr caDesc( new ts_ca_descriptor_s( pkeys->e_type,
                                                              m_pCas->i_ca_system_id,
                                                              pkeys->ecm_pid, 
                                                              NULL, 0 /*no private data*/) );

            caDesc->p_key = pkeys.get();//FIXME: Make CADESC accept smart pointer    
            
            tsCaDescriptorList.push_back( caDesc );  
        }
    }
    
    assert( m_pCas->pid[pmtPid].b_seen == true );
    assert( m_pCas->pid[pmtPid].pn == pmt.programNumber.to_ulong() );
    assert( m_pCas->pid[pmtPid].b_pmt_pid == true );
    
    TsBytes pmtTs = injectCaDescriptorToPmt( m_pCas, pmt, pmtPid, tsCaDescriptorList );
    
    if( m_outfilter )
        m_outfilter->process( m2t::Buffer(pmtTs.data,
                              sizeof(pmtTs.data), pmtTs.data, 0, 
                              m2t::Buffer::UNIT_START, pmtPid ) );
                              
    if( m_pCas->b_outputECMafterPMT ) 
    {
        for(EncryptionKeyList::iterator it=m_pCas->keys.begin(); 
                it != m_pCas->keys.end(); ++it )
        {
            EncryptionKeyPtr pkey = *it;
            if( pkey->pn==pmt.programNumber.to_ulong() || pkey->pn==FOR_ANY_PROGRAM ) {
                m_ecm = m_debugKeyType==GenerateNewKey 
                              ? Ecm( KeyGenerator::generateKey(), KeyGenerator::generateKey()) 
                              : Ecm( std::string("2222222222222222"),
                                     std::string("0123456789012345"));


                PsiBytes caSection = generateCaMessageSection( m_ecm, m_emm );

                TsBytes caInTs = tsPacketizeCaMessage( m_pCas, pkey->ecm_pid,
                                                       caSection );
                long long outpos=-1;                                                       
                if( m_outfilter )
                    m_outfilter->process( m2t::Buffer((unsigned char *)caInTs.data,
                                                     sizeof(caInTs.data), caInTs.data,
                                                     0, m2t::Buffer::UNIT_START,
                                                     pkey->ecm_pid), outpos );
                if( m_ecmOut ) {
                    unsigned char ecmBytes[1024];
                    m2t::OutputBitBuffer outb( ecmBytes, sizeof(ecmBytes) );
    
                    outb << m_ecm;
    
                    assert( outb.dataAvailable().first == m_ecm.sizeInBytes() );
                    
                    m_ecmOut->process( m2t::Buffer( ecmBytes, m_ecm.sizeInBytes(),
                                                    ecmBytes, outpos, m2t::Buffer::UNIT_START,
                                                    pkey->ecm_pid ) );
                }
            }
        }
    }
        
    return;
}

const Ecm& EcmGenerator::getCurrentEcm() const
{
    return m_ecm;
}
    
static unsigned int getFreePid( ca_sys_t *pcas, unsigned long programNumber )
{
    for(int i=0x10; i<0x01FFE; ++i) {
        if( pcas->pid[i].b_seen == false ) {
            pcas->pid[i].b_seen = true;
            pcas->pid[i].pn = programNumber;
            return i;
        }
    }    
    throw std::runtime_error("getFreePid(): No Free Pid found");
}

/*
----------------------------------------------------------------------
getStreamType: determine the stream type
ARGUMENTS:
    i_stream_type:   stream type value in the PMT es section
RETURN:
    stream type
----------------------------------------------------------------------
*/
static encrypt_type getStreamType(int i_stream_type )
{
    encrypt_type retType;
    switch( i_stream_type )
    {
        case 0x01:  /* MPEG-1 video */
        case 0x02:  /* MPEG-2 video */
        case 0x80:  /* MPEG-2 MOTO video */
        case 0x10:  /* MPEG4 (video) */
        case 0x1B:  /* H264 */
            retType = ENCRYPT_VIDEO_ONLY;
            break;

        case 0x03:  /* MPEG-1 audio */
        case 0x04:  /* MPEG-2 audio */
        case 0x11:  /* MPEG4 (audio) */
        case 0x0f:  /* ISO/IEC 13818-7 Audio with ADTS transport syntax */
        case 0x81:  /* A52 (audio) */
        case 0x83:  /* LPCM (audio) */
        case 0x84:  /* SDDS (audio) */
        case 0x85:  /* DTS (audio) */
        case 0x94:  /* SDDS (audio) */
            retType = ENCRYPT_AUDIO_ONLY;
            break;
        default:
        	retType = ENCRYPT_UNKNOWN;
        	break;
    }

    return retType;
}

TsBytes  injectCaDescriptorToPmt( ca_sys_t *pcas, m2t::Pmt& pmt, unsigned int pmtPid,
                                  TsCaDescriptorList& caList )
{
    /* Add program level CA Descriptors */
    
    for( TsCaDescriptorList::iterator it = caList.begin(); it != caList.end(); ++it )
    {
        if( (*it)->stream_type == ANY_STREAM_TYPE ) {
            pmt.descriptors.push_back((*it)->desc); 
        }
    }
    
    /* Add Elementary stream level CA descriptors */
    
    for(m2t::PmtEntryList::iterator it=pmt.entries.begin(); it!=pmt.entries.end(); ++it)
    {
        for( TsCaDescriptorList::iterator itdesc = caList.begin(); itdesc != caList.end(); ++itdesc )   
        {
            if( getStreamType( (*it).streamType.to_ulong() ) == (*itdesc)->stream_type )  {                               (*it).descriptors.push_back( (*itdesc)->desc );
            }
        }
    }
    
    // Convert PMT to bytes
    
    PsiBytes psibytes;
    m2t::OutputBitBuffer outb( psibytes.data, sizeof(psibytes.data) );
    
    assert( pmt.sizeInBytes() <= sizeof(psibytes.data) );

    outb << pmt;
    psibytes.length = pmt.sizeInBytes();
    
    pcas->pid[pmtPid].continuity_counter++;
    TsBytes tspkt = createTransportPacketForPsi( pmtPid, 
                                                 pcas->pid[pmtPid].continuity_counter,
                                                 psibytes );
    return tspkt;
}


PsiBytes generateCaMessageSection(const dream::cas::Ecm& ecm, const dream::cas::Emm& emm)
{
    PsiBytes ecmSectionBytes;
    
    unsigned char ecmBytes[1024];
    m2t::OutputBitBuffer ecmout( ecmBytes, sizeof(ecmBytes) );
    
    ecmout << ecm;
    
    assert( ecmout.dataAvailable().first == ecm.sizeInBytes() );
    
    int encryptedSize = encryptBufferData( ecmBytes, ecm.sizeInBytes(), 
                                           ecmBytes, sizeof(ecmBytes),
                                           (unsigned char *)emm.entitlementTokenData.data(),
                                           emm.entitlementTokenLength.to_ulong(),
                                           (unsigned char *)emm.seedData.data(),
                                           emm.seedLength.to_ulong(),
                                           AES_MODE(emm.cryptoMode.to_ulong()) ); 
        
    dream::cas::EcmSection ecmSection;
    ecmSection.sectionLength = encryptedSize;

    ecmSection.encryptedEcms.push_back( std::string((char *)ecmBytes, encryptedSize) );
    
    m2t::OutputBitBuffer outb( ecmSectionBytes.data, sizeof(ecmSectionBytes.data) );
        
    outb << ecmSection;
    
    assert( outb.dataAvailable().second == 0 );
    
   	ecmSectionBytes.length = outb.dataAvailable().first;
   	 
   	return ecmSectionBytes;

}


TsBytes tsPacketizeCaMessage( ca_sys_t *pcas, unsigned int ecmPid, PsiBytes& caBytes )
{
    pcas->pid[ecmPid].continuity_counter++;
    
    TsBytes outTs = createTransportPacketForPsi( ecmPid, 
                                                 pcas->pid[ecmPid].continuity_counter,
                                                 caBytes );
                                                 
    return outTs;                                                 
}



    }
}

