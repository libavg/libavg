AM_CPPFLAGS = -I.. @XML2_CFLAGS@ @PYTHON_CPPFLAGS@

ALL_H = Anim.h SimpleAnim.h LinearAnim.h AttrAnim.h ContinuousAnim.h EaseInOutAnim.h \
        WaitAnim.h ParallelAnim.h StateAnim.h
ALL_CPP = Anim.cpp SimpleAnim.cpp LinearAnim.cpp AttrAnim.cpp ContinuousAnim.cpp \
        EaseInOutAnim.cpp WaitAnim.cpp ParallelAnim.cpp StateAnim.cpp

noinst_LTLIBRARIES = libanim.la
libanim_la_SOURCES = $(ALL_CPP) $(ALL_H)
