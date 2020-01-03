#include <lake>
#include <webapi.h>
#include <lakeapi.h>

namespace lake
{
v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged)
{
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    webapi::init_global(isolate, global);
    if (privileged)
    {
        lakeapi::init_global(isolate, global);
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

NativeBind::NativeBind(v8::Isolate *isolate, v8::Local<v8::Object> handle, void *obj)
{
    handle->SetAlignedPointerInInternalField(0, this);
    persistent_handle->Reset(isolate, handle);
    native_obj = obj;
    refCount = 1;
    unref();
    
}

void NativeBind::ref()
{
    refCount++;
    persistent_handle->ClearWeak();
}

void NativeBind::unref()
{
    refCount--;
    if(refCount == 0) {
        persistent_handle->SetWeak(this, weak_callback, v8::WeakCallbackType::kParameter);
    }
}

void *NativeBind::get_native_obj() {
    return native_obj;
}

static void NativeBind::weak_callback(const v8::WeakCallbackInfo<NativeBind> &data)
{
    NativeBind *native_bind = data.getParameter();
    delete native_bind->get_obj();
    delete native_bind;
}

} // namespace lake