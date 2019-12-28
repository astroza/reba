#!/bin/bash

if [ "x$ARCH" == "x" ]; then
	ARCH=x64
fi

if [ ! -d "depot_tools" ]; then
	git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi
export PATH=$PATH:$(pwd)/depot_tools

cd ../deps
gclient sync
cd v8
tools/dev/gm.py $ARCH.release
cat << __EOF__ > out/$ARCH.release/args.gn
is_debug = false
target_cpu = "$ARCH"
v8_static_library = true
is_component_build = false
is_clang = true
use_custom_libcxx_for_host=false
use_custom_libcxx=false
__EOF__
autoninja -C out/$ARCH.release d8
