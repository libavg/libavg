ALL_H = cal_main.h matrix.h
ALL_C = cal_eval.c cal_main.c cal_tran.c cal_util.c ecalmain.c matrix.c

TESTS = testtsai

noinst_LTLIBRARIES = libtsai.la
libtsai_la_SOURCES = $(ALL_C) $(ALL_H)
noinst_PROGRAMS = testtsai

testtsai_SOURCES = testtsai.cpp $(ALL_H)
testtsai_LDADD = ./libtsai.la ../../lmfit/liblmfit.la ../../base/libbase.la

