# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header$

inherit eutils

DESCRIPTION="libavg is an engine for media installations"
HOMEPAGE="www.libavg.de"
SRC_URI="http://www.libavg.de/${P}.tar.gz"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~x86"
IUSE="ieee1394 directfb"

DEPEND="media-gfx/imagemagick
	>=media-video/ffmpeg-0.4.9_pre1-r1
	media-libs/libsdl
	dev-libs/libxml2
	x11-libs/pango
	directfb?(=dev-libs/DirectFB-0.9.22)
	ieee1394?(<=media-libs/libdc1394-1.0.0)
	>=dev-lang/python-2.3.4-r1
	dev-libs/boost"
RDEPEND=""

src_compile() {
	econf || die "econf failed"
	emake || die "emake failed"
}

src_install () {
	einstall || die "install failed"
	dodir /etc/env.d
	echo "PYTHONPATH=/usr/lib/python2.3/site-packages/libavg/:/usr/lib/python2.4/site-packages/libavg/" \
		> ${D}/etc/env.d/99libavg
}

