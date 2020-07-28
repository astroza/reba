#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp> 
#include <worker.h>
#include <worker_group.h>

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;

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
    Worker* ret;
    int count = this->privileged_ ? 1 : 8;
    for (int i = 0; i < count; i++) {
        auto new_worker = new Worker(this);
        registerWorker(new_worker);
        ret = new_worker;
    }
    return ret;
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
    for (;;) {
        puts("Patrol");
        boost::asio::steady_timer timer(io_context_, std::chrono::steady_clock::now() + std::chrono::seconds(4));
        co_await timer.async_wait(use_awaitable);
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