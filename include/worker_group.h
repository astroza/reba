#ifndef __WORKER_GROUP_H__
#define __WORKER_GROUP_H__

#include <list>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <engine.h>

namespace reba
{
class Worker;

class WorkerGroup
{
public:
    WorkerGroup(std::string script_source, bool privileged = false);
    Worker *scale_up();
    void delegate_request();
    size_t size();
    std::string script_source;
    bool privileged;
private:
    void add_worker(Worker *worker);
    std::list<Worker *> workers_;
};
} // namespace reba
#endif