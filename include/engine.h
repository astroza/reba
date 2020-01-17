#ifndef __LAKE_H__
#define __LAKE_H__

#include <v8.h>
#include <libplatform/libplatform.h>

namespace lake
{
namespace engine
{
extern std::unique_ptr<v8::Platform> platform;
extern v8::Isolate::CreateParams create_params;

v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged);
void init();
void destroy();
const char* utf8_value_to_cstring(const v8::String::Utf8Value& value);
void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);

class NativeBind
{
public:
    NativeBind(v8::Isolate *isolate, v8::Local<v8::Object> handle, void *obj, void (*delete_callback)(void *));
    void ref();
    void unref();
    void *get_native_object();
    v8::Local<v8::Object> get_object_handle(v8::Isolate *isolate);
private:
    unsigned int ref_count;
    void *native_object;
    void (*native_delete_callback)(void *);
    v8::Persistent<v8::Object> persistent_handle;
    static void weak_callback(const v8::WeakCallbackInfo<NativeBind> &data);
};

template<class T>
void NativeBindDeleteCallback(void *obj) {
    T *_obj = static_cast<T *>(obj);
    delete _obj;
}
} // namespace engine
} // namespace lake

#endif