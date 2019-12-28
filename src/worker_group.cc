#include <worker.h>
#include <worker_group.h>

#include <iostream>
WorkerGroup::WorkerGroup(V8Global &v8_global, std::string script_source, bool privileged) : v8_global(v8_global), script_source(script_source), privileged(privileged)
{
    std::cout << "1: " << this->script_source << std::endl;
    add_thread((boost::thread *)new Worker(this));
}
