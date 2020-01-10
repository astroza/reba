#ifndef __WORKER_H__
#define __WORKER_H__

#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <v8.h>
#include <libplatform/libplatform.h>

class WorkerGroup;

typedef enum {
    TIMER_CALLBACK = 0,
    MAX
} WorkerAPIPrivateKeyIndex;

class Worker : boost::thread {
public:
    Worker(WorkerGroup *wg);
    void run();
    v8::MaybeLocal<v8::Private> get_api_private_key(WorkerAPIPrivateKeyIndex idx);
    boost::asio::io_context io_context;
private:
    WorkerGroup *worker_group;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> keep_running;
    v8::MaybeLocal<v8::Private> api_private_keys[WorkerAPIPrivateKeyIndex::MAX];
    void init_api_private_keys(v8::Isolate *isolate);
};

#endif