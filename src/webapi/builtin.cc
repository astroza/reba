#include <webapi.h>

namespace webapi
{
void init_global(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> &global)
{
    global->Set(
        v8::String::NewFromUtf8(isolate, "Timer", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        timer::function_template(isolate));
    global->Set(
        v8::String::NewFromUtf8(isolate, "setTimeout", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, timer::set_timeout));
    global->Set(
        v8::String::NewFromUtf8(isolate, "addEventListener", v8::NewStringType::kNormal)
            .ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, event::addEventListener));
}
} // namespace webapi