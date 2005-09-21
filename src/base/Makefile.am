
INCLUDES = -I. \
           `xml2-config --cflags` 

ALL_H = FileHelper.h CmdLine.h Exception.h Logger.h ConfigMgr.h \
        XMLHelper.h TimeSource.h ProfilingZone.h Profiler.h \
        ScopeTimer.h IFrameListener.h Test.h TestSuite.h
		
noinst_LTLIBRARIES = libbase.la
libbase_la_SOURCES = FileHelper.cpp CmdLine.cpp Exception.cpp Logger.cpp \
    ConfigMgr.cpp XMLHelper.cpp TimeSource.cpp \
    ProfilingZone.cpp Profiler.cpp ScopeTimer.cpp Test.cpp TestSuite.cpp \
	$(ALL_H)
libbase_ladir=$(includedir)/avg/base
libbase_la_LDFLAGS = `xml2-config --libs`
