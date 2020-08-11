#include <engine.h>
#include <iostream>
#include <rebaapi.h>
#include <webapi.h>
#include <worker.h>
#include <worker_group.h>
#include <platform.h>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>

namespace reba {
// it must be called after root HandleScope creation
void Worker::initAPIPrivateKeys()
{
    api_private_keys_[WorkerAPIPrivateKeyIndex::Value::TimerCallback] = v8::Private::ForApi(isolate_,
        v8::String::NewFromUtf8(isolate_, "timer::callback").ToLocalChecked());
}

v8::MaybeLocal<v8::Private> Worker::getAPIPrivateKey(WorkerAPIPrivateKeyIndex::Value idx)
{
    return api_private_keys_[idx];
}

void Worker::setCallback(WorkerCallbackIndex::Value idx, v8::Local<v8::Function>& func)
{
    registered_callbacks_[idx].Reset(isolate_, func);
}

v8::MaybeLocal<v8::Function> Worker::getCallback(WorkerCallbackIndex::Value idx)
{
    v8::Persistent<v8::Function>& persistent_handle = registered_callbacks_[idx];
    if (persistent_handle.IsEmpty()) {
        return v8::MaybeLocal<v8::Function>();
    }
    return registered_callbacks_[idx].Get(isolate_);
}

v8::Local<v8::Context> Worker::createContext()
{
    if (isolate_->GetData(IsolateDataIndex::Value::FirstWorkerContextCreated)) {
        // An worker context was already created and a new Context will be based on it.
        return v8::Context::New(isolate_);
    }
    v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate_);
    webapi::initGlobal(isolate_, global_template);
    if (worker_group_->isPrivileged()) {
        rebaapi::initGlobal(isolate_, global_template);
    }
    webapi::Console* console = static_cast<webapi::Console*>(isolate_->GetData(IsolateDataIndex::Value::Console));
    if (!console) {
        console = new webapi::Console(isolate_);
        isolate_->SetData(IsolateDataIndex::Value::Console, console);
        v8::debug::SetConsoleDelegate(isolate_, console);
    }
    isolate_->SetData(IsolateDataIndex::Value::FirstWorkerContextCreated, isolate_);
    return v8::Context::New(isolate_, NULL, global_template);
}

Worker::Worker(WorkerGroup* worker_group)
    : worker_group_(worker_group)
    , execution_timer_(boost::asio::steady_timer(worker_group->io_context_, std::chrono::steady_clock::time_point::max()))
    , keep_running_(boost::asio::make_work_guard(io_context_))
    , execution_timedout_(false)
    , thread(&Worker::run, this)
{
}

void Worker::setExecutionTimeout(std::chrono::milliseconds timeout)
{
    execution_timer_.expires_from_now(timeout);
    execution_timedout_ = false;
    execution_timer_.async_wait([isolate = this->isolate_, &timedout = this->execution_timedout_](const boost::system::error_code &ec) {
        if(!ec.failed()) {
            isolate->TerminateExecution();
            isolate->GetCurrentContext()->SetAbortScriptExecution([](v8::Isolate* isolate, v8::Local<v8::Context> context) {
                puts("this context was interrupted");
            });
            timedout = true;
        }
    });
}

void Worker::clearExecutionTimeout() 
{
    boost::asio::post(worker_group_->io_context_, [&timer = this->execution_timer_]() {
        timer.cancel();
    });
    if(execution_timedout_) {
        isolate_->CancelTerminateExecution();
    }
}

bool Worker::isExecutionTimedout() 
{
    return execution_timedout_;
}

void Worker::initIsolate()
{
    isolate_ = v8::Isolate::New(reba::engine::g_create_params);
    isolate_->SetData(IsolateDataIndex::Value::Worker, this);
    /**
     * Execution timer: Stop JS if execution exceeds the limit
     * TODO: Decrease consumed time by call from session
     */
    isolate_->AddBeforeCallEnteredCallback([](v8::Isolate *isolate) {
        auto worker = static_cast<Worker *>(isolate->GetData(IsolateDataIndex::Value::Worker));
        worker->setExecutionTimeout(std::chrono::milliseconds(50));
    });
    isolate_->AddCallCompletedCallback([](v8::Isolate *isolate) {
        auto worker = static_cast<Worker *>(isolate->GetData(IsolateDataIndex::Value::Worker));
        worker->clearExecutionTimeout();
    });
}

void Worker::run()
{
    initIsolate();
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = createContext();
        v8::Context::Scope context_scope(context);

        initAPIPrivateKeys();

        v8::TryCatch try_catch(isolate_);
        v8::Local<v8::String> script_name = v8::String::NewFromUtf8(isolate_, "worker.js",
            v8::NewStringType::kNormal)
                                                .ToLocalChecked();
        v8::ScriptOrigin origin(script_name);

        v8::Local<v8::Script> script;
        v8::Local<v8::String> source;
        v8::String::NewFromUtf8(isolate_, worker_group_->script_source.c_str(), v8::NewStringType::kNormal, static_cast<int>(worker_group_->script_source.size())).ToLocal(&source);
        if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
            reba::engine::report_exception(isolate_, &try_catch);
            return;
        } else {
            v8::Local<v8::Value> result;
            if (!script->Run(context).ToLocal(&result)) {
                assert(try_catch.HasCaught());
                // Print errors that happened during execution.
                reba::engine::report_exception(isolate_, &try_catch);
                return;
            }
        }
        while (io_context_.run_one() > 0) {
            //isolate_->LowMemoryNotification();
            while (v8::platform::PumpMessageLoop(reba::engine::g_platform.get(), isolate_))
                continue;
        }
    }
    isolate_->Dispose();
}

void Worker::continueRequestProcessing(reba::http::Session& session)
{
    // ActiveSession active_session(&session, this);
    v8::HandleScope handle_scope(isolate_);
    auto context = createContext();
    v8::Context::Scope scope(context);

    auto maybe_callback = getCallback(WorkerCallbackIndex::Value::FetchEvent);
    if (!maybe_callback.IsEmpty()) {
        auto fetch_callback = v8::Local<v8::Function>::Cast(maybe_callback.ToLocalChecked());
        auto response = fetch_callback->Call(context, context->Global(), 0, nullptr);
        ::http::response<::http::string_body> res { ::http::status::ok, 11 };
        res.set(::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(::http::field::content_type, "text/html");
        res.keep_alive(false);
        if(!isExecutionTimedout()) {
            v8::String::Utf8Value str(isolate_, response.ToLocalChecked());
            res.body() = std::string(*str);
        } else {
            res.body() = std::string("Timedout");
        }
        res.prepare_payload();
        session.send(std::move(res));
    }
}
} // namespace reba
// isolate_->VisitHandlesWithClassIds( &phv );