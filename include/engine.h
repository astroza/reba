#ifndef __LAKE_H__
#define __LAKE_H__

#include <libplatform/libplatform.h>
#include <v8.h>

namespace reba::engine {
extern std::unique_ptr<v8::Platform> g_platform;
extern v8::Isolate::CreateParams g_create_params;

void init();
void destroy();
const char* utf8_value_to_cstring(const v8::String::Utf8Value& value);
void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);

class NativeBind {
public:
    NativeBind(v8::Isolate* isolate, v8::Local<v8::Object> handle, void* obj, void (*delete_callback)(void*));
    void ref();
    void unref();
    void* getNativeObject();
    v8::Local<v8::Object> getObjectHandle(v8::Isolate* isolate);

private:
    unsigned int ref_count_;
    void* native_object_;
    void (*native_delete_callback_)(void*);
    v8::Persistent<v8::Object> persistent_handle_;
    static void weakCallback(const v8::WeakCallbackInfo<NativeBind>& data);
};

template <class T>
void nativeBindDeleteCallback(void* obj)
{
    T* T_obj = static_cast<T*>(obj);
    delete T_obj;
}

template <class T>
NativeBind *bind(v8::Isolate* isolate, v8::Local<v8::Object> handle, T* obj) {
    return new NativeBind(isolate, handle, obj, nativeBindDeleteCallback<T>);
}

} // namespace reba::engine

#endif