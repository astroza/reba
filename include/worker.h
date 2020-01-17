#ifndef __WORKER_H__
#define __WORKER_H__

#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <v8.h>
#include <libplatform/libplatform.h>

namespace lake
{
class WorkerGroup;

struct WorkerAPIPrivateKeyIndex 
{
typedef enum
{
    TimerCallback = 0,
    Max
} Value;
};

struct WorkerCallbackIndex 
{
typedef enum 
{
    FetchEvent = 0,
    Max
} Value;
};

struct IsolateDataIndex {
typedef enum
{
    Worker = 0,
    Console,
    Max
} Value;
};

class Worker : boost::thread
{
public:
    Worker(WorkerGroup *wg);
    void run();
    void process_request();
    void set_callback(WorkerCallbackIndex::Value idx, v8::Local<v8::Function> &func);
    v8::MaybeLocal<v8::Function> get_callback(WorkerCallbackIndex::Value idx);
    v8::MaybeLocal<v8::Private> get_api_private_key(WorkerAPIPrivateKeyIndex::Value idx);
    boost::asio::io_context io_context;

private:
    v8::Isolate *isolate_;
    v8::Persistent<v8::Function> registered_callbacks_[WorkerCallbackIndex::Max];
    v8::MaybeLocal<v8::Private> api_private_keys[WorkerAPIPrivateKeyIndex::Max];
    WorkerGroup *worker_group;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> keep_running;
    void init_api_private_keys(v8::Isolate *isolate);
};
}
#endif