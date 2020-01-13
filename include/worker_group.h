#ifndef __WORKER_GROUP_H__
#define __WORKER_GROUP_H__

#include <list>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <engine.h>

namespace lake
{
class Worker;

class WorkerGroup : boost::thread_group
{
public:
    WorkerGroup(std::string script_source, bool privileged = false);
    std::string script_source;
    bool privileged;
};
} // namespace lake
#endif