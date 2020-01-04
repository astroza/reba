#ifndef __WORKER_GROUP_H__
#define __WORKER_GROUP_H__

#include <list>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <lake.h>

class Worker;

class WorkerGroup : boost::thread_group {
public:
    WorkerGroup(std::string script_source, bool privileged = false);
    boost::asio::io_context io_context;
    std::string script_source;
    bool privileged;
};

#endif