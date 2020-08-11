#include <boost/asio.hpp>
#include <webapi.h>
#include <worker.h>

#include <iostream>

#define MAX_SET_TIMEOUT_ARGC 16

namespace webapi
{
namespace timer
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    int64_t time_ms = 0;
    auto context = isolate->GetCurrentContext();

    if (!args.IsConstructCall()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Function is a constructor", v8::NewStringType::kNormal).ToLocalChecked());
        return;
    }

    if (args.Length() == 0 || !args[0]->IsFunction()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Must specify a callback", v8::NewStringType::kNormal).ToLocalChecked());
        return;
    }

    v8::HandleScope handle_scope(isolate);
    if(args.Length() > 1) {
        auto time_ms_integer = args[1]->ToInteger(context).ToLocalChecked();
        time_ms = time_ms_integer->Value();
    }
    auto worker = static_cast<reba::Worker *>(isolate->GetData(reba::IsolateDataIndex::Value::Worker));
    auto timer = new boost::asio::deadline_timer(worker->io_context_);
    timer->expires_from_now(boost::posix_time::milliseconds(time_ms));
    args.This()->SetPrivate(context, worker->getAPIPrivateKey(reba::WorkerAPIPrivateKeyIndex::Value::TimerCallback).ToLocalChecked(), args[0]);
    auto timer_bind = reba::engine::bind(isolate, args.This(), timer);
    timer_bind->ref();
    timer->async_wait([isolate, worker, timer_bind](const boost::system::error_code& ec)
    {
        v8::HandleScope handle_scope(isolate);
        v8::TryCatch try_catch(isolate);
        
        auto timer_obj = timer_bind->getObjectHandle(isolate);
        auto callback = timer_obj->GetPrivate(isolate->GetCurrentContext(), worker->getAPIPrivateKey(reba::WorkerAPIPrivateKeyIndex::Value::TimerCallback).ToLocalChecked()).ToLocalChecked();
        auto callback_as_function = v8::Local<v8::Function>::Cast(callback);
        auto context = timer_bind->getContext(isolate);
        try_catch.SetVerbose(true);
        callback_as_function->Call(context, context->Global(), 0, nullptr);
        timer_bind->unref();
    });
}

v8::Local<v8::FunctionTemplate> functionTemplate(v8::Isolate *isolate)
{
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> func_tmpl = v8::FunctionTemplate::New(isolate, constructor);
    func_tmpl->SetClassName(v8::String::NewFromUtf8(isolate, "Timer", v8::NewStringType::kNormal).ToLocalChecked());
    func_tmpl->InstanceTemplate()->SetInternalFieldCount(1); // For native bind
    return handle_scope.Escape(func_tmpl);
}

// Candidate for CodeStubAssembler/Torque rewrite
void set_timeout(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::EscapableHandleScope handle_scope(isolate);
    auto timer_constructor = context->Global()->Get(context, v8::String::NewFromUtf8(isolate, "Timer", 
        v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
    auto timer_constructor_as_function = v8::Local<v8::Function>::Cast(timer_constructor);
    int argc = std::min(MAX_SET_TIMEOUT_ARGC, args.Length());
    v8::Local<v8::Value> argv[argc];
    for(int i = 0; i < argc; i++) {
        argv[i] = args[i];
    }
    auto timer_instace = timer_constructor_as_function->NewInstance(context, argc, argv);
    args.GetReturnValue().Set(handle_scope.EscapeMaybe(timer_instace).ToLocalChecked());
}
} // namespace timer
} // namespace webapi