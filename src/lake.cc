#include <engine.h>
#include <webapi.h>
#include <lakeapi.h>

namespace lake
{
namespace engine {
std::unique_ptr<v8::Platform> platform;
v8::Isolate::CreateParams create_params;

v8::Local<v8::Context> create_context(v8::Isolate *isolate, bool privileged)
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

void init()
{
    v8::V8::InitializeICUDefaultLocation("lake");
    v8::V8::InitializeExternalStartupData("snapshot_blob.bin");
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
}

void destroy()
{
    delete create_params.array_buffer_allocator;
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

const char* utf8_value_to_cstring(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch) {
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
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() &&
        v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
      v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char* stack_trace_string = utf8_value_to_cstring(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

} // namespace engine
} // namespace lake