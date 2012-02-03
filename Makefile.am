SUBDIRS = src man 

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = bootstrap COPYING NEWS CreateVersionFile.py BuildMacDeps.sh installfonts.sh makedocs.sh MakeMacDist.sh valgrind.cmdline valgrind.suppressions 

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)/$(datadir)/avg/
