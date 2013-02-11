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
# $(@)Makefile.am $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:29 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
VERSION=@MPEG2TS_DEMUX_LIB_CURRENT@:@MPEG2TS_DEMUX_LIB_REVISION@:@MPEG2TS_DEMUX_LIB_AGE@

lib_LTLIBRARIES=libcas.la

libcas_la_SOURCES=cas_parser.c cas_sys.c cas_injection.c \
                  cas_lib.h cas_sys.h
libcas_la_LDFLAGS= -version-info $(VERSION) -lthread -lpthread -lc -lm @CACRYPTO_LDFLAGS@
libcas_la_CFLAGS=-D_REENTRANT -D__EXTENSIONS__ -Dcas_TEXT_DOMAIN=\"libcas.so.1\" -DASSERTS -DTARGETNAME=\"libcas.so.1\"
libcas_la_CPPFLAGS=-DMTHREAD -DHACK_DROP_AUDIO_VIDEO -DMT_DEFAULT_BUFFER_SIZE=512000 -DMT_OUTPUT_BUFFER_SIZE=1880000 @CACRYPTO_CPPFLAGS@ 
libcas_la_LIBADD=../mmp/libmmp.la
