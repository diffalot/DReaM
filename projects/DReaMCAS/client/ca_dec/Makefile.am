VERSION=@LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@

# lib_LTLIBRARIES = libcadec.la
# libcadec_la_SOURCES = casparse.c key_client.c config.c
# libcadec_la_LDFLAGS = -version-info $(VERSION_INFO) -release $(RELEASE_INFO)

lib_LTLIBRARIES = libcadec.la
libcadec_la_SOURCES = casparse.c key_client.c config.c
libcadec_la_LDFLAGS = -version-info $(VERSION)
library_includedir=$(includedir)/ca_dec
library_include_HEADERS = ca_dec.h
AUTOMAKE_OPTIONS = foreign
