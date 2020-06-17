#include <rebaapi.h>
#include <router.h>
#include <worker_group.h>

namespace rebaapi
{
namespace router
{
reba::Router g_default_router;

v8::Local<v8::ObjectTemplate> object_template(v8::Isolate *isolate)
{
    v8::EscapableHandleScope handle_scope(isolate);
    auto obj_tmpl = v8::ObjectTemplate::New(isolate);
    obj_tmpl->Set(
        v8::String::NewFromUtf8(isolate, "addHost", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, addHost));
    obj_tmpl->Set(
        v8::String::NewFromUtf8(isolate, "removeHost", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, removeHost));
    return handle_scope.Escape(obj_tmpl);
}

void addHost(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::HandleScope handle_scope(isolate);
    if (args.Length() < 2)
    {
        auto exception_msg = v8::String::NewFromUtf8(isolate, "Insufficient arguments", v8::NewStringType::kNormal);
        isolate->ThrowException(exception_msg.ToLocalChecked());
        return;
    }
    if (!args[0]->IsString())
    {
        auto exception_msg = v8::String::NewFromUtf8(isolate, "First argument must be a string", v8::NewStringType::kNormal);
        isolate->ThrowException(exception_msg.ToLocalChecked());
        return;
    }
    if (args[1]->IsObject())
    {
        auto worker_group_handle = v8::Local<v8::Object>::Cast(args[1]);
        auto worker_group_class = v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal);
        if (worker_group_handle->GetConstructorName()->Equals(context, worker_group_class.ToLocalChecked()).FromJust())
        {
            auto worker_group = static_cast<reba::engine::NativeBind *>(worker_group_handle->GetAlignedPointerFromInternalField(0));
            std::string host(reba::engine::utf8_value_to_cstring(v8::String::Utf8Value(isolate, args[0])));
            bool new_host = g_default_router.addHost(host, worker_group);
            args.GetReturnValue().Set(new_host);
            return;
        }
    }
    auto exception_msg = v8::String::NewFromUtf8(isolate, "Second argument must be a WorkerGroup object", v8::NewStringType::kNormal);
    isolate->ThrowException(exception_msg.ToLocalChecked());
}

void removeHost(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    if (args.Length() < 1 || !args[0]->IsString())
    {
        auto exception_msg = v8::String::NewFromUtf8(isolate, "First argument must be a string", v8::NewStringType::kNormal);
        isolate->ThrowException(exception_msg.ToLocalChecked());
        return;
    }
    std::string host(reba::engine::utf8_value_to_cstring(v8::String::Utf8Value(isolate, args[0])));
    bool host_found = g_default_router.removeHost(host);
    args.GetReturnValue().Set(host_found);
}
} // namespace router
} // namespace rebaapi