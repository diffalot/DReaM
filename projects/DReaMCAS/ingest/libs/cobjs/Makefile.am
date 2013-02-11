#
# The contents of this file are subject to the terms
# of the Common Development and Distribution License
# (the "License").  You may not use this file except
# in compliance with the License.
#
# You can obtain a copy of the license at
# http://www.opensource.org/licenses/cddl1.php
# See the License for the specific language governing
# permissions and limitations under the License.
#
# When distributing Covered Code, include this CDDL
# HEADER in each file and include the License file at
# http://www.opensource.org/licenses/cddl1.php.  If 
# applicable, add the following below this CDDL HEADER, 
# with the fields enclosed by brackets "[]" replaced 
# with your own identifying information: 
# Portions Copyright [yyyy]
# [name of copyright owner]
# 

#
# $(@)Makefile.am $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:30 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
VERSION=@COBJ_LIB_CURRENT@:@COBJ_LIB_REVISION@:@COBJ_LIB_AGE@

lib_LTLIBRARIES=libcobjs.la

libcobjs_la_SOURCES= Actor.c ArrayOf.c BinarySearch.c Bits.c CheckSum.c CircBuf.c \
	Delay.c Deque.c DequeOf.c Doors.c EventSim.c HashTable.c List.c \
	Log.c Macros.c MetricIo.c Option.c Package.c PriQueue.c Queue.c \
	QueueOf.c RetCode.c SchedCall.c SortTable.c StatMgr.c String.c \
	Thread.c Time.c Timer.c I18N.c \
    Actor.h ArrayOf.h BinarySearch.h Bits.h CheckSum.h CircBuf.h \
	Delay.h Deque.h DequeOf.h Doors.h EventSim.h HashTable.h List.h \
	Log.h Macros.h MetricIo.h Option.h Package.h PriQueue.h Queue.h \
	QueueOf.h RetCode.h SchedCall.h SortTable.h StatMgr.h String.h \
	Thread.h Time.h Timer.h Foreign.h Inline.h Types.h

libcobjs_la_LDFLAGS= -version-info $(VERSION) -ldoor
libcobjs_la_CFLAGS= -D_REENTRANT -D__EXTENSIONS__ -Dcobjs_TEXT_DOMAIN=\"libcobjs.so.1\" -DASSERTS  -DTARGETNAME=\"libcobjs.so.1\"
library_include_HEADERS = Foreign.h Types.h
library_includedir=$(includedir)
