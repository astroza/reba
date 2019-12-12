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
    WorkerGroup(std::shared_ptr<v8::Platform> platform, std::string script);
    boost::asio::io_context io_context;
    std::shared_ptr<V8Global> v8_global;
    v8::Local<v8::Script> script;
};

#endif