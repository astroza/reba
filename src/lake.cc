#include <lake.h>
#include <webapi.h>
#include <lakeapi.h>

namespace lake
{
std::unique_ptr<v8::Platform> v8_platform;
v8::Isolate::CreateParams v8_create_params;

v8::Local<v8::Context> v8_create_context(v8::Isolate *isolate, bool privileged)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    webapi::init_global(isolate, global);
    if (privileged)
    {
        lakeapi::init_global(isolate, global);
    }
    webapi::Console *console = static_cast<webapi::Console *>(isolate->GetData(1));
    if(!console) {
        console = new webapi::Console(isolate);
        isolate->SetData(0, console);
        v8::debug::SetConsoleDelegate(isolate, console);
    }
    return v8::Context::New(isolate, NULL, global);
}

void v8_init()
{
    v8::V8::InitializeICUDefaultLocation("lake");
    v8::V8::InitializeExternalStartupData("snapshot_blob.bin");
    v8_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(v8_platform.get());
    v8::V8::Initialize();
    v8_create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

void v8_destroy()
{
    delete v8_create_params.array_buffer_allocator;
}

NativeBind::NativeBind(v8::Isolate *isolate, v8::Local<v8::Object> handle, void *obj, void (*delete_callback)(void *))
{
    handle->SetAlignedPointerInInternalField(0, this);
    persistent_handle.Reset(isolate, handle);
    native_object = obj;
    native_delete_callback = delete_callback;
    refCount = 1;
    unref();
    
}

void NativeBind::ref()
{
    refCount++;
    persistent_handle.ClearWeak();
}

void NativeBind::unref()
{
    if(refCount) {
        refCount--;
        if(refCount == 0) {
            persistent_handle.SetWeak(this, weak_callback, v8::WeakCallbackType::kParameter);
        }
    }
}

void *NativeBind::get_native_object() {
    return native_object;
}

void NativeBind::weak_callback(const v8::WeakCallbackInfo<NativeBind> &data)
{
    NativeBind *native_bind = data.GetParameter();
    native_bind->native_delete_callback(native_bind->get_native_object());
    puts("weak_callback");
    delete native_bind;
}

v8::Local<v8::Object> NativeBind::get_object_handle(v8::Isolate *isolate) {
    return persistent_handle.Get(isolate);
}
} // namespace lake