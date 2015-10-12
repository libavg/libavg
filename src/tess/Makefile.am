AM_CPPFLAGS =
ALL_H = dict.h geom.h mesh.h priorityq.h priorityq-sort.h sweep.h tessmono.h \
dict-list.h memalloc.h normal.h render.h tess.h glutess.h

noinst_LTLIBRARIES = libtess.la
libtess_la_SOURCES = dict.c memalloc.c normal.c sweep.c tessmono.c \
        geom.c mesh.c priorityq.c render.c tess.c \
        $(ALL_H)
libtess_a_CXXFLAGS = -Wno-format-y2k
