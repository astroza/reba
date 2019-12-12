#ifndef __WORKER_H__
#define __WORKER_H__

#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

class WorkerGroup;

class Worker : boost::thread {
public:
    Worker(WorkerGroup &wg);
    void main_loop();
private:
    WorkerGroup &worker_group;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> keep_running;
};

#endif