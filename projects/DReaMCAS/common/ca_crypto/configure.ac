#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# software_version = 1.0.0

AC_PREREQ(2.59)
AC_INIT(ca_crypto, 1.0.0, [dev@dream.dev.java.net])
AC_CONFIG_SRCDIR([asym-crypto.c])
AM_INIT_AUTOMAKE(ca_crypto, 1.0.0)

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
LIB_CURRENT=0
LIB_REVISION=0
LIB_AGE=0
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memset])

# Checks for openssl 
#   - checks for libcrypto.so in openssl install tree (or default locations)
#   - checks for openssl/evp.h in openssl install tree
AC_ARG_WITH(openssl,
             [--with-openssl=PATH path where openssl can be found]
            )
if test -n "${with_openssl}";
then
    AC_MSG_CHECKING(for libcrypto in ${with_openssl}/lib)
    openssl_tree="`cd ${with_openssl} 2>/dev/null && pwd`"
    if test -z "${openssl_tree}"; then
        AC_MSG_ERROR([cannot cd to ${openssl_tree}]) 
    fi
    if test -f "${openssl_tree}/lib/libcrypto.so"; then
        AC_MSG_RESULT(${openssl_tree}/lib/libcrypto.so)
        CFLAGS="${CFLAGS} -L${openssl_tree}/lib -lcrypto"
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([cannot find ${openssl_tree}/lib/libcrypto.so in ${with_openssl}])
    fi

    AC_MSG_CHECKING(for openssl/evp.h in ${with_openssl}/include)
    if test -f "${openssl_tree}/include/openssl/evp.h"; then
        AC_MSG_RESULT(${openssl_tree}/include/openssl/evp.h)
        CPPFLAGS="${CPPFLAGS} -I${openssl_tree}/include"
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([cannot find ${openssl_tree}/include/openssl/evp.h in ${with_openssl}])
    fi
else
    AC_CHECK_LIB(crypto,EVP_aes_128_cfb,[],
             [AC_MSG_ERROR([openssl library: libcrypto required])])
    AC_CHECK_HEADER([openssl/evp.h],[],[AC_MSG_ERROR([Cannot find openssl/evp.h, exiting])])
fi 

# Check for cipher_ctr.c
# see README for details on why this is required
AC_MSG_CHECKING(for source file cipher_ctr.c)
if test -f cipher-ctr.c; then
    AC_MSG_RESULT(cipher-ctr.c)
else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([cipher-ctr.c source file required. See README for details])
fi



AC_CONFIG_FILES([Makefile])
AC_OUTPUT
