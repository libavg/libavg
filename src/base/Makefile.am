
INCLUDES = -I. \
           `xml2-config --cflags` 

ALL_H = FileHelper.h CmdLine.h Exception.h Logger.h ConfigMgr.h Test.h \
        XMLHelper.h TimeSource.h ProfilingZone.h Profiler.h \
        ScopeTimer.h IFrameListener.h
		
lib_LTLIBRARIES = libbase.la
libbase_la_SOURCES = FileHelper.cpp CmdLine.cpp Exception.cpp Logger.cpp \
    ConfigMgr.cpp Test.cpp XMLHelper.cpp TimeSource.cpp \
    ProfilingZone.cpp Profiler.cpp ScopeTimer.cpp
libbase_ladir=$(includedir)/avg/base
libbase_la_HEADERS = $(ALL_H)
libbase_la_LDFLAGS = `xml2-config --libs`
