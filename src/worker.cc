#include <iostream>
#include <lake.h>
#include <worker_group.h>
#include <worker.h>

bool report_exceptions = true;
bool print_result = true;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(isolate, try_catch->Exception());
  const char* exception_string = ToCString(exception);
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
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(
        isolate, message->GetSourceLine(context).ToLocalChecked());
    const char* sourceline_string = ToCString(sourceline);
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
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

// it must be called after root HandleScope creation
void Worker::init_api_private_keys(v8::Isolate *isolate) 
{
  api_private_keys[WorkerAPIPrivateKeyIndex::TIMER_CALLBACK] = v8::Private::ForApi(isolate, 
  v8::String::NewFromUtf8(isolate, "timer::callback").ToLocalChecked());
}

v8::MaybeLocal<v8::Private> Worker::get_api_private_key(WorkerAPIPrivateKeyIndex idx)
{
  assert(idx < WorkerAPIPrivateKeyIndex::MAX);
  return api_private_keys[idx];
}

Worker::Worker(WorkerGroup *worker_group) : worker_group(worker_group), keep_running(boost::asio::make_work_guard(io_context))
{
    thread(&Worker::run, this);
    detach();
}

void Worker::run()
{
    v8::Isolate *isolate = v8::Isolate::New(lake::v8_create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = lake::create_context(isolate, worker_group->privileged);
        v8::Context::Scope context_scope(context);
        isolate->SetData(0, this);
        init_api_private_keys(isolate);

        v8::TryCatch try_catch(isolate);
        v8::Local<v8::String> script_name =
        v8::String::NewFromUtf8(isolate, "worker.js",
                                  v8::NewStringType::kNormal).ToLocalChecked();
        v8::ScriptOrigin origin(script_name);
        
        v8::Local<v8::Script> script;
        v8::Local<v8::String> source;
        v8::String::NewFromUtf8(isolate, worker_group->script_source.c_str(), v8::NewStringType::kNormal, static_cast<int>(worker_group->script_source.size())).ToLocal(&source);
        if (!v8::Script::Compile(context, source, &origin).ToLocal(&script))
        {
            // Print errors that happened during compilation.
            if (report_exceptions)
                ReportException(isolate, &try_catch);
            return;
        }
        else
        {
            v8::Local<v8::Value> result;
            if (!script->Run(context).ToLocal(&result))
            {
                assert(try_catch.HasCaught());
                // Print errors that happened during execution.
                if (report_exceptions)
                    ReportException(isolate, &try_catch);
                return;
            }
            else
            {
                assert(!try_catch.HasCaught());
                if (print_result)
                {
                    // If all went well and the result wasn't undefined then print
                    // the returned value.
                    v8::String::Utf8Value str(isolate, result);
                    const char *cstr = ToCString(str);
                    printf("%s\n", cstr);
                }
            }
        }
      ReportException(isolate, &try_catch);
      while (io_context.run_one() > 0)
      {
        while (v8::platform::PumpMessageLoop(lake::v8_platform.get(), isolate))
            continue;
      }
    }
}

// isolate_->VisitHandlesWithClassIds( &phv );