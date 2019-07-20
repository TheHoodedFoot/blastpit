# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/dev-python/yolk/yolk-0.4.1-r1.ebuild,v 1.1 2011/07/20 17:53:54 neurogeek Exp $

EAPI="7"
RESTRICT_PYTHON_ABIS="3.*"

inherit eutils git-r3

DESCRIPTION="Library and applications for the blastpit laser controller"
HOMEPAGE="http://git.47or.com/thf/Blastpit"
# SRC_URI="https://git.47or.com/thf/Blastpit/archive/feat/mqtt.tar.gz -> ${P}.tar.gz"
#SRC_URI="https://git.47or.com/thf/Blastpit/archive/master.tar.gz -> ${P}.tar.gz"

EGIT_REPO_URI="https://git.47or.com/thf/Blastpit.git"
EGIT_BRANCH="feature/mqtt"
#EGIT_COMMIT="${PV}"

LICENSE="BSD"
SLOT="0"
KEYWORDS="~amd64 ~x86"

IUSE="qt test devel"

DEPEND="dev-python/setuptools
		devel? ( dev-util/tup )
		dev-lang/swig
		net-libs/paho-mqtt-c
		test? ( dev-util/unity )
		qt? ( dev-qt/qtcore:5 )"

src_prepare() {
	S="${WORKDIR}/${PF}/src/blastpy"
}

src_install() {
	distutils_src_install
}

