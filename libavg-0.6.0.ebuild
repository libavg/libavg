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
	ieee1394? ( =media-libs/libdc1394-1* )
	dev-libs/boost"

pkg_setup() {
	if ! built_with_use media-libs/libsdl opengl X; then
		einfo "Please re-emerge media-libs/libsdl with the opengl and X USE flags set."
		die "libsdl needs opengl and X USE flags set."
	fi
}

src_compile() {
	econf `use_enable ieee1394 dc1394` || die "econf failed"
	emake || die "emake failed"
}

src_install () {
	python_version
	einstall || die "install failed"
	dodir /etc/env.d
	echo "PYTHONPATH=/usr/lib/python${PYVER}/site-packages/libavg/" \
		> ${D}/etc/env.d/99libavg
}
