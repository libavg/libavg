SUBDIRS = src man

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = avg.dtd

install-data-hook:
	$(mkinstalldirs) $(datadir)/avg/
	$(INSTALL) -m 644 $(srcdir)/avg.dtd $(datadir)/avg/

bin_SCRIPTS = avg-config
