AM_CPPFLAGS = -I.
CPPFLAGS = -DBUG

ALL_H = lmmin.h lm_eval.h
noinst_LTLIBRARIES = liblmfit.la
liblmfit_la_SOURCES = lmmin.c lm_eval.c $(ALL_H)
liblmfit_a_CXXFLAGS = -Wno-format-y2k
