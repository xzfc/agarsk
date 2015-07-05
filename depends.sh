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

Simple-WebSocket-Server() {
	local COMMIT=fb00b73d74f3dfc6e300e2d599a250a4a6aa8c30
	local BASEURL=https://raw.githubusercontent.com/eidheim/Simple-WebSocket-Server/$COMMIT
	mkdir -p ext/include/
	download ext/include/server_ws.hpp $BASEURL/server_ws.hpp
	download ext/include/crypto.hpp $BASEURL/crypto.hpp
}

#mkdir -p external
#cd external
#libwebsockets

Simple-WebSocket-Server
