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
# $(@)Makefile.am $Revision: 1.2 $ $Date: 2006/07/15 00:02:40 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
VERSION=@MPEG2TS_DEMUX_LIB_CURRENT@:@MPEG2TS_DEMUX_LIB_REVISION@:@MPEG2TS_DEMUX_LIB_AGE@

lib_LTLIBRARIES=libmpeg2ts_demux.la

libmpeg2ts_demux_la_SOURCES = \
            InputBitBuffer.h OutputBitBuffer.h Filter.h PsiFilter.h Crc.h Descriptors.h \
            PesFilter.h TsDemuxer.h VideoEsFilter.h  mpeg2ts_demux_stub.h \
            TsFilter.cc Crc32.cc Descriptors.cc InputBitBuffer.cc \
            OutputBitBuffer.cc PesFilter.cc PsiFilter.cc TsDemuxer.cc VideoEsFilter.cc mpeg2ts_demux_stub.cc \
			FileWriter.h FileWriter.cc 

libmpeg2ts_demux_la_LDFLAGS= -version-info $(VERSION) -lc
libmpeg2ts_demux_la_CXXFLAGS=-D_REENTRANT -D__EXTENSIONS__ \
	-Dmpeg2ts_demux_TEXT_DOMAIN=\"libmpeg2ts_demux.so.1\" -DASSERTS \
	-DTARGETNAME=\"libmpeg2ts_demux.so.1\" -I@BOOST_INCDIR@


SUBDIRS = . tests


