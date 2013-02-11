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
 * $(@)EncryptEmm $Revision: 1.2 $ $Date: 2006/09/29 21:50:22 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
 
/*
 * Encrypt an EMM using the RSA Public Key of the Client.
 * The EMM is an MPEG-2 Transport Packet with the EMM structure as specified in
 * the Dream-CAS specification.
 */

#include "mpeg2/Filter.h"
#include "dreamcas/Emm.h"
#include "ca_crypto/crypto.h"
#include "mpeg2/mpeg2ts_demux_stub.h"

#include <iostream>
#include <fstream>
#include <cerrno>
#include <stdexcept>

static std::string getEmmBytes( const char *inputFile );

// RSA encrypt the emm data and return the encrypted data
static std::string asymEncrypt( const char *rsaPublicKeyFilename, 
                                std::string& emmBytes );
                         
class EmmFilter : public mpeg2ts::Filter
{
public:
    virtual int process(mpeg2ts::Buffer tspayload );
    std::string getEmmBytes() const {return m_emmBytes;}
private:
	std::string m_emmBytes;
};
static void hexPrintBuf(char * message, unsigned char * buf, int len)
{
    int i = 0;
    printf("\n---------------------------------");
    printf("\n  %s   len = %d bytes   ", message, len);
    printf("\n---------------------------------\n");
    for ( i = 0; i < len; i ++ ) {
        if ( i != 0 && !(i % 10) ) {
            printf("\n");
        }
        printf("%2x ", buf[i]);
    }
    fflush(stdout);
}

int main( int argc, char *argv[] )
{

    if( argc != 4 ) {
        std::cout << "Usage: EmmEncrypt : <Input: EMM(in TS) file name>  "
            "<Input: Client RSA Public Key file name>  "
            "<Output: Encrypted file name>\n";
        return -1;
    }
    
    try {
	std::string rawEmm = getEmmBytes( argv[1] );
	
	hexPrintBuf( "emm_buf", (unsigned char*)rawEmm.c_str(), rawEmm.size() );
	
	std::string encryptedEmm = asymEncrypt( argv[2], rawEmm );
	
	
	std::cout << "Encrypted EMM size: " << encryptedEmm.size() << std::endl;
	
	PsiBytes emmBytes;
	
	dream::cas::CaMessageSection emmSection(0x82);
    
    m2t::OutputBitBuffer outb( emmBytes.data, sizeof(emmBytes.data) );
    emmSection.sectionLength = encryptedEmm.size();

    outb << emmSection;
    
    
	
	memcpy( emmBytes.data + outb.dataAvailable().first, encryptedEmm.c_str(),
	        encryptedEmm.size() );
	emmBytes.length = outb.dataAvailable().first + encryptedEmm.size();
	
	TsBytes tspkt = createTransportPacketForPsi( 600, //FIXME: hardcoded pid
	                                             1, emmBytes );
	                                                      
	// write encrypted emm to file
	
	std::fstream outfile( argv[3], std::ios::out|std::ios::binary );
	outfile.write( (char *)tspkt.data, sizeof(tspkt.data) );
	if( !outfile ) {
	    std::cout << "ERROR: Writing to " << argv[3] << " failed: "
	              << strerror(errno) << std::endl;
	    return -1;
	}
	    
	} catch ( std::runtime_error& e ) {
	    std::cout << "Exception: " << e.what() << std::endl;
	    return -1;
	}
	
    return 0;
}


std::string getEmmBytes( const char *inputFilename )
{
    std::fstream infile( inputFilename, std::ios::in | std::ios::binary );
    if( !infile )
    {
        std::cout << "ERROR opening \"" << inputFilename << "\": " 
            << std::strerror(errno) << std::endl;
        throw std::runtime_error("Could not open file");
    }
    
    mpeg2ts::TsFilter tsfilter;
    boost::shared_ptr<EmmFilter> emmFilter( new EmmFilter );
    
    tsfilter.setFilter( 600, // FIXME: hard-coded pid
                        emmFilter  );
    
    char tspkt[188];
    
    while( infile ){
       infile.read( tspkt, sizeof(tspkt) );
       if( infile.eof() )
           break;
       int ret = tsfilter.process( mpeg2ts::Buffer((unsigned char *)tspkt, 
                                                    infile.gcount() ) );
   }
        
    return emmFilter->getEmmBytes();
}

int EmmFilter::process(mpeg2ts::Buffer tspayload)
{
    mpeg2ts::InputBitBuffer emmBytes((const unsigned char *)tspayload.data+1, tspayload.len-1);
    dream::cas::EmmSection emmSection;
    
    emmBytes >> emmSection;
    
    std::cout << emmSection << std::endl;
    
    char outbuf[1024];
        
    mpeg2ts::OutputBitBuffer outbytes((unsigned char *) outbuf, sizeof(outbuf) );
        
    for( std::vector<dream::cas::Emm>::const_iterator it=emmSection.emms.begin();
                it!=emmSection.emms.end(); ++it )
        outbytes << (*it);
            
	m_emmBytes = std::string( (const char *)outbytes.data(), 
	                           outbytes.dataAvailable().first );
	
    return 0;
}

std::string asymEncrypt( const char *rsaPublicKeyFilename, 
                                std::string& emmBytes )
{
    unsigned char outbuf[1024];
    
    FILE *fpPublicKey = fopen( rsaPublicKeyFilename, "rb+" );
    if( fpPublicKey == NULL ) {
        std::cout << "ERROR opening Public Key File: " << strerror(errno) << std::endl;
        throw std::runtime_error("Could not open public key file");
    }
        
    unsigned char emmbuf[1024];
    memcpy( emmbuf, emmBytes.c_str(), emmBytes.size() );
            	hexPrintBuf( "out_buf", emmbuf, 35 );

    int outLen = asym_encrypt( fpPublicKey, (unsigned char *)emmbuf,
                               emmBytes.size(), outbuf );
        	hexPrintBuf( "out_buf", outbuf, outLen );

    fclose(fpPublicKey);
    
    if( outLen <=0 ) {
        std::cout << "ERROR: asymEncrypt(): asym_encrypt() failed\n";
        throw std::runtime_error("asym_encrypt() failed");
    }
    
    std::cout << "asymEncrypt: EMM len: " << emmBytes.size() << " outlen: " 
    << outLen << std::endl;
    
    return std::string((char *)outbuf, outLen);
    
}

