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
# $(@)Makefile.am $Revision: 1.1.1.1 $ $Date: 2006/04/17 22:47:31 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
VERSION=@MMP_LIB_CURRENT@:@MMP_LIB_REVISION@:@MMP_LIB_AGE@

lib_LTLIBRARIES=libmmp.la
bin_PROGRAMS=MkCrc32Table

MkCrc32Table_SOURCES=MkCrc32Table.c
MkCrc32Table_CFLAGS=-v -Xa

# For creating Crc32Table.c
Crc32Table.c: MkCrc32Table
	./MkCrc32Table > Crc32Table.c	

libmmp_la_SOURCES = \
    Context.h Crc32Table.h Mmp.h Mpeg1Pes.h Mpeg1Sys.h Mpeg2.h \
    Mpeg2Pat.h Mpeg2Pes.h Mpeg2Pmt.h Mpeg2Ps.h Mpeg2Psi.h Mpeg2Ts.h \
    Pipe.h Mpeg2VideoEs.h  Mpeg1AudioEs.h  metadata.h \
    Context.c Crc32Table.c Mmp.c Mpeg1Pes.c Mpeg1Sys.c Mpeg2Pat.c \
    Mpeg2Pes.c Mpeg2Pmt.c Mpeg2Ps.c Mpeg2Psi.c Mpeg2Ts.c Pipe.c I18N.c \
    Mpeg2VideoEs.c Mpeg1AudioEs.c metadata.c

libmmp_la_LDFLAGS= -version-info $(VERSION) -lthread -lpthread -lc -lm
libmmp_la_CFLAGS=-D_REENTRANT -D__EXTENSIONS__ -Dmmp_TEXT_DOMAIN=\"libmmp.so.1\" -DASSERTS -DTARGETNAME=\"libmmp.so.1\"
libmmp_la_LIBADD=../cobjs/libcobjs.la
