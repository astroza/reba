#include <worker.h>
#include <worker_group.h>

WorkerGroup::WorkerGroup(V8Global &v8_global, std::string source) : v8_global(v8_global), script_source(source)
{
    add_thread((boost::thread *)new Worker(*this));
}
