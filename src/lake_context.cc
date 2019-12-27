#include <webapi.h>

namespace lake
{
v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    webapi::init_global(global);
    if(privileged) {
        lakeapi::init_global(global);
    }
    return v8::Context::New(isolate, NULL, global);
}
} // namespace webapi