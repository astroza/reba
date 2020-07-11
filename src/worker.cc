#include <iostream>
#include <engine.h>
#include <worker_group.h>
#include <worker.h>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

namespace reba
{
bool report_exceptions = true;
bool print_result = true;

// it must be called after root HandleScope creation
void Worker::init_api_private_keys(v8::Isolate *isolate)
{
    api_private_keys[WorkerAPIPrivateKeyIndex::Value::TimerCallback] = v8::Private::ForApi(isolate,
            v8::String::NewFromUtf8(isolate, "timer::callback").ToLocalChecked());
}

v8::MaybeLocal<v8::Private> Worker::get_api_private_key(WorkerAPIPrivateKeyIndex::Value idx)
{
    return api_private_keys[idx];
}

void Worker::set_callback(WorkerCallbackIndex::Value idx, v8::Local<v8::Function> &func)
{
    registered_callbacks_[idx].Reset(isolate_, func);
}

v8::MaybeLocal<v8::Function> Worker::get_callback(WorkerCallbackIndex::Value idx)
{
    v8::Persistent<v8::Function> &persistent_handle = registered_callbacks_[idx];
    if (persistent_handle.IsEmpty())
    {
        return v8::MaybeLocal<v8::Function>();
    }
    return registered_callbacks_[idx].Get(isolate_);
}

Worker::Worker(WorkerGroup *worker_group) : worker_group(worker_group), keep_running(boost::asio::make_work_guard(io_context))
{
    thread(&Worker::run, this);
    detach();
}

void Worker::run()
{
    isolate_ = v8::Isolate::New(reba::engine::g_create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = reba::engine::createContext(isolate_, worker_group->privileged);
        v8::Context::Scope context_scope(context);

        isolate_->SetData(IsolateDataIndex::Value::Worker, this);
        init_api_private_keys(isolate_);

        v8::TryCatch try_catch(isolate_);
        v8::Local<v8::String> script_name =
            v8::String::NewFromUtf8(isolate_, "worker.js",
                                    v8::NewStringType::kNormal)
                .ToLocalChecked();
        v8::ScriptOrigin origin(script_name);

        v8::Local<v8::Script> script;
        v8::Local<v8::String> source;
        v8::String::NewFromUtf8(isolate_, worker_group->script_source.c_str(), v8::NewStringType::kNormal, static_cast<int>(worker_group->script_source.size())).ToLocal(&source);
        if (!v8::Script::Compile(context, source, &origin).ToLocal(&script))
        {
            // Print errors that happened during compilation.
            if (report_exceptions)
                reba::engine::report_exception(isolate_, &try_catch);
            return;
        }
        else
        {
            v8::Local<v8::Value> result;
            if (!script->Run(context).ToLocal(&result))
            {
                assert(try_catch.HasCaught());
                // Print errors that happened during execution.
                if (report_exceptions)
                    reba::engine::report_exception(isolate_, &try_catch);
                return;
            }
            else
            {
                assert(!try_catch.HasCaught());
                if (print_result)
                {
                    // If all went well and the result wasn't undefined then print
                    // the returned value.
                    v8::String::Utf8Value str(isolate_, result);
                    const char *cstr = reba::engine::utf8_value_to_cstring(str);
                    printf("%s\n", cstr);
                }
            }
        }
        reba::engine::report_exception(isolate_, &try_catch);
        while (io_context.run_one() > 0)
        {
            // isolate_->LowMemoryNotification();
            while (v8::platform::PumpMessageLoop(reba::engine::g_platform.get(), isolate_))
                continue;
        }
    }
}

void Worker::continueRequestProcessing(reba::http::Session &session)
{
    v8::HandleScope handle_scope(isolate_);
    auto context = isolate_->GetCurrentContext();
    auto maybe_callback = get_callback(WorkerCallbackIndex::Value::FetchEvent);

    if (!maybe_callback.IsEmpty())
    {
        auto fetch_callback = v8::Local<v8::Function>::Cast(maybe_callback.ToLocalChecked());
        auto response = fetch_callback->Call(context, context->Global(), 0, nullptr);
        
        v8::String::Utf8Value str(isolate_, response.ToLocalChecked());

        ::http::response<::http::string_body> res{::http::status::ok, 11};
        
        res.set(::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(::http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = std::string(*str);
        res.prepare_payload();
        session.send(std::move(res));
    }
}
} // namespace reba
  // isolate_->VisitHandlesWithClassIds( &phv );