#ifndef __LAKE_H__
#define __LAKE_H__

#include <v8.h>
#include <libplatform/libplatform.h>

namespace lake
{
v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged);
void v8_init();
void v8_destroy();
std::unique_ptr<v8::Platform> v8_platform;
v8::Isolate::CreateParams v8_create_params;

class ObjWrap
{
public:
    ObjWrap(v8::Local<v8::Object> handle);
private:
    v8::Persistent<v8::Object> persistent_handle;
    static void weak_callback(const v8::WeakCallbackInfo<ObjWrap> &data);
}

} // namespace lake

#endif