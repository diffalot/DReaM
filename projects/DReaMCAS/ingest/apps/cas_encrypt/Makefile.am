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
# $(@)Makefile.am $Revision: 1.3 $ $Date: 2006/09/29 18:17:39 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#

#bin_PROGRAMS=encryptTs oldEncryptTs 
bin_PROGRAMS=encryptTs emmEncrypt 
#oldEncryptTs_SOURCES=main.c meta.c ecmfile.h
##oldEncryptTs_CPPFLAGS = -DDO_INLINING @CACRYPTO_CPPFLAGS@
#oldEncryptTs_CFLAGS =  -D_REENTRANT
#oldEncryptTs_LDADD=$(top_builddir)/libs/cas/libcas.la \
#	$(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la \
#	$(top_builddir)/libs/meta/libmeta.la
#oldEncryptTs_LDFLAGS=@CACRYPTO_LDFLAGS@
#oldEncryptTs_LINTFLAGS= -erroff=E_P_USE_SUSP,E_ASSGN_NEVER_USED

encryptTs_SOURCES=EncryptTs.cc
encryptTs_CPPFLAGS = -DDO_INLINING @CACRYPTO_CPPFLAGS@
encryptTs_CXXFLAGS = -D_REENTRANT -I@BOOST_INCDIR@ 
encryptTs_LDADD = $(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la \
	$(top_builddir)/libs/meta/libmeta.la \
	$(top_builddir)/libs/dreamcas/libdreamcas.la
encryptTs_LDFLAGS=@CACRYPTO_LDFLAGS@

emmEncrypt_SOURCES=EmmEncrypt.cc
emmEncrypt_CPPFLAGS = -DDO_INLINING @CACRYPTO_CPPFLAGS@
emmEncrypt_CXXFLAGS = -D_REENTRANT -I@BOOST_INCDIR@ 
emmEncrypt_LDADD = $(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la \
	$(top_builddir)/libs/meta/libmeta.la \
	$(top_builddir)/libs/dreamcas/libdreamcas.la
emmEncrypt_LDFLAGS=@CACRYPTO_LDFLAGS@
