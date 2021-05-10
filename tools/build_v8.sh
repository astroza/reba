#!/bin/bash

if [ "x$ARCH" == "x" ]; then
	ARCH=x64
fi

if [ ! -d "depot_tools" ]; then
	git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi
export PATH=$PATH:$(pwd)/depot_tools

cd ../deps
gclient sync -r d23dbf3b61912232a54dfcdcdcd87dbbe6708aea
cd v8
gn gen out/$ARCH.release --args='is_debug=false target_cpu="'$ARCH'" v8_static_library=true is_component_build=false is_clang=true use_custom_libcxx_for_host=false use_custom_libcxx=false'
ninja -C out/$ARCH.release d8
