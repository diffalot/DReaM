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
 * $(@)Time.c $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
 * 
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */
/*
 * Copyright (c) 1997 by Sun Microsystems, Inc.
 */

/*
 * Time.c -- Fixed tick time representation
 */

#pragma ident "@(#)Time.c 1.4	99/03/22 SMI"

#if	!defined(TIME_HEADER)
#define	TIME_BODY
#define	TIME_INLINE		extern
#include "cobjs/Time.h"
#endif					   /* !defined(TIME_HEADER) */

#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <time.h>

#include "cobjs/Log.h"
#include "cobjs/Macros.h"

/***********************************************************************
 * Defines
 ***********************************************************************/

/*
 * Timecode Frames per sec
 * (Not true frame rate).
 */
#define	NTSC_FRAMES_PER_SEC	30
#define	MPAL_FRAMES_PER_SEC	30
#define	PAL_FRAMES_PER_SEC	25

/***********************************************************************
 * Private Data
 ***********************************************************************/

static thread_key_t bufKey;

static s64 timeTicksPerSec = TIME_DEFAULT_TICKS_PER_SEC;
static Boolean wasCalled = FALSE;

/***********************************************************************
 * Initialization
 ***********************************************************************/

static void
timeInit(void)
{
    ABORT_IF_ERRNO(thr_keycreate(&bufKey, free));
}

#pragma	init(timeInit)

/*************************************************************************
 * Inline Functions
 *************************************************************************/

TIME_INLINE s64
timeSetTicksPerSec(s64 newTicksPerSec)
{
    s64 oldTicksPerSec = timeTicksPerSec;
    ABORT_IF_FALSE(! wasCalled);
    timeTicksPerSec = newTicksPerSec;
    return oldTicksPerSec;
}

/*
 * Returns current ticks per sec for time representation
 */
TIME_INLINE s64
timeGetTicksPerSec(void)
{
    wasCalled = TRUE;
    return timeTicksPerSec;
}

TIME_INLINE Time
timeOfDay(void)
{
    struct timeval tv;

    ABORT_IF_ERRNO(gettimeofday(&tv, NULL));
    return timeFromTimeval(tv);
}

TIME_INLINE Time
timeFromBoot(void)
{
    return timeFromNsecs(gethrtime());
}

TIME_INLINE Time
timeRelToAbs(double secsOffset)
{
    return timeOfDay() + timeFromDouble(secsOffset);
}

TIME_INLINE Time
timeDistance(Time time1, Time time2)
{
    Time diff = time1 - time2;
    return diff >= 0 ? diff : -diff;
}

TIME_INLINE Time
timeFromDouble(double secs)
{
    wasCalled = TRUE;
    return (Time)(secs * timeTicksPerSec);
}

TIME_INLINE double
timeToDouble(Time time)
{
    wasCalled = TRUE;
    return (double) time / timeTicksPerSec;
}

TIME_INLINE Time
timeFromTicks(s64 ticks, s32 ticksPerSec)
{
    Time time;

    wasCalled = TRUE;
    if (ticksPerSec == timeTicksPerSec) {
	time = ticks;
    } else {
	s64 secs = ticks / ticksPerSec;
	time = secs * timeTicksPerSec;

	ticks -= secs * ticksPerSec;
	time += DIV_ROUND(ticks * timeTicksPerSec, ticksPerSec);
    }
    return time;
}

TIME_INLINE s64
timeToTicks(Time time, s32 ticksPerSec)
{
    s64 ticks;

    wasCalled = TRUE;
    if (ticksPerSec == timeTicksPerSec) {
	ticks = time;
    } else {
	s64 secs = time / timeTicksPerSec;

	ticks = secs * ticksPerSec;
	time -= secs * timeTicksPerSec;
	ticks += DIV_ROUND(time * ticksPerSec, timeTicksPerSec);
    }
    return ticks;
}
 
TIME_INLINE Time
timeFromTimeval(struct timeval tv)
{
    return timeFromTicks(((s64) tv.tv_sec * TIME_TV_TICKS_PER_SEC) + tv.tv_usec,
	    TIME_TV_TICKS_PER_SEC);
}

TIME_INLINE struct timeval
timeToTimeval(Time time)
{
    struct timeval tv;

    wasCalled = TRUE;
    tv.tv_sec = time / timeTicksPerSec;
    time -= (Time) tv.tv_sec * timeTicksPerSec;
    tv.tv_usec = DIV_ROUND(time * TIME_TV_TICKS_PER_SEC, timeTicksPerSec);
    return tv;
}

TIME_INLINE Time
timeFromTimestruc(timestruc_t ts)
{
    wasCalled = TRUE;
    return timeFromTicks(((s64) ts.tv_sec * TIME_TS_TICKS_PER_SEC)
	+ ts.tv_nsec, TIME_TS_TICKS_PER_SEC);
}

TIME_INLINE timestruc_t
timeToTimestruc(Time time)
{
    timestruc_t ts;

    wasCalled = TRUE;
    ts.tv_sec = time / timeTicksPerSec;
    time -= (Time) ts.tv_sec * timeTicksPerSec;
    ts.tv_nsec = DIV_ROUND(time * TIME_TS_TICKS_PER_SEC, timeTicksPerSec);
    return ts;
}

TIME_INLINE Time
timeFromNsecs(s64 nsecs)
{
    wasCalled = TRUE;
    return timeFromTicks(nsecs, TIME_TS_TICKS_PER_SEC);
}

TIME_INLINE s64
timeToNsecs(Time time)
{
    wasCalled = TRUE;
    return timeToTicks(time, TIME_TS_TICKS_PER_SEC);
}

TIME_INLINE Time
timeFromUsecs(s64 usecs)
{
    wasCalled = TRUE;
    return timeFromTicks(usecs, TIME_TV_TICKS_PER_SEC);
}

TIME_INLINE s64
timeToUsecs(Time time)
{
    wasCalled = TRUE;
    return timeToTicks(time, TIME_TV_TICKS_PER_SEC);
}

TIME_INLINE Time
timeFromTimet(time_t secs)
{
    wasCalled = TRUE;
    return (Time)(secs) * timeTicksPerSec;
}

TIME_INLINE time_t
timeToTimet(Time time)
{
    wasCalled = TRUE;
    return (time_t) ((double) time / timeTicksPerSec);
}

TIME_INLINE Time
timeFromPcr(s64 pcr)
{
    wasCalled = TRUE;
    return timeFromTicks(pcr, TIME_PCR_TICKS_PER_SEC);
}

TIME_INLINE s64
timeToPcr(Time time)
{
    wasCalled = TRUE;
    return timeToTicks(time, TIME_PCR_TICKS_PER_SEC);
}

TIME_INLINE Time
timeFromDts(s64 dts)
{
    wasCalled = TRUE;
    return timeFromTicks(dts, TIME_DTS_TICKS_PER_SEC);
}

TIME_INLINE s64
timeToDts(Time time)
{
    wasCalled = TRUE;
    return timeToTicks(time, TIME_DTS_TICKS_PER_SEC);
}

TIME_INLINE Time
timeFromPictureNum(s32 pictureNum, TimePerPicture period)
{
    wasCalled = TRUE;
    return timeFromPcr((s64) pictureNum * period);
}

TIME_INLINE s32
timeToPictureNum(Time time, TimePerPicture period)
{
    wasCalled = TRUE;
    return timeToPcr(time) / period;
}

#if	!defined(TIME_HEADER)

Time
timeFromTimecode(Timecode timecode)
{
    TimePerPicture period;
    s32 pictureNum;
    s32 minutes;

    ASSERT(timecode.hours >= 0);
    ASSERT(timecode.minutes >= 0 && timecode.minutes < 60);
    ASSERT(timecode.seconds >= 0 && timecode.seconds < 60);
    wasCalled = TRUE;
    switch (timecode.type) {
    case TIMECODE_TYPE_NTSC_NON_DROP:
	ASSERT(timecode.frames >= 0 && timecode.frames < NTSC_FRAMES_PER_SEC);
	period = TIME_PER_PICTURE_29_97;
	pictureNum = timecode.frames
	    + NTSC_FRAMES_PER_SEC * timecode.seconds
	    + NTSC_FRAMES_PER_SEC * 60 * timecode.minutes
	    + NTSC_FRAMES_PER_SEC * 60 * 60 * timecode.hours;
	break;
    case TIMECODE_TYPE_NTSC_DROP:
	ASSERT(timecode.frames >= 0 && timecode.frames < NTSC_FRAMES_PER_SEC);
	period = TIME_PER_PICTURE_29_97;
	pictureNum = timecode.frames
	    + NTSC_FRAMES_PER_SEC * timecode.seconds
	    + NTSC_FRAMES_PER_SEC * 60 * timecode.minutes
	    + NTSC_FRAMES_PER_SEC * 60 * 60 * timecode.hours;
	/*
	 * Frames 0 and 1 are dropped at start of every minute except
	 * minutes 0, 10, 20, 30, 40, and 50
	 */
	ASSERT(timecode.frames >= 2 || timecode.seconds != 0
		|| (timecode.minutes % 10) == 0);
	minutes = (timecode.hours * 60) + timecode.minutes;
	pictureNum -= 2 * 9 * (minutes / 10);
	pictureNum -= 2 * (minutes % 10);
	break;
    case TIMECODE_TYPE_PAL:
	ASSERT(timecode.frames >= 0 && timecode.frames < PAL_FRAMES_PER_SEC);
	period = TIME_PER_PICTURE_25;
	pictureNum = timecode.frames
	    + PAL_FRAMES_PER_SEC * timecode.seconds
	    + PAL_FRAMES_PER_SEC * 60 * timecode.minutes
	    + PAL_FRAMES_PER_SEC * 60* 60 * timecode.hours;
	break;
    case TIMECODE_TYPE_MPAL_NON_DROP:
	ASSERT(timecode.frames >= 0 && timecode.frames < MPAL_FRAMES_PER_SEC);
	period = TIME_PER_PICTURE_29_97;
	pictureNum = timecode.frames
	    + MPAL_FRAMES_PER_SEC * timecode.seconds
	    + MPAL_FRAMES_PER_SEC * 60 * timecode.minutes
	    + MPAL_FRAMES_PER_SEC * 60* 60 * timecode.hours;
	break;
    case TIMECODE_TYPE_MPAL_DROP:
	ASSERT(timecode.frames >= 0 && timecode.frames < MPAL_FRAMES_PER_SEC);
	period = TIME_PER_PICTURE_29_97;
	pictureNum = timecode.frames
	    + MPAL_FRAMES_PER_SEC * timecode.seconds
	    + MPAL_FRAMES_PER_SEC * 60 * timecode.minutes
	    + MPAL_FRAMES_PER_SEC * 60* 60 * timecode.hours;
	/*
	 * First 4 frames (0 - 3) are dropped at start of every EVEN minute
	 * except minutes 0, 20, and 40
	 */
	ASSERT(timecode.frames >= 4 || timecode.seconds != 0
		|| (timecode.minutes & 1) != 0
		|| (timecode.minutes % 20) == 0);
	minutes = (timecode.hours * 60) + timecode.minutes;
	pictureNum -= 4 * (10 - 1) * (minutes / 20);
	pictureNum -= 4 * ((minutes % 20) / 2);
	break;
    default:
#if __lint
	pictureNum = 0;
	period = 0;
#endif	/* __lint */
	ABORT("Invalid timecode type");
    }
    return timeFromPictureNum(pictureNum, period);

}

Timecode
timeToTimecode(Time time, TimecodeType type)
{
    s64 pictureNum;
    s32 hours;
    s32 minutes;
    s32 seconds;
    s32 frames;
    s32 firstFrame;
    s32 picsPer10Min;
    s32 min10s;
    s32 dropMins;
    s32 picsPer20Min;
    s32 min20s;
    s32 dropDblMins;
    Timecode timecode;

    switch (type) {
    case TIMECODE_TYPE_NTSC_NON_DROP:
	pictureNum = timeToPictureNum(time, TIME_PER_PICTURE_29_97);
	hours = pictureNum / (NTSC_FRAMES_PER_SEC * 60 * 60);
	pictureNum -= hours * (NTSC_FRAMES_PER_SEC * 60 * 60);
	minutes = pictureNum / (NTSC_FRAMES_PER_SEC * 60);
	pictureNum -= minutes * (NTSC_FRAMES_PER_SEC * 60);
	seconds = pictureNum / NTSC_FRAMES_PER_SEC;
	pictureNum -= seconds * NTSC_FRAMES_PER_SEC;
	frames = pictureNum;
	ASSERT(frames >= 0 && frames < NTSC_FRAMES_PER_SEC);
	break;
    case TIMECODE_TYPE_NTSC_DROP:
	pictureNum = timeToPictureNum(time, TIME_PER_PICTURE_29_97);
	picsPer10Min = 60 * NTSC_FRAMES_PER_SEC
	    + 9 * (60 * NTSC_FRAMES_PER_SEC - 2);
	min10s = pictureNum / picsPer10Min;
	pictureNum -= min10s * picsPer10Min;
	hours = min10s / 6;
	min10s -= hours * 6;
	minutes = min10s * 10;
	firstFrame = 0;
	if (pictureNum >= 60 * NTSC_FRAMES_PER_SEC) {
	    minutes += 1;
	    pictureNum -= 60 * NTSC_FRAMES_PER_SEC;
	    firstFrame = 2;
	    dropMins = pictureNum / (60 * NTSC_FRAMES_PER_SEC - 2);
	    minutes += dropMins;
	    pictureNum -= dropMins * (60 * NTSC_FRAMES_PER_SEC - 2);
	}
	seconds = (pictureNum + firstFrame) / NTSC_FRAMES_PER_SEC;
	pictureNum -= (seconds * NTSC_FRAMES_PER_SEC) - firstFrame;
	frames = pictureNum;
	ASSERT(frames >= 0 && frames < NTSC_FRAMES_PER_SEC);
	break;
    case TIMECODE_TYPE_PAL:
	pictureNum = timeToPictureNum(time, TIME_PER_PICTURE_25);
	hours = pictureNum / (PAL_FRAMES_PER_SEC * 60 * 60);
	pictureNum -= hours * (PAL_FRAMES_PER_SEC * 60 * 60);
	minutes = pictureNum / (PAL_FRAMES_PER_SEC * 60);
	pictureNum -= minutes * (PAL_FRAMES_PER_SEC * 60);
	seconds = pictureNum / PAL_FRAMES_PER_SEC;
	pictureNum -= seconds * PAL_FRAMES_PER_SEC;
	frames = pictureNum;
	ASSERT(frames >= 0 && frames < PAL_FRAMES_PER_SEC);
	break;
    case TIMECODE_TYPE_MPAL_NON_DROP:
	pictureNum = timeToPictureNum(time, TIME_PER_PICTURE_29_97);
	hours = pictureNum / (MPAL_FRAMES_PER_SEC * 60 * 60);
	pictureNum -= hours * (MPAL_FRAMES_PER_SEC * 60 * 60);
	minutes = pictureNum / (MPAL_FRAMES_PER_SEC * 60);
	pictureNum -= minutes * (MPAL_FRAMES_PER_SEC * 60);
	seconds = pictureNum / MPAL_FRAMES_PER_SEC;
	pictureNum -= seconds * MPAL_FRAMES_PER_SEC;
	frames = pictureNum;
	ASSERT(frames >= 0 && frames < MPAL_FRAMES_PER_SEC);
	break;
    case TIMECODE_TYPE_MPAL_DROP:
	pictureNum = timeToPictureNum(time, TIME_PER_PICTURE_29_97);
	picsPer20Min = 2 * 60 * MPAL_FRAMES_PER_SEC
	    + 9 * (2 * 60 * MPAL_FRAMES_PER_SEC - 4);
	min20s = pictureNum / picsPer20Min;
	pictureNum -= min20s * picsPer20Min;
	hours = min20s / 3;
	min20s -= hours * 3;
	minutes = min20s * 20;
	firstFrame = 0;
	if (pictureNum >= 2 * 60 * MPAL_FRAMES_PER_SEC) {
	    minutes += 2;
	    pictureNum -= 2 * 60 * MPAL_FRAMES_PER_SEC;
	    firstFrame = 4;
	    dropDblMins = pictureNum / (2 * 60 * MPAL_FRAMES_PER_SEC - 4);
	    minutes += dropDblMins * 2;
	    pictureNum -= dropDblMins * (2 * 60 * MPAL_FRAMES_PER_SEC - 4);
	}
	if (pictureNum > 60 * MPAL_FRAMES_PER_SEC - firstFrame) {
	    minutes += 1;
	    pictureNum -= 60 * MPAL_FRAMES_PER_SEC - firstFrame;
	    firstFrame = 0;
	}
	seconds = (pictureNum + firstFrame) / MPAL_FRAMES_PER_SEC;
	pictureNum -= (seconds * MPAL_FRAMES_PER_SEC) - firstFrame;
	frames = pictureNum;
	ASSERT(frames >= 0 && frames < MPAL_FRAMES_PER_SEC);
	break;
    default:
#if	__lint
	hours = 0;
	minutes = 0;
	seconds = 0;
	frames = 0;
#endif	/* __lint */
	ABORT("Invalid timecode type");
    }
    ASSERT(minutes >= 0 && minutes < 60);
    ASSERT(seconds >= 0 && seconds < 60);
    timecode.type = type;
    timecode.hours = hours;
    timecode.minutes = minutes;
    timecode.seconds = seconds;
    timecode.frames = frames;
    return timecode;
}

const char *
timeToHMS(Time time, char *buf, size_t buflen)
{
    Time hourTime = timeFromTicks(3600, 1);
    Time minTime = timeFromTicks(60, 1);
    s32 hours;
    s32 mins;
    double secs;

    if (buf == NULL || buflen < 20) {
	buflen = 20;
	if (thr_getspecific(bufKey, (void **) &buf) != 0 || buf == NULL) {
	    buf = NEW(char, buflen);
	    ABORT_IF_ERRNO(thr_setspecific(bufKey, buf));
	}
    }

    hours = time / hourTime;
    time -= hours * hourTime;
    mins = time / minTime;
    time -= mins * minTime;
    secs = timeToDouble(time);
    (void) snprintf(buf, buflen, "%ld:%02ld:%06.3f", hours, mins, secs);
    return buf;
}
#endif	/* !defined(TIME_HEADER) */
