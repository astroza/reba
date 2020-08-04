#pragma once
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <pthread.h>

using boost::chrono::thread_clock;
namespace platform {
thread_clock::time_point threadCPUTime(boost::thread::native_handle_type handle);
}
