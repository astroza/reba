#include <engine.h>
#include <rebaapi.h>

namespace rebaapi
{
void initGlobal(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global) {
    global->Set(
        v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        worker_group::functionTemplate(isolate));
    global->Set(
        v8::String::NewFromUtf8(isolate, "router", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        router::objectTemplate(isolate));
}
} // namespace webapi
