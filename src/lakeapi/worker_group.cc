#include <lakeapi.h>
#include <worker_group.h>

namespace lakeapi
{
namespace worker_group
{
void constructor(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    if (!args.IsConstructCall()) {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Function is a constructor", v8::NewStringType::kNormal).ToLocalChecked());
        return;
    }
    v8::String::Utf8Value script(isolate, args[0]);
    WorkerGroup *worker_group = new WorkerGroup(script);
    
}

v8::Local<v8::FunctionTemplate> function_template(v8::Isolate *isolate)
{
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> func_tmpl = v8::FunctionTemplate::New(isolate, constructor);
    func_tmpl->SetClassName(v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal).ToLocalChecked());
    func_tmpl->InstanceTemplate()->SetInternalFieldCount(1);
}
} // namespace worker_group
} // namespace lakeapi