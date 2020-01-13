#include <iostream>
#include <engine.h>
#include <worker_group.h>
#include <worker.h>

namespace lake 
{
bool report_exceptions = true;
bool print_result = true;

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
    v8::Isolate *isolate = v8::Isolate::New(lake::engine::create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = lake::engine::create_context(isolate, worker_group->privileged);
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
                lake::engine::report_exception(isolate, &try_catch);
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
                    lake::engine::report_exception(isolate, &try_catch);
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
                    const char *cstr = lake::engine::utf8_value_to_cstring(str);
                    printf("%s\n", cstr);
                }
            }
        }
      lake::engine::report_exception(isolate, &try_catch);
      while (io_context.run_one() > 0)
      {
        while (v8::platform::PumpMessageLoop(lake::engine::platform.get(), isolate))
            continue;
      }
    }
}
}
// isolate_->VisitHandlesWithClassIds( &phv );