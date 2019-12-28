#include <webapi.h>
#include <lakeapi.h>

namespace lake
{
v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    webapi::init_global(isolate, global);
    if(privileged) {
        lakeapi::init_global(isolate, global);
    }
    return v8::Context::New(isolate, NULL, global);
}
} // namespace webapi