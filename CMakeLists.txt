cmake_minimum_required(VERSION 3.15)
if(APPLE)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
elseif(UNIX)
	set(CMAKE_CXX_COMPILER g++)
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
endif()
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

project(Reba CXX)

add_compile_options(-std=c++2a -fPIC -DV8_COMPRESS_POINTERS -O3)

file(GLOB core_cc_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
file(GLOB webapi_cc_source ${CMAKE_CURRENT_SOURCE_DIR}/src/webapi/*.cc)
file(GLOB webapi_js_source ${CMAKE_CURRENT_SOURCE_DIR}/src/webapi/js/*.js)
file(GLOB rebaapi_cc_source ${CMAKE_CURRENT_SOURCE_DIR}/src/rebaapi/*.cc)

set(SOURCE_FILES ${core_cc_source} ${webapi_cc_source} ${rebaapi_cc_source} ${CMAKE_CURRENT_SOURCE_DIR}/src/snapshot_generated.cc)

add_custom_target(libv8_build
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools
                  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/build_v8.sh ${webapi_js_source})

option(ENABLE_LIB_ONLY ON)
option(ENABLE_EXAMPLES OFF)

set(Boost_USE_STATIC_LIBS        OFF)
# https://github.com/boostorg/boost_install/issues/13
set(Boost_NO_BOOST_CMAKE         ON)
set(Boost_USE_DEBUG_LIBS         ON)
add_executable(reba ${SOURCE_FILES})
add_dependencies(reba libv8_build)
find_package(Boost 1.76.0 COMPONENTS system coroutine thread)
if(!Boost_FOUND)
  message(SEND_ERROR "Boost needed to build reba")
endif()
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/gen)
add_library(libv8 OBJECT IMPORTED)
list(APPEND libv8_objs
obj/third_party/zlib/zlib_adler32_simd/adler32_simd.o
obj/third_party/zlib/zlib_inflate_chunk_simd/inffast_chunk.o
obj/third_party/zlib/zlib_inflate_chunk_simd/inflate.o
obj/third_party/zlib/zlib_crc32_simd/crc32_simd.o
obj/third_party/zlib/zlib_x86_simd/crc_folding.o
obj/third_party/zlib/zlib_x86_simd/fill_window_sse.o
obj/third_party/zlib/google/libcompression_utils_portable.a
obj/third_party/zlib/libchrome_zlib.a
obj/third_party/inspector_protocol/libcrdtp_platform.a
obj/third_party/inspector_protocol/libcrdtp.a
obj/third_party/icu/libicui18n.a
obj/third_party/icu/libicuuc.a
obj/libv8_snapshot.a
obj/libv8_compiler.a
obj/src/inspector/libinspector_string_conversions.a
obj/src/inspector/libinspector.a
obj/libv8_bigint.a
obj/libtorque_generated_definitions.a
obj/libv8_cppgc_shared.a
obj/libcppgc_base.a
obj/libv8_base_without_compiler.a
obj/libv8_libplatform.a
obj/libv8_libbase.a
)

list(TRANSFORM libv8_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out/x64.release/)
set_target_properties(libv8 PROPERTIES IMPORTED_OBJECTS "${libv8_objs}")
# https://cmake.org/pipermail/cmake-developers/2015-January/024072.html
target_link_libraries(reba -Wl,--start-group $<TARGET_OBJECTS:libv8> -Wl,--end-group ${Boost_LIBRARIES} dl)
target_include_directories(reba PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
