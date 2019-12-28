cmake_minimum_required(VERSION 3.15)
project(Lake CXX)
add_compile_options(-std=c++11 -O3 -fPIC)
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
# https://cmake.org/pipermail/cmake/2010-March/035611.html
# Sort external objects list is a mess so I tell to linker iterate over objects to resolve all the symbols
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group")

file(GLOB core_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file(GLOB webapi_source ${CMAKE_CURRENT_SOURCE_DIR}/src/webapi/*.cc)
file(GLOB lakeapi_source ${CMAKE_CURRENT_SOURCE_DIR}/src/lakeapi/*.cc)

set(SOURCE_FILES ${core_source} ${webapi_source} ${lakeapi_source})

add_custom_target(llhttp npm install
		  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp
		  COMMAND make CFLAGS=-fPIC)

option(ENABLE_LIB_ONLY ON)
option(ENABLE_EXAMPLES OFF)
# add_subdirectory(deps/nghttp2)

set(Boost_USE_STATIC_LIBS        OFF)
# https://github.com/boostorg/boost_install/issues/13
set(Boost_NO_BOOST_CMAKE         ON)
set(Boost_USE_DEBUG_LIBS         ON)
add_executable(lake ${SOURCE_FILES})
add_dependencies(lake llhttp)
find_package(Boost 1.70.0 COMPONENTS system coroutine thread)
if(!Boost_FOUND)
  message(SEND_ERROR "Boost needed to build Lake")
endif()
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/include)
target_link_libraries(lake ${Boost_LIBRARIES})
target_link_libraries(lake ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp/build/libllhttp.a)
if(APPLE)
elseif(UNIX)
	target_link_options(lake PRIVATE -pie -Wl,--fatal-warnings -fPIC -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,-z,defs -Wl,--as-needed  -m64 -Werror -rdynamic --sysroot=${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/usr/local/lib/x86_64-linux-gnu -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/lib/x86_64-linux-gnu -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/usr/lib/x86_64-linux-gnu -pie -Wl,--disable-new-dtags -Wl,-O2 -Wl,--gc-sections)
endif()
add_library(libv8 OBJECT IMPORTED)
file(GLOB libv8_objs ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/obj/third_party/zlib/*/*.o ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/obj/libv8*.a ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/obj/third_party/icu/*.a ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/obj/src/inspector/*.a ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/obj/libtorque*.a)
set_target_properties(libv8 PROPERTIES IMPORTED_OBJECTS "${libv8_objs}")
target_link_libraries(lake $<TARGET_OBJECTS:libv8>)
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp/build)
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
