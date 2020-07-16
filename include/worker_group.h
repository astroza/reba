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
    Worker *createWorker();
    Worker *selectOrCreateWorker();
    size_t size();
    std::string script_source;
    bool privileged;
private:
    void registerWorker(Worker *new_worker);
    std::list<Worker *> workers_;
    std::list<Worker *>::iterator it;
};
} // namespace reba
#endif