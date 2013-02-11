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
# $(@)configure.ac $Revision: 1.2 $ $Date: 2006/07/15 00:02:29 $
#
# Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
#
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(dream-ingest, 1.0.0, [dev@dream.dev.java.net])
AC_CONFIG_SRCDIR([libs/cas/cas_sys.h])
AM_INIT_AUTOMAKE([foreign no-define])

# 
# LIB_VERSION=         0:0:0
#                      | | |
#               +------+ | +---+
#               |        |     |
#             current:revision:age
#               |        |     |
#               |        |     +- increment only if additions made to prev
#               |        |        interface. (age = number of previous
#               |        |        interface versions that current interface
#               |        |        has backward compatibility with)
#               |        |        set to zero if interfaces have been removed
#               |        |        or changed
#               |        |
#               |        +- increment if source code has changed
#               |           set to zero if current is incremented
#               |
#               +- increment if there are ANY changes to the previous library
#                  interface.
COBJ_LIB_CURRENT=1
COBJ_LIB_REVISION=0
COBJ_LIB_AGE=0
AC_SUBST(COBJ_LIB_CURRENT)
AC_SUBST(COBJ_LIB_REVISION)
AC_SUBST(COBJ_LIB_AGE)

MMP_LIB_CURRENT=1
MMP_LIB_REVISION=0
MMP_LIB_AGE=0
AC_SUBST(MMP_LIB_CURRENT)
AC_SUBST(MMP_LIB_REVISION)
AC_SUBST(MMP_LIB_AGE)

MPEG2TS_DEMUX_LIB_CURRENT=0
MPEG2TS_DEMUX_LIB_REVISION=7
MPEG2TS_DEMUX_LIB_AGE=0
AC_SUBST(MPEG2TS_DEMUX_LIB_CURRENT)
AC_SUBST(MPEG2TS_DEMUX_LIB_REVISION)
AC_SUBST(MPEG2TS_DEMUX_LIB_AGE)

META_LIB_CURRENT=1
META_LIB_REVISION=0
META_LIB_AGE=0
AC_SUBST(META_LIB_CURRENT)
AC_SUBST(META_LIB_REVISION)
AC_SUBST(META_LIB_AGE)


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

if test "$CC" = "cc" -o "$CC" = "CC"; then
	CXXFLAGS=-library=stlport4
fi

AC_ARG_WITH(cacrypto,
             [--with-cacrypto=PATH path where cacrypto can be found]
            )
if test -n "${with_cacrypto}";
then
    AC_MSG_CHECKING(for libcacrypto in ${with_cacrypto}/lib)
    cacrypto_tree="`cd ${with_cacrypto} 2>/dev/null && pwd`"
    if test -z "${cacrypto_tree}"; then
        AC_MSG_ERROR([cannot cd to ${cacrypto_tree}]) 
    fi
    if test -f "${cacrypto_tree}/lib/libcacrypto.so"; then
        AC_MSG_RESULT(${cacrypto_tree}/lib/libcacrypto.so)
        CACRYPTO_LDFLAGS="-L${cacrypto_tree}/lib -lcacrypto"
        AC_SUBST(CACRYPTO_LDFLAGS)
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([cannot find ${cacrypto_tree}/lib/libcacrypto.so in ${with_cacrypto}])
    fi

    AC_MSG_CHECKING(for cacrypto/crypto.h in ${with_cacrypto}/include)
    if test -f "${cacrypto_tree}/include/ca_crypto/crypto.h"; then
        AC_MSG_RESULT(${cacrypto_tree}/include/ca_crypto/crypto.h)
        CACRYPTO_CPPFLAGS="-I${cacrypto_tree}/include"
        AC_SUBST(CACRYPTO_CPPFLAGS)
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([cannot find ${cacrypto_tree}/include/cacrypto/cyrpto.h in ${with_cacrypto}])
    fi
else
    AC_CHECK_LIB(cacrypto,encryptBufferData,[],
             [AC_MSG_ERROR([cacrypto library: libcacrypto required])])
    AC_CHECK_HEADER([ca_crypto/crypto.h],[],[AC_MSG_ERROR([Cannot find ca_crypto/crypto.h, exiting])])
fi 

AC_ARG_WITH(boost,[  --with-boost-dir=<location of Boost Library installation>])
AC_CHECK_FILE($with_boost_dir/include/boost-1_32/boost/shared_ptr.hpp,
		      [BOOST_INCDIR=$with_boost_dir/include/boost-1_32/],
			  [AC_MSG_ERROR([Configure Boost location using --with-boost-dir])])
AC_SUBST(BOOST_INCDIR)

# some flags used everywhere..
CPPFLAGS="-I\$(top_srcdir)/libs"

# Checks for libraries.

# Checks for header files.
# AC_HEADER_STDC
# AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_HEADER_STDBOOL
# AC_C_CONST
# AC_TYPE_SIZE_T

# Checks for library functions.
# AC_FUNC_MALLOC
# AC_CHECK_FUNCS([memmove memset strerror])

AC_CONFIG_FILES([Makefile
                libs/Makefile
                libs/cobjs/Makefile
                libs/mmp/Makefile
                libs/mpeg2/Makefile
				libs/mpeg2/tests/Makefile
                libs/cas/Makefile
                libs/meta/Makefile
				libs/dreamcas/Makefile
                apps/Makefile
                apps/cas_encrypt/Makefile
                ])

AC_OUTPUT

