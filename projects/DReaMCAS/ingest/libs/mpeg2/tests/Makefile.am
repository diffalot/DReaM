
bin_PROGRAMS=InputBitBufferTest OutputBitBufferTest demuxTs \
fileWriterTest printEmm

InputBitBufferTest_SOURCES=InputBitBufferTest.cc
InputBitBufferTest_CXXFLAGS=-D_REENTRANT -I@BOOST_INCDIR@
InputBitBufferTest_LDADD=$(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la

OutputBitBufferTest_SOURCES=OutputBitBufferTest.cc
OutputBitBufferTest_CXXFLAGS=-D_REENTRANT -I@BOOST_INCDIR@
OutputBitBufferTest_LDADD=$(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la

demuxTs_SOURCES=demux_ts.cc
demuxTs_CXXFLAGS=-D_REENTRANT -I@BOOST_INCDIR@
demuxTs_LDADD=$(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la

printEmm_SOURCES=PrintEmm.cc
printEmm_CXXFLAGS=-D_REENTRANT -I@BOOST_INCDIR@
printEmm_LDADD=$(top_builddir)/libs/mpeg2/OutputBitBuffer.o $(top_builddir)/libs/mpeg2/InputBitBuffer.o $(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la $(top_builddir)/libs/dreamcas/libdreamcas.la @CACRYPTO_LDFLAGS@

fileWriterTest_SOURCES=FileWriterTest.cc
fileWriterTest_CXXFLAGS=-D_REENTRANT -I@BOOST_INCDIR@
fileWriterTest_LDADD=$(top_builddir)/libs/mpeg2/libmpeg2ts_demux.la


