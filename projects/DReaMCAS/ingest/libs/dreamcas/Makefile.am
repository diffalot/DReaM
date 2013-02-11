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
# $(@)Makefile.am $Revision: 1.1 $ $Date: 2006/07/15 00:26:54 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#

VERSION=@MPEG2TS_DEMUX_LIB_CURRENT@:@MPEG2TS_DEMUX_LIB_REVISION@:@MPEG2TS_DEMUX_LIB_AGE@

lib_LTLIBRARIES=libdreamcas.la

libdreamcas_la_SOURCES=Ecm.h Ecm.cc Emm.h Emm.cc EcmWriter.h EcmWriter.cc Scrambler.h \
					   Scrambler.cc EcmGenerator.h EcmGenerator.cc

libdreamcas_la_CPPFLAGS=-D_REENTRANT @CACRYPTO_CPPFLAGS@ -I@BOOST_INCDIR@					   


