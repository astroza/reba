#include <rebaapi.h>
#include <worker_group.h>

#include <iostream>

namespace rebaapi
{
namespace worker_group
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto *isolate = args.GetIsolate();
    if (!args.IsConstructCall()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Function is a constructor", v8::NewStringType::kNormal).ToLocalChecked());
        return;
    }
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value script(isolate, args[0]);
    auto *worker_group = new reba::WorkerGroup(std::string(*script), false);
    new reba::engine::NativeBind(isolate, args.This(), worker_group, reba::engine::NativeBindDeleteCallback<reba::WorkerGroup>);
}

v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate)
{
    v8::EscapableHandleScope handle_scope(isolate);
    auto func_tmpl = v8::FunctionTemplate::New(isolate, constructor);
    func_tmpl->SetClassName(v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal).ToLocalChecked());
    func_tmpl->InstanceTemplate()->SetInternalFieldCount(1); // For native bind
    return handle_scope.Escape(func_tmpl);
}
} // namespace worker_group
} // namespace rebaapi