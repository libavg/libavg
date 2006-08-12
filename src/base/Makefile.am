
INCLUDES = -I.. @XML2_CFLAGS@
ALL_H = FileHelper.h CmdLine.h Exception.h Logger.h ConfigMgr.h \
        XMLHelper.h TimeSource.h ProfilingZone.h Profiler.h \
        ScopeTimer.h IFrameListener.h Test.h TestSuite.h CountedPointer.h
		
noinst_LTLIBRARIES = libbase.la
libbase_la_SOURCES = FileHelper.cpp CmdLine.cpp Exception.cpp Logger.cpp \
    ConfigMgr.cpp XMLHelper.cpp TimeSource.cpp \
    ProfilingZone.cpp Profiler.cpp ScopeTimer.cpp Test.cpp TestSuite.cpp \
	$(ALL_H)
libbase_a_CXXFLAGS = -Wno-format-y2k
