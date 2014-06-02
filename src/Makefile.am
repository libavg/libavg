SUBDIRS = base graphics imaging lmfit oscpack audio video player \
        anim wrapper python test utils samples

EXTRA_DIST = avgrc avgconfigwrapper.h api.h avgconfig_win.h version.h \
        $(wildcard glm/*.hpp) $(wildcard glm/core/*.hpp) $(wildcard glm/core/*.inl) \
        $(wildcard glm/gtc/*.hpp) $(wildcard glm/gtc/*.inl) \
        $(wildcard glm/gtx/*.hpp) $(wildcard glm/gtx/*.inl) \
        $(wildcard glm/virtrev/*.hpp) $(wildcard glm/detail/*.inl) \
        $(wildcard glm/detail/*.hpp)

BUILT_SOURCES = version.h
DISTCLEANFILES = version.h

version.h: FORCE
	$(top_srcdir)/CreateVersionFile.py $(top_builddir)/src

FORCE:
