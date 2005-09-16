SUBDIRS = src man

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = avg.dtd doc bootstrap

dist-hook:
	rm -rf `find $(distdir)/doc -name CVS`
	rm -rf $(distdir)/doc/21c3lecture

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)/$(datadir)/avg/
	$(INSTALL) -m 644 $(srcdir)/avg.dtd $(DESTDIR)$(datadir)/avg/
