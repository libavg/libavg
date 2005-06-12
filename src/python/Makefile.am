BUILT_SOURCES = $(srcdir)/Logger_wrap.cpp $(srcdir)/ConradRelais_wrap.cpp
SWIG_SOURCES = Logger.i ConradRelais.i

pkgpython_PYTHON = avg.py
pkgpyexec_LTLIBRARIES = _avg.la
_avg_la_SOURCES = Logger_wrap.cpp Conradrelais_wrap.cpp $(SWIG_SOURCES)
_avg_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS)
_avg_la_LDFLAGS = -module
_avg_la_LIBADD = ../base/libbase.la ../conradrelais/libconradrelais.la

Logger_wrap.cpp : $(SWIG_SOURCES)
	 $(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<
 
ConradRelais_wrap.cpp : $(SWIG_SOURCES)
	 $(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<
 
