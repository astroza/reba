#include <webapi.h>

namespace webapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global)
{
    v8::Local<v8::ObjectTemplate> console = v8::ObjectTemplate::New(isolate);
    global->Set(
        v8::String::NewFromUtf8(isolate, "log", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, webapi::console::log));
}
} // namespace webapi