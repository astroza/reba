#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp> 
#include <worker.h>
#include <worker_group.h>

#include <platform.h>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::chrono::thread_clock;

namespace reba {
WorkerGroup::WorkerGroup(std::string script_source,
    bool privileged)
    : script_source(script_source)
    , privileged_(privileged)
    , keep_running_(boost::asio::make_work_guard(io_context_))
    , workers_length_(0)
{
    thread(&WorkerGroup::runPatrol, this);
    detach();
    if (privileged) {
        createWorker();
    }
}

void WorkerGroup::registerWorker(Worker* new_worker)
{
    assert(workers_length_ < workers_count_max);
    boost::unique_lock guard(workers_shared_mutex_);
    workers_[workers_length_++] = new_worker;
}

Worker* WorkerGroup::createWorker()
{
    auto new_worker = new Worker(this); 
    registerWorker(new_worker);
    return new_worker;
}

size_t WorkerGroup::size()
{
    return workers_length_;
}

Worker *WorkerGroup::selectWorker() 
{
    boost::shared_lock guard(workers_shared_mutex_);
    int idx = std::rand() % size();
    return workers_[idx];
}

Worker* WorkerGroup::selectOrCreateWorker()
{
    if (size() > 0) {
        return selectWorker();
    }
    return createWorker();
}

bool WorkerGroup::isPrivileged()
{
    return privileged_;
}

awaitable<void> WorkerGroup::checkWorkers()
{
    thread_clock::time_point lastCPUTime[workers_count_max];

    for (;;) {
        boost::asio::steady_timer timer(io_context_, std::chrono::steady_clock::now() + patrol_sampling_period);
        co_await timer.async_wait(use_awaitable);
        boost::chrono::duration<long double, boost::nano> usage(0.0);
        for(int i = 0; i < workers_length_; i++) {
            auto cpuTime = platform::threadCPUTime(workers_[i]->native_handle());
            usage += (cpuTime - lastCPUTime[i]) * 1.0;
            lastCPUTime[i] = cpuTime;
        }
        usage /= (double)workers_length_;
        if(usage.count() > patrol_scale_up_threshold.count()) {
            if(workers_length_ < std::thread::hardware_concurrency()) {
                createWorker();
            }
        }
    }
}

void WorkerGroup::runPatrol()
{
    co_spawn(
        io_context_, [this]() {
            return checkWorkers();
        },
        detached);
    io_context_.run();
}

} // namespace reba