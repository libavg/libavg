# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit eutils python

DESCRIPTION="libavg is an engine for media installations"
HOMEPAGE="www.libavg.de"
SRC_URI="http://www.libavg.de/${P}.tar.gz"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~x86"
IUSE="ieee1394 directfb"
RESTRICT="test"

DEPEND="media-gfx/imagemagick
	media-video/ffmpeg
	media-libs/libsdl
	dev-libs/libxml2
	x11-libs/pango
	directfb? ( >=dev-libs/DirectFB-0.9.22 )
	ieee1394? ( <=media-libs/libdc1394-1.9.9 )
	dev-libs/boost"
RDEPEND=""

pkg_setup() {
	if ! built_with_use media-libs/libsdl opengl; then
		einfo "Please re-emerge media-libs/libsdl with the opengl USE flag set."
		die "libsdl needs opengl USE flag set."
	fi
	if ! built_with_use media-libs/libsdl X; then
		einfo "Please re-emerge media-libs/libsdl with the X USE flag set."
		die "libsdl needs X USE flag set."
	fi
}

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install () {
	python_version
	einstall || die "install failed"
	dodir /etc/env.d
	echo "PYTHONPATH=/usr/lib/python${PYVER}/site-packages/libavg/" \
		> ${D}/etc/env.d/99libavg
}

pkg_postinst(){
	ewarn "To run libavg, the environment variable PYTHONPATH must be set"
	ewarn "correctly. The installer does this, but it can't change the"
	ewarn "environment in shells that are already running. In these shells,"
	ewarn "you need to execute 'source /etc/profile' before you run libavg."
}
