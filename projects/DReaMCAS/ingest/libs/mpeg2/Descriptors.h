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
 * $(@)Descriptors.h $Revision: 1.3 $ $Date: 2007/03/06 02:31:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#ifndef MPEG2TS_DESCRIPTORS_H
#define MPEG2TS_DESCRIPTORS_H

#include <bitset>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace mpeg2ts {

class Descriptor;
typedef boost::shared_ptr<Descriptor> DescriptorPtr;
typedef std::vector<Descriptor> DescriptorList;

class InputBitBuffer;
class OutputBitBuffer;
class AudioStreamDescriptor;
class VideoStreamDescriptor;
class LanguageDescriptor;
class UnknownDescriptor;
class CaDescriptor;
OutputBitBuffer& operator<<( OutputBitBuffer& dest, Descriptor& descriptor );
OutputBitBuffer& operator<<( OutputBitBuffer& dest, AudioStreamDescriptor& adesc );
OutputBitBuffer& operator<<( OutputBitBuffer& dest, VideoStreamDescriptor& vdesc );
OutputBitBuffer& operator<<( OutputBitBuffer& dest, LanguageDescriptor& vdesc );
OutputBitBuffer& operator<<( OutputBitBuffer& dest, UnknownDescriptor& vdesc );    
OutputBitBuffer& operator<<( OutputBitBuffer& dest, CaDescriptor& cadesc );    
std::ostream& operator<<(std::ostream& cout, const CaDescriptor& caDesc);

struct Descriptor
{
    enum { VideoStreamTag=2,
       AudioStreamTag=3,
       HierarchyTag=4,
       RegistrationTag=5,
       DataStreamAlignmentTag=6,
       TargetBackgroundTag=7,
       VideoWindowTag=8,
       CaTag=9,
       LanguageTag=10,
       SystemClockTag=11,
       MultiplexBufferUtilization=12,
       Copyright=13,
       MaximumBitrate=14,
       PrivateDataIndicator=15,
       SmoothingBuffer=16,
       STD=17,
       IBP=18,
       UserPrivateBegin=64,
       UserPrivateEnd=255
    };
    static DescriptorPtr createDescriptor(InputBitBuffer& indata);
    Descriptor( uint8_t tag, uint8_t length ): descriptorTag(tag), descriptorLength(length){}
    virtual ~Descriptor(){}
    std::bitset<8> descriptorTag;
    std::bitset<8> descriptorLength;
    static int nbytes(){ return 2; }
    virtual int sizeInBytes() { return nbytes(); }
    virtual OutputBitBuffer& write( OutputBitBuffer& outb )=0;
};

struct UnknownDescriptor: public Descriptor
{
    UnknownDescriptor(uint8_t tag, std::string descData)
    : Descriptor(tag, descData.length()), data(descData) 
    {}
    UnknownDescriptor(uint8_t tag ):Descriptor(tag, 0){}

    std::string data;
    virtual int sizeInBytes() { 
	return data.length() + Descriptor::sizeInBytes(); }
    virtual OutputBitBuffer& write( OutputBitBuffer& outb ){
	outb << *this;
	return outb;
    }
};

struct VideoStreamDescriptor: public Descriptor
{
    VideoStreamDescriptor( ): Descriptor( VideoStreamTag, 0 )
    {}
    
    std::bitset<1> multipleFrameRateFlag;
    std::bitset<4> frameRateCode;
    std::bitset<1> mpeg1OnlyFlag;
    std::bitset<1> constrainedParameterFlag;
    std::bitset<1> stillPictureFlag;
    
    // The following fields are valid if mpeg1OnlyFlag is set to '0'
    std::bitset<8> profileAndLevelIndication;
    std::bitset<2> chromaFormat;
    std::bitset<1> frameRateExtensionFlag;
    std::bitset<5> reserved;
    
    virtual int sizeInBytes(){ return mpeg1OnlyFlag == 0 ? 3+Descriptor::nbytes() 
                         : 1+Descriptor::nbytes(); }
    virtual OutputBitBuffer& write( OutputBitBuffer& outb ){ outb << *this; return outb; }
};
    
struct AudioStreamDescriptor: public Descriptor
{
    AudioStreamDescriptor(): Descriptor(AudioStreamTag, 0){}
    std::bitset<1> freeFormatFlag;
    std::bitset<1> ID;
    std::bitset<2> layer;
    std::bitset<1> variableRateAudioIndicator;
    std::bitset<3> reserved;
    virtual int sizeInBytes(){ return nbytes(); }
    static int nbytes(){ return 1+Descriptor::nbytes(); }
    virtual OutputBitBuffer& write( OutputBitBuffer& outb ){ outb << *this; return outb; }
};

struct LanguageDescriptorEntry: public Descriptor
{
    LanguageDescriptorEntry(): Descriptor(LanguageTag, 0){}
    std::bitset<24> iso639LanguageCode;
    std::bitset<8>  audioType;
    virtual int sizeInBytes(){ return nbytes(); }
    static int nbytes() { return 4; }
    virtual OutputBitBuffer& write( OutputBitBuffer& outb ){ outb << *this; return outb;}
};

typedef std::vector<LanguageDescriptorEntry> LanguageDescriptorEntryList;

struct LanguageDescriptor: public Descriptor
{
    LanguageDescriptor(): Descriptor(LanguageTag, 0){}
    LanguageDescriptorEntryList entries;
    virtual int sizeInBytes(){ return Descriptor::nbytes()
        + LanguageDescriptorEntry::nbytes()*entries.size();
    }
};

struct CaDescriptor: public Descriptor
{
    CaDescriptor(): Descriptor(CaTag, 0), reserved(0x7)
    {}
    
    std::bitset<16> caSystemId;
    std::bitset<3>  reserved;
    std::bitset<13> caPid;
    std::string     privateData;
    
    virtual int sizeInBytes(){ return 4 + privateData.size() + Descriptor::nbytes(); }
    
    virtual OutputBitBuffer& write( OutputBitBuffer& outb ){ outb << *this; return outb; }
};


}

#endif
