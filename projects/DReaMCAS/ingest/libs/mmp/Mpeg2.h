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
 * $(@)Mpeg2.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

/*
 * Mpeg2.h
 * 
 * Mpeg2.h is the definition of Mpeg2 descriptors and other common
 * Mpeg2 types.
 */

#ifndef	_MMP_MPEG2_H
#define	_MMP_MPEG2_H

#pragma ident "@(#)Mpeg2.h 1.1	96/09/18 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include "cobjs/Foreign.h"
#include "cobjs/Macros.h"
#include "cobjs/Types.h"

_FOREIGN_START

#define	MPEG2_DESCRIPTOR_MAX_LEN	255

/***********************************************************************
 * Mpeg2 Stream Types
 ***********************************************************************/

typedef enum Mpeg2StreamType {
    MPEG2_STREAM_TYPE_MPEG1_VIDEO = 1,
    MPEG2_STREAM_TYPE_MPEG2_VIDEO = 2,
    MPEG2_STREAM_TYPE_MPEG1_AUDIO = 3,
    MPEG2_STREAM_TYPE_MPEG2_AUDIO = 4,
    MPEG2_STREAM_TYPE_PRIVATE_SECTION = 5,
    MPEG2_STREAM_TYPE_PRIVATE_STREAM = 6,
    MPEG2_STREAM_TYPE_MHEG = 7,
    MPEG2_STREAM_TYPE_DSMCC = 8,
    MPEG2_STREAM_TYPE_AUXILIARY = 9
} Mpeg2StreamType;

/***********************************************************************
 * Mpeg2 Descriptor Types
 ***********************************************************************/

typedef enum Mpeg2DescriptorTag {
    MPEG2_DESCRIPTOR_TAG_VIDEO_STREAM = 2,
    MPEG2_DESCRIPTOR_TAG_AUDIO_STREAM = 3,
    MPEG2_DESCRIPTOR_TAG_HIERARCHY = 4,
    MPEG2_DESCRIPTOR_TAG_REGISTRATION = 5,
    MPEG2_DESCRIPTOR_TAG_DATA_STREAM_ALIGNMENT = 6,
    MPEG2_DESCRIPTOR_TAG_TARGET_BACKGROUND_GRID = 7,
    MPEG2_DESCRIPTOR_TAG_VIDEO_WINDOW = 8,
    MPEG2_DESCRIPTOR_TAG_CA = 9,
    MPEG2_DESCRIPTOR_TAG_LANGUAGE = 10,
    MPEG2_DESCRIPTOR_TAG_SYSTEM_CLOCK = 11,
    MPEG2_DESCRIPTOR_TAG_MULTIPLEX_BUFFER_UTILIZATION = 12,
    MPEG2_DESCRIPTOR_TAG_COPYRIGHT = 13,
    MPEG2_DESCRIPTOR_TAG_MAXIMUM_BITRATE = 14,
    MPEG2_DESCRIPTOR_TAG_PRIVATE_DATA_INDICATOR = 15
} Mpeg2DescriptorTag;

typedef struct Mpeg2VideoStreamDescriptor {
    BIT5(multipleFrameRateFlag:1,
	 frameRateCode:4,
	 mpeg2Flag:1,
	 constrainedParameterFlag:1,
	 stillPictureFlag:1);
    /* if m2f == 1 */
    BIT3(profileLevelEscape:1,
	 profile:3,
	 level:4);
    BIT3(chromaFormat:2,
	 frameRateExtensionFlag:1,
	 reserved:5);
} Mpeg2VideoStreamDescriptor;

typedef enum Mpeg2FrameRateCode {
    MPEG2_FRAME_RATE_CODE_23_976 = 1,
    MPEG2_FRAME_RATE_CODE_24 = 2,
    MPEG2_FRAME_RATE_CODE_25 = 3,
    MPEG2_FRAME_RATE_CODE_29_97 = 4,
    MPEG2_FRAME_RATE_CODE_30 = 5,
    MPEG2_FRAME_RATE_CODE_50 = 6,
    MPEG2_FRAME_RATE_CODE_59_94 = 7,
    MPEG2_FRAME_RATE_CODE_60 = 8
} Mpeg2FrameRateCode;

typedef enum Mpeg2Profile {
    MPEG2_PROFILE_HIGH = 1,
    MPEG2_PROFILE_SPATIALLY_SCALABLE = 2,
    MPEG2_PROFILE_SNR_SCALABLE = 3,
    MPEG2_PROFILE_MAIN = 4,
    MPEG2_PROFILE_SIMPLE = 5
} Mpeg2Profile;

typedef enum Mpeg2Level {
    MPEG2_LEVEL_HIGH = 4,
    MPEG2_LEVEL_HIGH_1440 = 6,
    MPEG2_LEVEL_MAIN = 8,
    MPEG2_LEVEL_LOW = 10
} Mpeg2Level;

typedef enum Mpeg2ChromaFormat {
    MPEG2_CHROMA_FORMAT_4_2_0 = 1,
    MPEG2_CHROMA_FORMAT_4_2_2 = 2,
    MPEG2_CHROMA_FORMAT_4_4_4 = 3
} Mpeg2ChromaFormat;

typedef struct Mpeg2AudioStreamDescriptor {
    BIT4(freeFormatFlag:1,
	 id:1,
	 layer:2,
	 reserved:4);
} Mpeg2AudioStreamDescriptor;

typedef struct Mpeg2HierarchyDescriptor {
    BIT2(reserved_1:4,
	 hierarchyType:4);
    BIT2(reserved_2:2,
	 hierarchyLayerIndex:6);
    BIT2(reserved_3:2,
	 hierarchyEmbeddedLayer:6);
    BIT2(reserved_4:2,
	 hierarchyPriority:6);
} Mpeg2HierarchyDescriptor;

typedef enum Mpeg2HierarchyType {
    MPEG2_HIERARCHY_TYPE_VIDEO_SPATIAL = 1,
    MPEG2_HIERARCHY_TYPE_VIDEO_SNR = 2,
    MPEG2_HIERARCHY_TYPE_VIDEO_TEMPORAL = 3,
    MPEG2_HIERARCHY_TYPE_VIDEO_DATA = 4,
    MPEG2_HIERARCHY_TYPE_AUDIO_EXTENSION = 5,
    MPEG2_HIERARCHY_TYPE_BASE_LAYER = 15
} Mpeg2HierarchyType;

typedef struct Mpeg2RegistrationDescriptor {
    BIT1(formatIdentifier31_24:8);
    BIT1(formatIdentifier23_16:8);
    BIT1(formatIdentifier15_8:8);
    BIT1(formatIdentifier7_0:8);
    /*
     * Followed by additional identification info for rest of descriptor
     */
} Mpeg2RegistrationDescriptor;

#define	MPEG2_REGISTRATION_FORMAT_IDENTIFIER(p)			\
	(   ((p)->formatIdentifier31_24 << 24)			\
	  | ((p)->formatIdentifier23_16 << 16)			\
	  | ((p)->formatIdentifier15_8 << 8)			\
	  |  (p)->formatIdentifier7_0				\
	)

typedef struct Mpeg2DataAlignmentDescriptor {
    BIT1(alignmentType:8);
} Mpeg2DataAlignmentDescriptor;

typedef enum Mpeg2DataAlignmentTypeVideo {
    MPEG2_DATA_ALIGNMENT_TYPE_VIDEO_SLICE = 1, /* slice, picture, GOP, or SEQ */
    MPEG2_DATA_ALIGNMENT_TYPE_VIDEO_PICTURE = 2,/* picture, GOP, or SEQ */
    MPEG2_DATA_ALIGNMENT_TYPE_VIDEO_GOP = 3,   /* GOP, or SEQ */
    MPEG2_DATA_ALIGNMENT_TYPE_VIDEO_SEQ = 4    /* SEQ */
} Mpeg2DataAlignmentTypeVideo;

typedef enum Mpeg2DataAlignmentTypeAudio {
    MPEG2_DATA_ALIGNMENT_TYPE_AUDIO_FRAME = 1  /* audio frame */
} Mpeg2DataAlignmentTypeAudio;

typedef struct Mpeg2TargetBackgroundGridDescriptor {
    BIT1(horizontalSize13_6:8);
    BIT2(horizontalSize5_0:6,
	 verticalSize13_12:2);
    BIT1(verticalSize11_4:8);
    BIT2(verticalSize3_0:4,
	 pelAspectRatio:4);
} Mpeg2TargetBackgroundGridDescriptor;

#define	MPEG2_TARGET_BACKGROUND_GRID_HORIZONTAL_SIZE(p)		\
			( ((p)->horizontalSize13_6 << 6)	\
			 | (p)->horizontalSize5_0)

#define	MPEG2_TARGET_BACKGROUND_GRID_VERTICAL_SIZE(p)		\
			(  ((p)->verticalSize13_12 << 12) 	\
			 | ((p)->verticalSize11_4 << 4)		\
			 | (p)->verticalSize3_0)

typedef enum Mpeg2AspectRatio {
    MPEG2_ASPECT_RATIO_1_1 = 1,
    MPEG2_ASPECT_RATIO_3_4 = 2,
    MPEG2_ASPECT_RATIO_9_16 = 3,
    MPEG2_ASPECT_RATIO_1_2_21 = 4
} Mpeg2AspectRatio;

typedef struct Mpeg2VideoWindowDescriptor {
    BIT1(horizontalOffset13_6:8);
    BIT2(horizontalOffset5_0:6,
	 verticalOffset13_12:2);
    BIT1(verticalOffset11_4:8);
    BIT2(verticalOffset3_0:4,
	 windowPriority:4);
} Mpeg2VideoWindowDescriptor;

#define	MPEG2_VIDEO_WINDOW_HORIZONTAL_OFFSET(p)			\
			( ((p)->horizontalOffset13_6 << 6)	\
			 | (p)->horizontalOffset5_0)

#define	MPEG2_VIDEO_WINDOW_VERTICAL_OFFSET(p)			\
			(  ((p)->verticalOffset13_12 << 12) 	\
			 | ((p)->verticalOffset11_4 << 4)	\
			 | (p)->verticalOffset3_0)

typedef struct Mpeg2CaDescriptor {
    BIT1(caSystemId15_8:8);
    BIT1(caSystemId7_0:8);
    BIT2(resv:3,
	 caPid12_8:5);
    BIT1(caPid7_0:8);
    /*
     * Followed by private data for rest of descriptor
     */
} Mpeg2CaDescriptor;

#define	MPEG2_CA_SYSTEM_ID(p)					\
			( ((p)->caSystemId15_8 << 8)		\
			 | (p)->caSystemId7_0)

#define	MPEG2_CA_PID(p)						\
			( ((p)->caPid12_8 << 8) | (p)->caPid7_0)

/*
 * Mpeg2Iso639LanguageDescriptor
 *	As many 3 byte language descriptors as will fit in descriptor
 *	Followed by 8-bits of Audio Type
 */
typedef enum Mpeg2AudioType {
    MPEG2_AUDIO_TYPE_CLEAN_EFFECTS = 1,
    MPEG2_AUDIO_TYPE_HEARING_IMPAIRED = 2,
    MPEG2_AUDIO_TYPE_VISUAL_IMPAIRED_COMMENTARY = 3
} Mpeg2AudioType;

typedef struct Mpeg2SystemClockDescriptor {
    BIT3(externalClockReferenceIndicator:1,
	 reserved_1:1,
	 clockAccuracyInteger:6);
    BIT2(clockAccuracyExponent:3,
	 reserved_2:5);
} Mpeg2SystemClockDescriptor;

typedef struct Mpeg2MultiplexBufferUtilizationDescriptor {
    BIT2(mdvValidFlag:1,
	 multiplexDelayVariation15_8:7);
    BIT1(multiplexDelayVariation7_0:8);
    BIT2(multiplexStrategy:3,
	 reserved:5);
} Mpeg2MultiplexBufferUtilizationDescriptor;

#define	MPEG2_MULTIPLEX_BUFFER_UTILIZATION_MULTIPLEX_DELAY_VARIATION(p)	\
			( ((p)->multiplexDelayVariation15_8 << 8)	\
			 | (p)->multiplexDelayVariation7_0)

typedef enum Mpeg2MultiplexStrategy {
    MPEG2_MULTIPLEX_STRATEGY_EARLY = 1,
    MPEG2_MULTIPLEX_STRATEGY_LATE = 2,
    MPEG2_MULTIPLEX_STRATEGY_MIDDLE = 3
} Mpeg2MultiplexStrategy;

typedef struct Mpeg2CopyrightDescriptor {
    BIT1(copyrightIdentifier31_24:8);
    BIT1(copyrightIdentifier23_16:8);
    BIT1(copyrightIdentifier15_8:8);
    BIT1(copyrightIdentifier7_0:8);
    /*
     * Followed by additional copyright info for rest of descriptor
     */
} Mpeg2CopyrightDescriptor;

#define	MPEG2_COPYRIGHT_IDENTIFIER(p)					\
	(   ((p)->copyrightIdentifier31_24 << 24)			\
	  | ((p)->copyrightIdentifier23_16 << 16)			\
	  | ((p)->copyrightIdentifier15_8 << 8)				\
	  |  (p)->copyrightIdentifier7_0				\
	)

typedef struct Mpeg2MaximumBitrateDescriptor {
    BIT2(reserved:2,
	 maximumBitrate21_16:6);
    BIT1(maximumBitrate15_8:8);
    BIT1(maximumBitrate7_0:8);
} Mpeg2MaximumBitrateDescriptor;

#define	MPEG2_MAXIMUM_BITRATE(p)					\
			( ((p)->maximumBitrate21_16 << 16)		\
			 | ((p)->maximumBitrate15_8 << 8)		\
			 | (p)->maximumBitrate7_0)

typedef struct Mpeg2Descriptor {
    u8                  descriptorTag;
    u8                  descriptorLen;
    union {
        char                descriptor[MPEG2_DESCRIPTOR_MAX_LEN];
        Mpeg2VideoStreamDescriptor videoStreamDescriptor;
        Mpeg2AudioStreamDescriptor audioStreamDescriptor;
        Mpeg2HierarchyDescriptor hierarchyDescriptor;
        Mpeg2RegistrationDescriptor registrationDescriptor;
        Mpeg2DataAlignmentDescriptor dataAlignmentDescriptor;
        Mpeg2TargetBackgroundGridDescriptor targetBackgroundGridDescriptor;
        Mpeg2VideoWindowDescriptor videoWindowDescriptor;
        Mpeg2CaDescriptor   caDescriptor;
        Mpeg2SystemClockDescriptor systemClockDescriptor;
        Mpeg2MultiplexBufferUtilizationDescriptor
			    multiplexBufferUtilizationDescriptor;
        Mpeg2CopyrightDescriptor copyrightDescriptor;
        Mpeg2MaximumBitrateDescriptor maximumBitrateDescriptor;
    } un;
} Mpeg2Descriptor;

_FOREIGN_END

#endif	/* _MMP_MPEG2_H */
