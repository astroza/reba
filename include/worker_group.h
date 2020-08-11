#ifndef __WORKER_GROUP_H__
#define __WORKER_GROUP_H__

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <list>

#include <engine.h>

#ifndef WORKER_GROUP_MAX_SIZE
#define WORKER_GROUP_MAX_SIZE 64
#endif

#ifndef WORKER_GROUP_PATROL_PERIOD
#define WORKER_GROUP_PATROL_PERIOD 4
#endif

using boost::asio::awaitable;

namespace reba {

class Worker;
class WorkerGroup : boost::thread {
public:
    WorkerGroup(std::string script_source, bool privileged = false);
    Worker* createWorker();
    Worker* selectOrCreateWorker();
    bool isPrivileged();
    size_t size();
    void runPatrol();
    
    std::string script_source;
    boost::asio::io_context io_context_;

    static constexpr int workers_count_max = WORKER_GROUP_MAX_SIZE;
    static constexpr double threshold_ratio = 0.75;
    static constexpr auto patrol_sampling_period = std::chrono::seconds(WORKER_GROUP_PATROL_PERIOD);
    static constexpr auto patrol_scale_up_threshold = std::chrono::duration_cast<std::chrono::duration<long double, std::nano>>(patrol_sampling_period) * threshold_ratio;
private:
    awaitable<void> checkWorkers();
    void registerWorker(Worker* new_worker);
    Worker* selectWorker();

    bool privileged_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> keep_running_;
    /** 
     * workers_ type reasoning:
     *   What I know:
     *     1) workers_ modification frequency (add/remove) will be lesser than
     *     read frequency
     *     2) Random access to workers_ is expected
     *   If I choose std::array:
     *     a) workers_ modification complexity is O(n)
     *     b) workers_ random read is O(1)
     *   If I choose std::list
     *     a) workers_ modification complexity is O(n)
     *     b) workers_ random read is O(n)
     */
    std::array<Worker*, workers_count_max> workers_;
    int workers_length_;
    boost::shared_mutex workers_shared_mutex_;

    friend class Worker;
};
} // namespace reba
#endif