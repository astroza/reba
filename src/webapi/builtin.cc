#include <webapi.h>
namespace webapi
{
void init_global(v8::Local<v8::ObjectTemplate> &global)
{
    v8::Local<v8::ObjectTemplate> console = v8::ObjectTemplate::New(isolate);
    console->Set(
        v8::String::NewFromUtf8(isolate, "log", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, webapi::console::log));
    global->Set(
        v8::String::NewFromUtf8(isolate, "console", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        console);
}
} // namespace webapi