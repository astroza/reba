#include <iostream>
#include <worker_group.h>
#include <worker.h>

Worker::Worker(WorkerGroup &wg) : worker_group(wg), keep_running(boost::asio::make_work_guard(wg.io_context))
{
    thread(&Worker::main_loop, this);
    detach();
}

void Worker::main_loop() 
{
    v8::Isolate *isolate = v8::Isolate::New(wg.v8_global.create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Context::Scope context_scope(context);
        while(worker_group.io_context.run_once() > 0) {
            std::cout << "run once" << std::endl;
            while (v8::platform::PumpMessageLoop(wg.v8_global.platform, isolate)) continue;
            std::cout << "pass pump" << std::endl;
        }
    }
}