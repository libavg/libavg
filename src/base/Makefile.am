SUBDIRS = triangulate

INCLUDES = -I.. @XML2_CFLAGS@ @PTHREAD_CFLAGS@
ALL_H = FileHelper.h CmdLine.h Exception.h Logger.h ConfigMgr.h ObjectCounter.h \
        XMLHelper.h TimeSource.h ProfilingZone.h ThreadProfiler.h \
        ScopeTimer.h IFrameEndListener.h IPreRenderListener.h IPlaybackEndListener.h \
        Test.h TestSuite.h OSHelper.h Queue.h WorkerThread.h Command.h ObjectCounter.h \
        Rect.h Directory.h DirEntry.h StringHelper.h MathHelper.h GeomHelper.h \
        CubicSpline.h BezierCurve.h UTF8String.h Triangle.h \
        WideLine.h DlfcnWrapper.h Signal.h Backtrace.h \
        CmdQueue.h ProfilingZoneID.h GLMHelper.h VersionInfo.h 

TESTS=testbase

EXTRA_DIST = DlfcnWrapper.cpp

noinst_LTLIBRARIES = libbase.la
libbase_la_SOURCES = FileHelper.cpp CmdLine.cpp Exception.cpp Logger.cpp \
    ConfigMgr.cpp XMLHelper.cpp TimeSource.cpp OSHelper.cpp \
    ProfilingZone.cpp ThreadProfiler.cpp ScopeTimer.cpp Test.cpp \
    TestSuite.cpp ObjectCounter.cpp Directory.cpp DirEntry.cpp \
    StringHelper.cpp MathHelper.cpp GeomHelper.cpp CubicSpline.cpp \
    BezierCurve.cpp UTF8String.cpp Triangle.cpp WideLine.cpp \
    Backtrace.cpp ProfilingZoneID.cpp GLMHelper.cpp VersionInfo.cpp \
    $(ALL_H)
libbase_a_CXXFLAGS = -Wno-format-y2k

noinst_PROGRAMS=testbase
testbase_SOURCES=testbase.cpp $(ALL_H)
testbase_LDADD = ./libbase.la ./triangulate/libtriangulate.la -l@BOOST_THREAD_LIB@ \
        @PTHREAD_LIBS@
# -rdynamic needed only for testBacktrace to work under linux.
testbase_LDFLAGS = -rdynamic
