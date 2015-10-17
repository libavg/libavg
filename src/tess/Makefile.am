AM_CPPFLAGS =
ALL_H = bucketalloc.h  dict.h  geom.h  mesh.h  priorityq.h  sweep.h  tesselator.h  tess.h

noinst_LTLIBRARIES = libtess.la
libtess_la_SOURCES = bucketalloc.c  dict.c  geom.c  mesh.c  priorityq.c  sweep.c  tess.c \
        $(ALL_H)
libtess_a_CXXFLAGS = -Wno-format-y2k
