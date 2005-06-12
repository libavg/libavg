SUBDIRS = src man

EXTRA_DIST = avg.dtd

install-data-hook:
	$(mkinstalldirs) $(datadir)/avg/
	$(INSTALL) -m 644 $(srcdir)/avg.dtd $(datadir)/avg/

bin_SCRIPTS = avg-config
