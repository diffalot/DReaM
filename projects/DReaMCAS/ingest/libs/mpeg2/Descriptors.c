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
 * $(@)Descriptors.cc $Revision: 1.2 $ $Date: 2006/07/15 00:02:40 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

#include "Descriptors.h"
#include "OutputBitBuffer.h"
#include "InputBitBuffer.h"
#include <iostream>

namespace mpeg2ts {

InputBitBuffer& operator>>( InputBitBuffer& source, VideoStreamDescriptor& vdesc );
InputBitBuffer& operator>>(InputBitBuffer& source, AudioStreamDescriptor& adesc );
InputBitBuffer& operator>>( InputBitBuffer& source, UnknownDescriptor& udesc );
InputBitBuffer& operator>>( InputBitBuffer& source, CaDescriptor& udesc );

DescriptorPtr Descriptor::createDescriptor(InputBitBuffer& indata)
{
    std::bitset<8> byteval;
    indata >> byteval;

    switch( byteval.to_ulong() )
    {
    case VideoStreamTag:
    {
        VideoStreamDescriptor *vdesc=new VideoStreamDescriptor;
        indata >> *vdesc;
        return DescriptorPtr(vdesc);
        break;
    }
    case AudioStreamTag:
    {
        AudioStreamDescriptor *adesc=new AudioStreamDescriptor;
        indata >> *adesc;
        return DescriptorPtr(adesc);
        break;
    }
    case CaTag:
    {
        CaDescriptor *caDesc = new CaDescriptor;
        indata >> *caDesc;
        return DescriptorPtr(caDesc);
        break;
    }
    default:
    {
        UnknownDescriptor *desc = new UnknownDescriptor(byteval.to_ulong());
        indata >> *desc;
        return DescriptorPtr(desc);
        break;
    }
    }
}
    
OutputBitBuffer& operator<<( OutputBitBuffer& dest, Descriptor& desc )
{
    dest << desc.descriptorTag << desc.descriptorLength;

    return dest;
}

InputBitBuffer& operator>>( InputBitBuffer& source, VideoStreamDescriptor& vdesc )
{
    source >> vdesc.descriptorLength;
    
    PairBytesBits curpos = source.dataConsumed();

    source >> vdesc.multipleFrameRateFlag >> vdesc.frameRateCode 
       >> vdesc.mpeg1OnlyFlag >> vdesc.constrainedParameterFlag 
       >> vdesc.stillPictureFlag;

    if( vdesc.mpeg1OnlyFlag.none() )
    source >> vdesc.profileAndLevelIndication >> vdesc.chromaFormat
           >> vdesc.frameRateExtensionFlag >> vdesc.reserved;

    PairBytesBits endpos = source.dataConsumed();

    assert( endpos.first - curpos.first == vdesc.descriptorLength.to_ulong() );

    return source;
}
    
OutputBitBuffer& operator<<( OutputBitBuffer& dest, VideoStreamDescriptor& vdesc )
{
    vdesc.descriptorLength = vdesc.sizeInBytes() - Descriptor::nbytes();
    
    dest << (Descriptor&)vdesc
         << vdesc.multipleFrameRateFlag << vdesc.frameRateCode 
         << vdesc.mpeg1OnlyFlag << vdesc.constrainedParameterFlag
         << vdesc.stillPictureFlag;
         
    if( vdesc.mpeg1OnlyFlag.to_ulong()==0 )
        dest << vdesc.profileAndLevelIndication << vdesc.chromaFormat
             << vdesc.frameRateExtensionFlag << vdesc.reserved;
             
    return dest;
}

InputBitBuffer& operator>>(InputBitBuffer& source, AudioStreamDescriptor& adesc )
{
    source >> adesc.descriptorLength;
    source >> adesc.freeFormatFlag >> adesc.ID >> adesc.layer 
       >> adesc.variableRateAudioIndicator >> adesc.reserved;

    return source;
}

OutputBitBuffer& operator<<( OutputBitBuffer& dest, AudioStreamDescriptor& adesc )
{
    adesc.descriptorLength = AudioStreamDescriptor::nbytes() - Descriptor::nbytes();
    dest << (Descriptor&)adesc
         << adesc.freeFormatFlag << adesc.ID << adesc.layer
         << adesc.variableRateAudioIndicator << adesc.reserved;
    
    return dest;
}
    
    
OutputBitBuffer& operator<<( OutputBitBuffer& dest, LanguageDescriptor& ldesc )
{
    ldesc.descriptorLength = ldesc.sizeInBytes() - Descriptor::nbytes();
    
    dest << (Descriptor&)ldesc;
    
    LanguageDescriptorEntryList::iterator it = ldesc.entries.begin();
    for( ; it!=ldesc.entries.end(); ++it )
    {
        dest << (*it).iso639LanguageCode  << (*it).audioType;
    }
    
    return dest;
}

InputBitBuffer& operator>>( InputBitBuffer& source, UnknownDescriptor& udesc )
{
    source >> udesc.descriptorLength;
    std::string data;
    for( int i=0; i<udesc.descriptorLength.to_ulong(); ++i )
    {
    std::bitset<8> byteval;
    source >> byteval;
    data += (char)byteval.to_ulong();
    }
    udesc.data = data;

    return source;
}

OutputBitBuffer& operator<<( OutputBitBuffer& dest, UnknownDescriptor& udesc )
{
    dest << (Descriptor&)udesc;
    const char *data = udesc.data.data();

    for( int i=0; i<udesc.descriptorLength.to_ulong(); ++i ){
        dest.writeBits( 8, data[i] );
    }
    return dest;
}

OutputBitBuffer& operator<<( OutputBitBuffer& dest, CaDescriptor& caDesc )
{
    caDesc.descriptorLength = caDesc.sizeInBytes() - Descriptor::nbytes();
    
    dest << (Descriptor&)caDesc
         << caDesc.caSystemId << caDesc.reserved << caDesc.caPid
         << caDesc.privateData;
         
    return dest;
}

std::ostream& operator<<(std::ostream& cout, const CaDescriptor& caDesc)
{
    cout << "CaDescriptor:\n\tTag: " << (int)caDesc.descriptorTag.to_ulong()
         << "\n\tlength: " << (int)caDesc.descriptorLength.to_ulong()
         << "\n\tCA_system_ID: " << caDesc.caSystemId.to_ulong() 
         << "\n\treserved: " << caDesc.reserved
         << "\n\tCA_PID: " << caDesc.caPid.to_ulong() << std::endl;

         return cout;
}

InputBitBuffer& operator>>( InputBitBuffer& source, CaDescriptor& caDesc )
{
    source >> caDesc.descriptorLength >> caDesc.caSystemId >> caDesc.reserved
           >> caDesc.caPid;
           
    
    int privateBytesNum = caDesc.descriptorLength.to_ulong() 
                          - ( caDesc.caSystemId.size() + caDesc.reserved.size()
                              + caDesc.caPid.size())/8;
                              
    assert( privateBytesNum >= 0 );
    
    for(int i=0; i<privateBytesNum; ++i) {
        std::bitset<8>  byteval;
        
        source >> byteval;
        caDesc.privateData.push_back(byteval.to_ulong());
    }
                   
    return source;
}                  

}
