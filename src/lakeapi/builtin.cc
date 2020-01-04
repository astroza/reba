#include <lake.h>
#include <lakeapi.h>

namespace lakeapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global) {
    global->Set(
        v8::String::NewFromUtf8(isolate, "WorkerGroup", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        worker_group::function_template(isolate));
}
} // namespace webapi