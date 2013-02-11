#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ca_dec, 1.0.0, [dev@dream.dev.java.net])
AM_INIT_AUTOMAKE(ca_dec, 1.0.0)
AC_CONFIG_SRCDIR([ca_dec.h])

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
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyname memset socket strchr strdup strstr])

# Checks for curl 
AC_ARG_WITH(curl-install-path,
             [    --with-curl-install-path=PATH path where curl can be found]
            )
if test -n "${with_curl_install_path}";
then
    AC_MSG_CHECKING(for libcurl in ${with_curl_install_path}/lib)
    curl_tree="`cd ${with_curl_install_path} 2>/dev/null && pwd`"
    if test -z "${curl_tree}"; then
        AC_MSG_ERROR([cannot cd to ${curl_tree}]) 
    fi
dnl   if test -f "${curl_tree}/lib/libcurl.so"; then
dnl       AC_MSG_RESULT(${curl_tree}/lib/libcurl.so)
dnl       CFLAGS="${CFLAGS} -L${curl_tree}/lib -lcurl"
dnl   else
dnl       AC_MSG_RESULT(no)
dnl       AC_MSG_ERROR([cannot find ${curl_tree}/lib/libcurl.so in ${with_curl_install_path}])        
dnl   fi

    AC_MSG_CHECKING(for curl/curl.h in ${with_curl_install_path}/include)
    if test -f "${curl_tree}/include/curl/curl.h"; then
        AC_MSG_RESULT(${curl_tree}/include/curl/curl.h)
        CPPFLAGS="${CPPFLAGS} -I${curl_tree}/include"
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR([cannot find ${curl_tree}/include/curl/curl.h in ${with_curl_install_path}])
    fi
else
dnl    AC_CHECK_LIB(curl,curl_easy_init,[],
dnl             [AC_MSG_ERROR([libcurl required])])
    AC_CHECK_HEADER([curl/curl.h],[],[AC_MSG_ERROR([Cannot find curl/curl.h, exiting])])
fi 

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
