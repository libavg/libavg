AM_CPPFLAGS = -I.. @PTHREAD_CFLAGS@

ALL_H = AudioEngine.h AudioBuffer.h AudioParams.h \
        Dynamics.h IProcessor.h AudioMsg.h AudioSource.h

TESTS = testlimiter

noinst_LTLIBRARIES = libaudio.la
noinst_PROGRAMS = testlimiter

libaudio_la_SOURCES = AudioEngine.cpp AudioBuffer.cpp AudioParams.cpp AudioMsg.cpp \
        AudioSource.cpp $(ALL_H)

testlimiter_SOURCES = testlimiter.cpp $(ALL_H)
testlimiter_LDADD = ./libaudio.la ../base/libbase.la \
        ../base/triangulate/libtriangulate.la \
        @BOOST_THREAD_LIBS@ @PTHREAD_LIBS@
