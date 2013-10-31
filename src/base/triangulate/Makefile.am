AM_CPPFLAGS = -I.. @XML2_CFLAGS@ @PTHREAD_CFLAGS@
ALL_H = Triangulate.h Shapes.h Utils.h \
        AdvancingFront.h Sweep.h SweepContext.h

noinst_LTLIBRARIES = libtriangulate.la
libtriangulate_la_SOURCES = Triangulate.cpp Shapes.cpp \
        AdvancingFront.cpp Sweep.cpp SweepContext.cpp \
        $(ALL_H)
