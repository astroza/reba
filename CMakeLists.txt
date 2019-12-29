cmake_minimum_required(VERSION 3.15)
project(Lake CXX)
add_compile_options(-std=c++11 -O3 -fPIC)
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})

file(GLOB core_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file(GLOB webapi_source ${CMAKE_CURRENT_SOURCE_DIR}/src/webapi/*.cc)
file(GLOB lakeapi_source ${CMAKE_CURRENT_SOURCE_DIR}/src/lakeapi/*.cc)

set(SOURCE_FILES ${core_source} ${webapi_source} ${lakeapi_source})

add_custom_target(llhttp npm install
		  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp
		  COMMAND make CFLAGS=-fPIC)

add_custom_target(libv8_build
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools
                  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/build_v8.sh)

option(ENABLE_LIB_ONLY ON)
option(ENABLE_EXAMPLES OFF)

set(Boost_USE_STATIC_LIBS        OFF)
# https://github.com/boostorg/boost_install/issues/13
set(Boost_NO_BOOST_CMAKE         ON)
set(Boost_USE_DEBUG_LIBS         ON)
add_executable(lake ${SOURCE_FILES})
add_dependencies(lake llhttp)
add_dependencies(lake libv8_build)
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
	target_link_options(lake PRIVATE -fuse-ld=lld -pie -Wl,--fatal-warnings -fPIC -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,-z,defs -Wl,--as-needed -m64 -Werror -rdynamic --sysroot=${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/usr/local/lib/x86_64-linux-gnu -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/lib/x86_64-linux-gnu -L${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/build/linux/debian_sid_amd64-sysroot/usr/lib/x86_64-linux-gnu -pie -Wl,--disable-new-dtags -Wl,-O2 -Wl,--gc-sections)
endif()
add_library(libv8 OBJECT IMPORTED)
list(APPEND libv8_objs
	obj/third_party/zlib/zlib_adler32_simd/adler32_simd.o
	obj/third_party/zlib/zlib_inflate_chunk_simd/inffast_chunk.o
	obj/third_party/zlib/zlib_inflate_chunk_simd/inflate.o
	obj/third_party/zlib/zlib_crc32_simd/crc32_simd.o
	obj/third_party/zlib/zlib_x86_simd/crc_folding.o
	obj/third_party/zlib/zlib_x86_simd/fill_window_sse.o
	obj/libv8_libbase.a
	obj/libv8_libplatform.a
	obj/libv8_base_without_compiler.a
	obj/third_party/icu/libicui18n.a
	obj/third_party/icu/libicuuc.a
	obj/libtorque_generated_definitions.a
	obj/libv8_libsampler.a
	obj/src/inspector/libinspector.a
	obj/src/inspector/libinspector_string_conversions.a
	obj/third_party/inspector_protocol/libcrdtp.a
	obj/third_party/inspector_protocol/libcrdtp_platform.a
	obj/third_party/zlib/libchrome_zlib.a
	obj/libv8_compiler.a
	obj/libv8_snapshot.a
)
list(TRANSFORM libv8_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/)

set_target_properties(libv8 PROPERTIES IMPORTED_OBJECTS "${libv8_objs}")
target_link_libraries(lake $<TARGET_OBJECTS:libv8>)
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp/build)
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
