SUBDIRS = src 

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = bootstrap COPYING NEWS CreateVersionFile.sh

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)/$(datadir)/avg/
	./CreateVersionFile.sh $(pkgpythondir)
