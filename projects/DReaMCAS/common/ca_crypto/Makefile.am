VERSION=@LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@

lib_LTLIBRARIES = libcacrypto.la
libcacrypto_la_SOURCES = crypto.c asym-crypto.c cipher-ctr.c crypto.h
libcacrypto_la_LDFLAGS= -version-info $(VERSION) 
library_includedir=$(includedir)/ca_crypto
library_include_HEADERS = crypto.h
AUTOMAKE_OPTIONS = foreign
