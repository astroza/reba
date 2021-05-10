#ifdef __APPLE__
#include <mach/thread_act.h>
#endif
#include <platform.h>

using boost::chrono::thread_clock;
typedef thread_clock::duration duration;

namespace platform {
thread_clock::time_point threadCPUTime(boost::thread::native_handle_type handle)
{
#ifdef __APPLE__
    // Borrow from include/boost/chrono/detail/inlined/mac/thread_clock.hpp
    mach_port_t port = pthread_mach_thread_np(handle);
    // get the thread info
    thread_basic_info_data_t info;
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    if (thread_info(port, THREAD_BASIC_INFO, (thread_info_t)&info, &count) != KERN_SUCCESS) {
        BOOST_ASSERT(0 && "Boost::Chrono - Internal Error");
        return thread_clock::time_point();
    }
    // convert to nanoseconds
    duration user = duration(
        static_cast<thread_clock::rep>(info.user_time.seconds) * 1000000000
        + static_cast<thread_clock::rep>(info.user_time.microseconds) * 1000);

    duration system = duration(
        static_cast<thread_clock::rep>(info.system_time.seconds) * 1000000000
        + static_cast<thread_clock::rep>(info.system_time.microseconds) * 1000);

    return thread_clock::time_point(user + system);
#elif __linux__
    return thread_clock::time_point(duration(0));
#endif
}

}
