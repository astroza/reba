cmake_minimum_required(VERSION 3.15)
project(Lake CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdlib++ -nostdinc++ -stdlib=libc++ -O3")
set(SOURCE_FILES server.cc session.cc http.cc worker.cc worker_group.cc v8_global.cc)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/buildtools/third_party/libc++abi/trunk/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/buildtools/third_party/libc++/trunk/include)
target_link_libraries(lake pthread)
target_link_libraries(lake ${Boost_LIBRARIES})
target_link_libraries(lake ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp/build/libllhttp.a)

file(GLOB libv8_libs ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release/obj/*.a)
file(GLOB libv8_third_party_libs ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release/obj/third_party/icu/*.a ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release/obj/buildtools/third_party/*/*/*.o)

target_link_libraries(lake ${libv8_libs} ${libv8_third_party_libs})
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/llhttp/build)
target_include_directories(lake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
