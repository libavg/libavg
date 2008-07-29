
INCLUDES = -I.. @XML2_CFLAGS@ @PTHREAD_CFLAGS@
ALL_H = FileHelper.h CmdLine.h Exception.h Logger.h ConfigMgr.h ObjectCounter.h \
        XMLHelper.h TimeSource.h ProfilingZone.h Profiler.h ThreadProfiler.h \
        ScopeTimer.h IFrameListener.h Test.h TestSuite.h OSHelper.h \
        Queue.h WorkerThread.h Command.h ObjectCounter.h Point.h Rect.h \
        Directory.h DirEntry.h StringHelper.h MathHelper.h GeomHelper.h \
        CubicSpline.h BicubicSpline.h 

TESTS=testbase

noinst_LTLIBRARIES = libbase.la
libbase_la_SOURCES = FileHelper.cpp CmdLine.cpp Exception.cpp Logger.cpp \
    ConfigMgr.cpp XMLHelper.cpp TimeSource.cpp OSHelper.cpp \
    ProfilingZone.cpp Profiler.cpp ThreadProfiler.cpp ScopeTimer.cpp Test.cpp \
    TestSuite.cpp ObjectCounter.cpp Point.cpp Directory.cpp DirEntry.cpp \
    StringHelper.cpp MathHelper.cpp GeomHelper.cpp CubicSpline.cpp BicubicSpline.cpp \
    $(ALL_H)
libbase_a_CXXFLAGS = -Wno-format-y2k

noinst_PROGRAMS=testbase
testbase_SOURCES=testbase.cpp $(ALL_H)
testbase_LDADD = ./libbase.la
testbase_LDFLAGS = -l@BOOST_THREAD_LIB@ @PTHREAD_LIBS@
