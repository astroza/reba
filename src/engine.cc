#include <engine.h>

namespace reba::engine {
std::unique_ptr<v8::Platform> g_platform;
v8::Isolate::CreateParams g_create_params;

void init()
{
    v8::V8::InitializeICUDefaultLocation("reba");
    g_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(g_platform.get());
    v8::V8::Initialize();
    g_create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

void destroy()
{
    delete g_create_params.array_buffer_allocator;
}

NativeBind::NativeBind(v8::Isolate* isolate, v8::Local<v8::Object> handle, void* obj, void (*delete_callback)(void*))
{
    handle->SetAlignedPointerInInternalField(0, this);
    handle_.Reset(isolate, handle);
    context_.Reset(isolate, isolate->GetCurrentContext());
    native_object_ = obj;
    native_delete_callback_ = delete_callback;
    ref_count_ = 1;
    unref();
}

void NativeBind::ref()
{
    ref_count_++;
    handle_.ClearWeak();
}

void NativeBind::unref()
{
    if (ref_count_) {
        ref_count_--;
        if (ref_count_ == 0) {
            handle_.SetWeak(this, weakCallback, v8::WeakCallbackType::kParameter);
        }
    }
}

void* NativeBind::getNativeObject()
{
    return native_object_;
}

void NativeBind::weakCallback(const v8::WeakCallbackInfo<NativeBind>& data)
{
    NativeBind* native_bind = data.GetParameter();
    native_bind->native_delete_callback_(native_bind->getNativeObject());
    native_bind->handle_.Reset();
    native_bind->context_.Reset();
    delete native_bind;
}

v8::Local<v8::Object> NativeBind::getObjectHandle(v8::Isolate* isolate)
{
    return handle_.Get(isolate);
}

v8::Local<v8::Context> NativeBind::getContext(v8::Isolate* isolate)
{
    return context_.Get(isolate);
}

const char* utf8_value_to_cstring(const v8::String::Utf8Value& value)
{
    return *value ? *value : "<string conversion failed>";
}

void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch)
{
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(isolate, try_catch->Exception());
    const char* exception_string = utf8_value_to_cstring(exception);
    v8::Local<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        fprintf(stderr, "%s\n", exception_string);
    } else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(isolate,
            message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char* filename_string = utf8_value_to_cstring(filename);
        int linenum = message->GetLineNumber(context).FromJust();
        fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(
            isolate, message->GetSourceLine(context).ToLocalChecked());
        const char* sourceline_string = utf8_value_to_cstring(sourceline);
        fprintf(stderr, "%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++) {
            fprintf(stderr, " ");
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
        v8::Local<v8::Value> stack_trace_string;
        if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() && v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
            v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
            const char* stack_trace_string = utf8_value_to_cstring(stack_trace);
            fprintf(stderr, "%s\n", stack_trace_string);
        }
    }
}
} // namespace reba::engine
