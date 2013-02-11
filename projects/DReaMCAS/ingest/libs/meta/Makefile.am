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
# $(@)Makefile.am $Revision: 1.2 $ $Date: 2007/01/30 23:24:14 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
VERSION=@META_LIB_CURRENT@:@META_LIB_REVISION@:@META_LIB_AGE@

lib_LTLIBRARIES=libmeta.la

libmeta_la_SOURCES=metadata.c metadata.h

#libmeta_la_LDFLAGS= -version-info $(VERSION) -lthread -lpthread -lm -lc
libmeta_la_LDFLAGS= -version-info $(VERSION) -lpthread -lm -lc
