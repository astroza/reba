#include <engine.h>
#include <rebaapi.h>

namespace rebaapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global) {
    global->Set(
        v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        worker_group::function_template(isolate));
    global->Set(
        v8::String::NewFromUtf8(isolate, "router", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        router::object_template(isolate));
}
} // namespace webapi
