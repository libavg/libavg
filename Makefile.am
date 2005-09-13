SUBDIRS = src man

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = avg.dtd doc

dist-hook:
	rm -rf `find $(distdir)/doc -name CVS`
	rm -rf $(distdir)/doc/21c3lecture

install-data-hook:
	$(mkinstalldirs) $(datadir)/avg/
	$(INSTALL) -m 644 $(srcdir)/avg.dtd $(datadir)/avg/

bin_SCRIPTS = avg-config
