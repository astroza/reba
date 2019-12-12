#include <worker.h>
#include <worker_group.h>

WorkerGroup::WorkerGroup(std::shared_ptr<V8Global> v8_global, std::string script) : v8_global(v8_global)
{
    add_thread((boost::thread *)new Worker(*this));
}
