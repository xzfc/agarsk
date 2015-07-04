#!/bin/bash

download() {
	[ -f "$1" ] || wget "$2" -O $1
}


set -e
BASE="$(dirname -- "$(readlink -f -- "$0")")"

libwebsockets() (
	local R=1.4-chrome43-firefox-36
	local NR=libwebsockets-${R}

	download $NR.tar.gz https://github.com/warmcat/libwebsockets/archive/v$R.tar.gz

	rm -rf $NR
	tar xf $NR.tar.gz

	rm -rf $NR-build
	mkdir -p $NR-build
	cd $NR-build

	cmake ../$NR -DCMAKE_INSTALL_PREFIX:PATH="$BASE/ext"
	make $MAKEOPTS
	make install
)

mkdir -p external
cd external
libwebsockets
