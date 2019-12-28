#ifndef __WORKER_GROUP_H__
#define __WORKER_GROUP_H__

#include <list>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <v8_global.h>

class Worker;

class WorkerGroup : boost::thread_group {
public:
    WorkerGroup(V8Global &v8_global, std::string script_source, bool privileged = false);
    boost::asio::io_context io_context;
    V8Global &v8_global;
    std::string script_source;
    bool privileged;
};

#endif