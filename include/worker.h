#ifndef __WORKER_H__
#define __WORKER_H__

#include <boost/thread/thread.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <v8.h>
#include <libplatform/libplatform.h>
#include <http.h>

namespace reba
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
    FirstWorkerContextCreated,
    Max
} Value;
};

class Worker;

/** 
 * Register a session in a worker. If worker has any problem, all registered
 * sessions will be handled properly.
 */ 
class ActiveSession
{
    ActiveSession(reba::http::Session *session, Worker *worker) {

    }

    ~ActiveSession() {

    }
};

class Worker : boost::thread
{
public:
    Worker(WorkerGroup *wg);
    void run();
    void continueRequestProcessing(reba::http::Session &session);
    void setCallback(WorkerCallbackIndex::Value idx, v8::Local<v8::Function> &func);
    v8::Local<v8::Context> createContext();
    v8::MaybeLocal<v8::Function> getCallback(WorkerCallbackIndex::Value idx);
    v8::MaybeLocal<v8::Private> getAPIPrivateKey(WorkerAPIPrivateKeyIndex::Value idx);
    boost::asio::io_context io_context_;
private:
    v8::Isolate *isolate_;
    v8::Persistent<v8::Function> registered_callbacks_[WorkerCallbackIndex::Max];
    v8::MaybeLocal<v8::Private> api_private_keys_[WorkerAPIPrivateKeyIndex::Max];
    WorkerGroup *worker_group_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> keep_running_;
    void initAPIPrivateKeys(v8::Isolate *isolate);
};
}
#endif