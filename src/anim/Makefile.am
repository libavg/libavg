INCLUDES = -I.. @XML2_CFLAGS@ @PYTHON_INCLUDES@

ALL_H = Anim.h SimpleAnim.h LinearAnim.h EaseInOutAnim.h WaitAnim.h \
        GroupAnim.h ParallelAnim.h StateAnim.h

ALL_CPP = Anim.cpp SimpleAnim.cpp LinearAnim.cpp EaseInOutAnim.cpp WaitAnim.cpp \
        GroupAnim.cpp ParallelAnim.cpp StateAnim.cpp 

noinst_LTLIBRARIES = libanim.la
libanim_la_SOURCES = $(ALL_CPP) $(ALL_H)
