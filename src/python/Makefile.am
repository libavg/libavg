INCLUDES = `xml2-config --cflags`

BUILT_SOURCES = $(srcdir)/avg_wrap.cpp
#$(srcdir)/ConradRelais_wrap.cpp \
#		$(srcdir)/Player_wrap.cpp
SWIG_SOURCES = avg.i Player.i Logger.i ConradRelais.i Node.i Image.i \
		RasterNode.i Container.i DivNode.i AVGNode.i Excl.i VideoBase.i \
		Video.i

pkgpython_PYTHON = avg.py
pkgpyexec_LTLIBRARIES = _avg.la
_avg_la_SOURCES = avg_wrap.cpp $(SWIG_SOURCES)
_avg_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS)
_avg_la_LDFLAGS = -module
_avg_la_LIBADD = ../base/libbase.la ../conradrelais/libconradrelais.la \
		../player/libplayer.la

avg_wrap.cpp : $(SWIG_SOURCES)
	 $(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<
 
