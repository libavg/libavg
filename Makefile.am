SUBDIRS = src 

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = bootstrap COPYING NEWS CreateVersionFile.sh BuildMacDeps.sh epydoc_libavg.css installfonts.sh makedocs.sh MakeMacDist.sh valgrind.cmdline valgrind.suppressions 

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)/$(datadir)/avg/
	$(shell $(srcdir)/CreateVersionFile.sh $(pkgpythondir))
