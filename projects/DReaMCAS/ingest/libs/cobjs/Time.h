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
 * $(@)Time.h $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Time.h -- An arbitrary tick representation of time and a
 * set of routines for converting to and from the many other time
 * representations.
 */

#ifndef	_COBJS_TIME_H
#define	_COBJS_TIME_H

#pragma ident "@(#)Time.h 1.4	99/03/22 SMI"

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <time.h>

#include "cobjs/Foreign.h"
#include "cobjs/Inline.h"
#include "cobjs/Types.h"

/***********************************************************************
 * Defines
 ***********************************************************************/

/*
 * As per Unix specification
 */
#define	TIME_TV_TICKS_PER_SEC		1000000		/* microseconds */
#define	TIME_TS_TICKS_PER_SEC		1000000000	/* nanoseconds */

/*
 * As per ISO/IEC 13818-1 Section 2.4.2.1
 */
#define	TIME_PCR_TICKS_PER_SEC		27000000	/* 27 MHz */
#define	TIME_DTS_TICKS_PER_SEC		90000		/* 90 KHz */

/*
 * Default Time ticks per sec
 */
#define	TIME_DEFAULT_TICKS_PER_SEC	TIME_TS_TICKS_PER_SEC

_FOREIGN_START

/***********************************************************************
 * Public Types
 ***********************************************************************/

typedef	s64		   Time;	/* in established ticks */

/*
 * Time per picture in 27 MHz ticks for various picture rates.
 * As per ISO/IEC 13818-1 Section 2.7.10
 */
typedef enum TimePerPicture {
    TIME_PER_PICTURE_23_976	= 1126125,
    TIME_PER_PICTURE_24		= 1125000,
    TIME_PER_PICTURE_25		= 1080000,
    TIME_PER_PICTURE_29_97	= 900900,
    TIME_PER_PICTURE_30		= 900000,
    TIME_PER_PICTURE_50		= 540000,
    TIME_PER_PICTURE_59_94	= 450450,
    TIME_PER_PICTURE_60		= 450000
} TimePerPicture;

typedef enum TimecodeType {
    TIMECODE_TYPE_NTSC_NON_DROP = 0,	    /* (M) NTSC non-drop frame */
    TIMECODE_TYPE_NTSC_DROP = 1,	    /* (M) NTSC drop frame */
    TIMECODE_TYPE_PAL = 2,		    /* (B, D, G, H, I) PAL */
    TIMECODE_TYPE_MPAL_NON_DROP = 3,	    /* (M) PAL non-drop (29.97 f/s) */
    TIMECODE_TYPE_MPAL_DROP = 4		    /* (M) PAL drop frame */
} TimecodeType;

typedef struct Timecode {
    TimecodeType	type;
    s32			hours;
    s8			minutes;
    s8			seconds;
    s8			frames;
} Timecode;

/***********************************************************************
 * Inline Support
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TIME_BODY)
#define	TIME_INLINE	static
#else		   /* defined(DO_INLINING) && !defined(TIME_BODY) */
#define	TIME_INLINE	extern
#endif		   /* defined(DO_INLINING) && !defined(TIME_BODY) */

/***********************************************************************
 * Routine Prototypes
 ***********************************************************************/

/*
 * Change ticks per sec in Time representation.
 * MUST BE CALLED BEFORE ANY OTHER Time routines.
 * Returns old timeTicksPerSec.
 */
TIME_INLINE s64 timeSetTicksPerSec(s64 newTicksPerSec);

/*
 * Returns current ticks per sec for time representation
 */
TIME_INLINE s64 timeGetTicksPerSec(void);

/*
 * Time of day in nanoseconds after Jan 1, 1970 UTC
 */
TIME_INLINE Time timeOfDay(void);

/*
 * Time relative to some OS defined fixed point (e.g. boot).
 *
 * This time is not subject to clock updates (i.e. ntpd shifts or
 * super-user "settime".  It is typically of higher accuracy and
 * resolution than time of day and is more appropriate for measuring
 * durations.  (This time is based on gethrtime().)
 *
 * timeFromBoot() can NOT be directly compared with timeOfDay()!
 */
TIME_INLINE Time timeFromBoot(void);

/*
 * Basic time arithmetic
 *
 * timeRelToAbs() -- conversion of duration to future date
 * timeDistance() -- abs(time1 - time2)
 */
TIME_INLINE Time timeRelToAbs(double secsOffset);
TIME_INLINE Time timeDistance(Time time1, Time time2);

/*
 * Conversion to/from floating point seconds
 */
TIME_INLINE Time timeFromDouble(double secs);
TIME_INLINE double timeToDouble(Time time);

/*
 * Conversion to/from other common ticks
 */
TIME_INLINE Time timeFromNsecs(s64 nsecs);
TIME_INLINE s64 timeToNsecs(Time time);

TIME_INLINE Time timeFromUsecs(s64 usecs);
TIME_INLINE s64 timeToUsecs(Time time);

/*
 * Conversion to/from arbitrary ticks
 */
TIME_INLINE Time timeFromTicks(s64 ticks, s32 ticksPerSec);
TIME_INLINE s64 timeToTicks(Time time, s32 ticksPerSec);

/*
 * Conversion to/from common OS representations
 */
TIME_INLINE Time timeFromTimeval(struct timeval tv);
TIME_INLINE struct timeval timeToTimeval(Time time);

TIME_INLINE Time timeFromTimestruc(timestruc_t ts);
TIME_INLINE timestruc_t timeToTimestruc(Time time);

TIME_INLINE Time timeFromTimet(time_t secs);
TIME_INLINE time_t timeToTimet(Time time);

/*
 * Conversion to/from common Video representations
 *
 * Dts is 90 KHz ticks.
 * Pcr is 27 MHz ticks.
 *
 * NOTE: TimePerPicture period is ALWAYS assumed to be in 27 MHz ticks
 * regardless of Time tick value.
 */
TIME_INLINE Time timeFromPcr(s64 pcr);
TIME_INLINE s64 timeToPcr(Time time);

TIME_INLINE Time timeFromDts(s64 dts);
TIME_INLINE s64 timeToDts(Time time);

TIME_INLINE Time timeFromPictureNum(s32 pictureNum, TimePerPicture period);
TIME_INLINE s32 timeToPictureNum(Time time, TimePerPicture period);

extern Time timeFromTimecode(Timecode timecode);
extern Timecode timeToTimecode(Time time, TimecodeType type);

/*
 * Conversion to string in form "hh:mm:ss.sss"
 *
 * If buf is null, or buflen is insufficient; a per-thread static
 * buffer will be allocated.  Returns pointer to formatted string.
 */
extern const char *
timeToHMS(Time time, char *buf, size_t buflen);

/***********************************************************************
 * Inline Function Bodies
 ***********************************************************************/

#if	defined(DO_INLINING) && ! defined(TIME_BODY)
#define	TIME_HEADER
#include "cobjs/Time.c"
#undef	TIME_HEADER
#endif		   /* defined(DO_INLINING) && !defined(TIME_BODY) */

_FOREIGN_END

#endif					   /* _COBJS_TIME_H */
