#!/bin/bash

download() {
	[ -f "$1" ] || wget "$2" -O $1
}

set -e
BASE="$(dirname -- "$(readlink -f -- "$0")")"

Simple-WebSocket-Server() {
	local COMMIT=fb00b73d74f3dfc6e300e2d599a250a4a6aa8c30
	local BASEURL=https://raw.githubusercontent.com/eidheim/Simple-WebSocket-Server/$COMMIT
	mkdir -p ext/include/
	download ext/include/server_ws.hpp $BASEURL/server_ws.hpp
	download ext/include/crypto.hpp $BASEURL/crypto.hpp
}

Simple-WebSocket-Server
